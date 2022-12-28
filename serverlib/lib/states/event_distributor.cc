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

#include <iot/states/event_distributor.hh>
#include <iot/watcher/helperevent.hh>
#include <iot/core/log.hh>
#include <sys/epoll.h>
#include <limits>
#include <iot/core/memory.hh>
#include <sys/eventfd.h>

namespace rohit {

event_distributor::event_distributor(const int thread_count, const int max_event_size)
        : thread_count(thread_count), thread_entry_map(max_thread_supported), is_terminate(false) {
    epollfd = epoll_create(max_event_size);

    if (epollfd == -1) {
        log<log_t::EVENT_DIST_CREATE_FAILED>(errno);
        throw exception_t(err_t::EVENT_DIST_CREATE_FAILED);
    } else {
        log<log_t::EVENT_DIST_CREATE_SUCCESS>();
    }

    auto cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
    if (!thread_count) this->thread_count = cpu_count;
    if (thread_count > cpu_count) {
        log<log_t::EVENT_DIST_TOO_MANY_THREAD>();
    }

    pthread_mutex_init(&eventdist_lock, nullptr);
}

event_distributor::~event_distributor() {
    delete helperevent;
}

void event_distributor::init() {
    pthread_t cleanup_thread;
    auto cleanup_ret = pthread_create(&cleanup_thread, NULL, &event_distributor::cleanup, this);
    if (cleanup_ret != 0) {
        log<log_t::PTHREAD_CREATE_FAILED>(cleanup_ret);
    }
    add_thread_map(cleanup_thread);

    log<log_t::EVENT_DIST_CREATING_THREAD>(this->thread_count);
    for (size_t cpu_index = 0; cpu_index < this->thread_count; ++cpu_index) {
        pthread_t pthread;
        auto ret = pthread_create(&pthread, NULL, &event_distributor::loop, this);
        if (ret != 0) {
            log<log_t::PTHREAD_CREATE_FAILED>(ret);
            this->thread_count = cpu_index;
            break;
        }
        add_thread_map(pthread);
    }

    if (this->thread_count == 0) {
        log<log_t::EVENT_DIST_CREATE_NO_THREAD>();
        throw exception_t(err_t::EVENT_DIST_CREATE_FAILED);
    }

    helperevent = new helperevent_executor(*this);
    helperevent->init();
}

thread_local thread_context ctx {};

void *event_distributor::loop(void *pvoid_evtdist) {
    event_distributor *pevtdist = static_cast<event_distributor *>(pvoid_evtdist);
    ctx.evtdist = pevtdist;

    while(pevtdist->thread_entry_map.find(pthread_self()) == pevtdist->thread_entry_map.end()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(config::event_dist_loop_wait_in_millis));
    }
    event_thread_entry &thread_entry = pevtdist->thread_entry_map[pthread_self()];

    // This is infinite loop
    log<log_t::EVENT_DIST_LOOP_CREATED>();

    epoll_event events[event_wait_count];
    while(true) {
        thread_entry.set_state(state_t::EVENT_DIST_EPOLL_WAIT);
        auto ret = epoll_wait(pevtdist->epollfd, events, event_wait_count, -1);

        if (ret == -1) {
            if (errno == EINTR || errno == EINVAL) {
                if (pevtdist->is_terminate) {
                    thread_entry.set_state(state_t::EVENT_DIST_EPOLL_TERMINATE);
                    pthread_exit(nullptr);
                }
            }

            log<log_t::EVENT_DIST_LOOP_WAIT_INTERRUPTED>(errno);
            sleep(1);
            // Check again if terminated
            if (pevtdist->is_terminate) {
                thread_entry.set_state(state_t::EVENT_DIST_EPOLL_TERMINATE);
                pthread_exit(nullptr);
            }
            continue;
        }

        for(decltype(ret) index = 0; index < ret; ++index) {
            thread_entry.set_state(state_t::EVENT_DIST_EPOLL_PROCESSING);

            epoll_event &event = events[index];
            log<log_t::EVENT_DIST_EVENT_RECEIVED>(event.events);

            event_executor *executor = (event_executor *)(event.data.ptr);
            thread_entry.set_state(state_t::EVENT_DIST_EPOLL_EXECUTE);

            if ((event.events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR)) != 0) {
                // Responsiblity of close is to free
                executor->mark_closed();
                thread_entry.set_state(state_t::EVENT_DIST_EPOLL_CLOSE);
            } else {
                executor->execute_protector();
            }
        }
    }

    return nullptr;
} // void *event_distributor::loop

void *event_distributor::cleanup(void *pvoid_evtdist) {
    event_distributor *pevtdist = static_cast<event_distributor *>(pvoid_evtdist);
    ctx.evtdist = pevtdist;

    while(!pevtdist->is_terminate) {
        // Cleaning up logs
        pthread_mutex_lock(&pevtdist->eventdist_lock);
        while (!pevtdist->cleanup_queue.empty() && pevtdist->cleanup_queue.front().to_free()) {
            pevtdist->cleanup_queue.front().remove_and_free(pevtdist->closed_received);
            pevtdist->cleanup_queue.pop();
        }
        pthread_mutex_unlock(&pevtdist->eventdist_lock);

        uint64_t current_time = std::chrono::system_clock::now().time_since_epoch().count();
        for(auto thread_entry: pevtdist->thread_entry_map) {
            if (thread_entry.second.pthread != pthread_self() && 
                thread_entry.second.state != state_t::EVENT_DIST_EPOLL_WAIT &&
                current_time - thread_entry.second.timestamp >= config::event_dist_deadlock_in_nanos)
            {
                log<log_t::EVENT_DIST_DEADLOCK_DETECTED>(thread_entry.second.pthread, thread_entry.second.state);
            }
        }

        std::this_thread::sleep_for(std::chrono::nanoseconds(cleanup_loop_time_in_ns));
    }

    while (!pevtdist->cleanup_queue.empty()) {
        pevtdist->cleanup_queue.front().remove_and_free(pevtdist->closed_received);
        pevtdist->cleanup_queue.pop();
    }

    return nullptr;    
} // void *event_distributor::cleanup

void event_distributor::wait() {
    for (auto thread_entry: thread_entry_map) {
        auto ret = pthread_join(thread_entry.second.pthread, nullptr);
        if (ret != 0) {
            log<log_t::EVENT_DIST_EXIT_THREAD_JOIN_FAILED>(ret);
        } else {
            log<log_t::EVENT_DIST_EXIT_THREAD_JOIN_SUCCESS>();
        }
    }   
}

class terminate_executor : public event_executor {
private:
    event_distributor &evtdist;
    int terminatefd;
public:
    terminate_executor(event_distributor &evtdist, int terminatefd)
        : evtdist(evtdist), terminatefd(terminatefd) {
    }

private:
    void execute() override {
        pthread_exit(nullptr);
    }

    void close() override {
        ctx.delayed_free(this);
    }
};

void event_distributor::terminate() {
    log<log_t::EVENT_DIST_TERMINATING>();
    is_terminate = true;

    for(decltype(thread_count) thread_index = 0; thread_index < thread_count; ++thread_index) {
        auto tempfd = eventfd(1, EFD_SEMAPHORE);
        terminate_executor termateexecutor(*this, tempfd);
        add(tempfd, EPOLLIN, &termateexecutor);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        close(tempfd);
    }

    auto ret = close(epollfd);
    if (ret != 0) {
        log<log_t::EVENT_DIST_EXIT_EPOLL_CLOSE_FAILED>(ret);
    }
}

bool event_distributor::pause() {
    return helperevent->pause_all_thread();
}

bool event_distributor::resume() {
    return helperevent->resume_all_thread();
}

} // namespace rohit