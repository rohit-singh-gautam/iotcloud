/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// This program is free software: you can redistribute it and/or modify it under the terms //
// of the GNU General Public License as published by the Free Software Foundation, either  //
// version 3 of the License, or (at your option) any later version.                        //
//                                                                                         //
// This program is distributed in the hope that it will be useful, but WITHOUT ANY         //
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A         //
// PARTICULAR PURPOSE. See the GNU General Public License for more details.                //
//                                                                                         //
// You should have received a copy of the GNU General Public License along with this       //
// program. If not, see <https://www.gnu.org/licenses/>.                                   //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <iot/states/statesentry.hh>
#include <iot/core/error.hh>
#include <iot/core/log.hh>
#include <unordered_map>
#include <sys/epoll.h>
#include <queue>
#include <unordered_set>

namespace rohit {

class thread_context;
extern thread_local thread_context ctx;

class event_executor {
protected:
    std::atomic<int> executor_count = 0;
    bool closed = false;   

    // This is pure virtual function can be called only from event_distributor
    // event is irreralevent as in our case we are following
    // no read till all write is done and compulsory read after write
    virtual void execute() = 0;

    // Close is not thread safe
    // Implementation requires to care about it
    // close also require to free itself
    // ctx.delayed_free(this); is recommended method to free
    virtual void close() = 0;
    virtual void flush() = 0;

    friend class event_distributor;

public:
    virtual ~event_executor() = default;

    // Make sure to call enter loop before making this call
    inline void execute_protector_noenter() {
        assert(executor_count >= 1);
        auto loop = true;

        while(loop) {
            if (closed) {
                close();

                // No need to exit loop
                // This will prevent other thread to enter
                break;
            }
            execute();
            loop = !exit_loop();
        }
    }

    inline void execute_protector() {
        auto loop = enter_loop();

        while(loop) {
            if (closed) {
                close();

                // No need to exit loop
                // This will prevent other thread to enter
                break;
            }
            execute();
            loop = !exit_loop();
        }
    }

    inline void mark_closed(bool readclose) {
        closed = true;
        auto loop = enter_loop();

        // If existing thread is running
        // it is task of existing thread to close
        if (loop) {
            // No need to exit loop
            if (readclose) flush();
            close();
        }
    }

    inline bool enter_loop() {
        auto value = executor_count++;
        return value == 0;
    }
    
    inline bool exit_loop() {
        return --executor_count == 0;
    }

}; // class event_executor

class event_cleanup {
private:
    event_executor *ptr;
    const uint64_t timestamp;

public:
    inline event_cleanup(event_executor *ptr)
            : ptr(ptr), timestamp(std::chrono::system_clock::now().time_since_epoch().count()) { }

    inline bool to_free() const {
        const uint64_t current_timestamp = std::chrono::system_clock::now().time_since_epoch().count();
        return timestamp + config::event_cleanup_time_in_ns <= current_timestamp;
    }

    inline void remove_and_free(std::unordered_set<event_executor *> &closed_received) {
        closed_received.erase(ptr);
        delete ptr;
    }
};

struct event_thread_entry {
    pthread_t pthread;
    state_t state;
    uint64_t timestamp;

    inline event_thread_entry() {}

    inline event_thread_entry(const pthread_t pthread)
            :   pthread(pthread),
                state(state_t::EVENT_DIST_NONE),
                timestamp(std::chrono::system_clock::now().time_since_epoch().count())  { }

    inline void set_state(const state_t state) {
        this->state = state;
        timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    }
};

class helperevent_executor;

class event_distributor {
public:
    static constexpr int max_event_size = 1000000;
    static constexpr int max_thread_supported = 64;
    static constexpr int event_wait_count = 1;

    static constexpr uint64_t cleanup_loop_time_in_ns = 2ULL * 1000ULL * 1000000ULL; // two second

private:
    int epollfd;
    size_t thread_count;
    std::unordered_map<pthread_t, event_thread_entry> thread_entry_map;

    bool is_terminate;

    // This is a loop will keep on executing
    // till it exit
    static void *loop(void *pevtdist);
    static void *cleanup(void *pevtdist);

    pthread_mutex_t eventdist_lock;
    std::unordered_set<event_executor *> closed_received;
    std::queue<event_cleanup> cleanup_queue;

    friend class terminate_executor;

public:
    // Delayed free can be called while transfer
    // true = Entry was not made, executor can be called for config::event_cleanup_time_in_ns time
    inline bool delayed_free(event_executor *ptr) {
        bool call_execute = false;
        pthread_mutex_lock(&eventdist_lock);
        if (closed_received.find(ptr) == closed_received.end()) {
            // Entry has not been made yet
            call_execute = true;
            closed_received.insert(ptr);
            cleanup_queue.push({ ptr });
        }
        pthread_mutex_unlock(&eventdist_lock);
        return call_execute;
    }

private:
    inline void add_thread_map(pthread_t pthread) {
        event_thread_entry thread_entry(pthread);
        thread_entry_map.insert(std::make_pair(pthread, thread_entry));
    }

    std::unique_ptr<helperevent_executor> helperevent;

public:
    event_distributor(const int thread_count = 0, const int max_event_size = event_distributor::max_event_size);

    void init();

    // event_executor memory will be used directly
    // clean up is responsibility of event_executor
    // itself.
    inline err_t add(const int fd, const uint32_t event, event_executor *pexecutor) const {
        epoll_event epoll_data;
        epoll_data.events = event | EPOLLET | EPOLLRDHUP;
        epoll_data.data.ptr = pexecutor;

        if constexpr (config::debug) {
            if (fd == 0) {
                throw exception_t(err_t::EVENT_CREATE_FAILED_ZERO);
            }
        }

        auto ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &epoll_data);

        if (ret == -1) {
            log<log_t::EVENT_CREATE_FAILED>(fd, errno);
            return err_t::EVENT_CREATE_FAILED;
        } else {
            log<log_t::EVENT_CREATE_SUCCESS>(fd);
            return err_t::SUCCESS;
        }
    }

    inline err_t remove(const int fd) {
        auto ret = epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr);
            if (ret == -1) {
            log<log_t::EVENT_REMOVE_FAILED>(fd, errno);
            return err_t::EVENT_REMOVE_FAILED;
        } else {
            log<log_t::EVENT_REMOVE_SUCCESS>(fd);
            return err_t::SUCCESS;
        }
    }

    constexpr size_t get_thread_count() const { return thread_count; }

    void wait();

    void terminate();
    bool isTerminated() const { return is_terminate; }

    bool pause();
    bool resume();
    
}; // class event_distributor

class thread_context {
private:
    event_distributor *evtdist { nullptr };

    friend event_distributor;
public:
    inline thread_context() {}

    inline err_t remove_event(const int fd) {
        return evtdist->remove(fd);
    }

    inline bool delayed_free(event_executor *executor) {
        return evtdist->delayed_free(executor);
    }

    inline err_t add_event(const int fd, const uint32_t event, event_executor *pexecutor) {
        return evtdist->add(fd, event, pexecutor);
    }

    static constexpr size_t buffer_size = 16384;
    uint8_t read_buffer[buffer_size]; // Read buffer size;
    uint8_t write_buffer[buffer_size]; // Write buffer size
}; // class thread_context

} // namespace rohit