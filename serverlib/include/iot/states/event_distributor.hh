////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

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

class event_executor {
private:
    friend class event_distributor;

    // This is pure virtual function can be called only from event_distributor
    virtual void execute(thread_context &ctx, const uint32_t event) = 0;

public:
    virtual ~event_executor() { }

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

    // true is not yet called and execute must be called
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

    inline void add_thread_map(pthread_t pthread) {
        event_thread_entry thread_entry(pthread);
        thread_entry_map.insert(std::make_pair(pthread, thread_entry));
    }

    helperevent_executor *helperevent;

public:
    event_distributor(const int thread_count = 0, const int max_event_size = event_distributor::max_event_size);
    ~event_distributor();

    void init();

    // event_executor memory will be used directly
    // clean up is responsibility of event_executor
    // itself.
    inline err_t add(const int fd, const uint32_t event, event_executor &executor) const {
        epoll_event epoll_data;
        epoll_data.events = event | EPOLLET | EPOLLRDHUP;
        epoll_data.data.ptr = &executor;

        auto ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &epoll_data);

        if (ret == -1) {
            glog.log<log_t::EVENT_CREATE_FAILED>(errno);
            return err_t::EVENT_CREATE_FAILED;
        } else {
            glog.log<log_t::EVENT_CREATE_SUCCESS>();
            return err_t::SUCCESS;
        }
    }

    inline err_t remove(const int fd) {
        auto ret = epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr);
            if (ret == -1) {
            glog.log<log_t::EVENT_REMOVE_FAILED>(errno);
            return err_t::EVENT_REMOVE_FAILED;
        } else {
            glog.log<log_t::EVENT_REMOVE_SUCCESS>();
            return err_t::SUCCESS;
        }
    }

    constexpr size_t get_thread_count() const { return thread_count; }

    void wait();

    void terminate();

    bool pause(thread_context &ctx);
    bool resume(thread_context &ctx);
    
}; // class event_distributor

class thread_context {
private:
    logger<false> cxtlog;
    event_distributor &evtdist;

public:
    inline thread_context(event_distributor &evtdist) : evtdist(evtdist) {}

    template<log_t ID, typename... ARGS>
    constexpr void log(const ARGS&... args) {
        cxtlog.log<ID, ARGS...>(args...);
    }

    inline err_t remove_event(const int fd) {
        return evtdist.remove(fd);
    }

    inline err_t add_event(const int fd, const uint32_t event, event_executor &executor) {
        return evtdist.add(fd, event, executor);
    }

}; // class thread_context

} // namespace rohit