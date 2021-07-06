////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iot/states/event_distributor.hh>
#include <iot/core/log.hh>
#include <sys/epoll.h>
#include <limits>
#include <iot/core/memory.hh>
#include <sys/eventfd.h>
#include <iot/states/statesentry.hh>

namespace rohit {

event_distributor::event_distributor(const int thread_count, const int max_event_size)
        : thread_count(thread_count), pthread(), is_terminate(false) {
    epollfd = epoll_create(max_event_size);

    if (epollfd == -1) {
        glog.log<log_t::EVENT_DIST_CREATE_FAILED>(errno);
        throw exception_t(err_t::EVENT_DIST_CREATE_FAILED);
    } else {
        glog.log<log_t::EVENT_DIST_CREATE_SUCCESS>();
    }

    auto cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
    if (!thread_count) this->thread_count = cpu_count;
    if (thread_count > cpu_count) {
        glog.log<log_t::EVENT_DIST_TOO_MANY_THREAD>();
    }
}

void event_distributor::init() {
    auto cleanup_ret = pthread_create(&cleanup_thread, NULL, &event_distributor::cleanup, this);
    if (cleanup_ret != 0) {
        glog.log<log_t::PTHREAD_CREATE_FAILED>(cleanup_ret);
    }

    glog.log<log_t::EVENT_DIST_CREATING_THREAD>(this->thread_count);
    for (size_t cpu_index = 0; cpu_index < this->thread_count; ++cpu_index) {
        auto ret = pthread_create(&pthread[cpu_index], NULL, &event_distributor::loop, this);
        if (ret != 0) {
            glog.log<log_t::PTHREAD_CREATE_FAILED>(ret);
            this->thread_count = cpu_index;
            break;
        }
    }

    if (this->thread_count == 0) {
        glog.log<log_t::EVENT_DIST_CREATE_NO_THREAD>();
        throw exception_t(err_t::EVENT_DIST_CREATE_FAILED);
    }
}

void *event_distributor::loop(void *pvoid_evtdist) {
    event_distributor *pevtdist = static_cast<event_distributor *>(pvoid_evtdist);
    thread_context ctx(*pevtdist);

    // This is infinite loop
    ctx.log<log_t::EVENT_DIST_LOOP_CREATED>();

    epoll_event events[event_wait_count];
    while(true) {
        auto ret = epoll_wait(pevtdist->epollfd, events, event_wait_count, std::numeric_limits<int>::max());

        if (ret == -1) {
            if (errno == EINTR || errno == EINVAL) {
                if (pevtdist->is_terminate) pthread_exit(nullptr);
            }

            ctx.log<log_t::EVENT_DIST_LOOP_WAIT_INTERRUPTED>(errno);
            sleep(1);
            // Check again if terminated
            if (pevtdist->is_terminate) pthread_exit(nullptr);
            continue;
        }

        for(size_t index = 0; index < ret; ++index) {
            epoll_event &event = events[index];
            ctx.log<log_t::EVENT_DIST_EVENT_RECEIVED>(event.events);
            event_executor *executor = static_cast<event_executor *>(event.data.ptr);
            if ((event.events & (EPOLLHUP | EPOLLRDHUP )) != 0) {
                if (!pevtdist->delayed_free(executor)) continue;
            }
            executor->execute(ctx, event.events);
        }
    }

    return nullptr;
} // void *event_distributor::loop

void *event_distributor::cleanup(void *pvoid_evtdist) {
    event_distributor *pevtdist = static_cast<event_distributor *>(pvoid_evtdist);
    thread_context ctx(*pevtdist);

    while(!pevtdist->is_terminate) {
        pthread_mutex_lock(&pevtdist->cleanup_lock);
        while (!pevtdist->cleanup_queue.empty() && pevtdist->cleanup_queue.front().to_free()) {
            std::cout << "Reached here cleanup, Timestamp " << std::chrono::system_clock::now().time_since_epoch().count() << std::endl;
            pevtdist->cleanup_queue.front().remove_and_free(pevtdist->closed_received);
            pevtdist->cleanup_queue.pop();
        }
        pthread_mutex_unlock(&pevtdist->cleanup_lock);

        std::this_thread::sleep_for(std::chrono::nanoseconds(cleanup_loop_time_in_ns));
    }

    while (!pevtdist->cleanup_queue.empty()) {
        pevtdist->cleanup_queue.front().remove_and_free(pevtdist->closed_received);
        pevtdist->cleanup_queue.pop();
    }

    return nullptr;    
} // void *event_distributor::cleanup

void event_distributor::wait() {
    for (size_t cpu_index = 0; cpu_index < this->thread_count; ++cpu_index) {
        auto ret = pthread_join(pthread[cpu_index], nullptr);
        if (ret != 0) {
            glog.log<log_t::EVENT_DIST_EXIT_THREAD_JOIN_FAILED>(ret);
        } else {
            glog.log<log_t::EVENT_DIST_EXIT_THREAD_JOIN_SUCCESS>();
        }
    }

    auto cleanup_ret = pthread_join(cleanup_thread, nullptr);
    if (cleanup_ret != 0) {
        glog.log<log_t::EVENT_DIST_EXIT_THREAD_JOIN_FAILED>(cleanup_ret);
    } else {
        glog.log<log_t::EVENT_DIST_EXIT_THREAD_JOIN_SUCCESS>();
    }
        
}

class terminate_executor : public event_executor {
private:
    event_distributor &evtdist;
    int terminatefd;
public:
    terminate_executor(event_distributor &evtdist, int terminatefd)
        : evtdist(evtdist), terminatefd(terminatefd) {
        evtdist.add(terminatefd, EPOLLIN, *this);
    }

private:
    void execute(thread_context &ctx, const uint32_t event) override {
        pthread_exit(nullptr);
    }
};

void event_distributor::terminate() {
    glog.log<log_t::EVENT_DIST_TERMINATING>();
    is_terminate = true;

    epoll_event epoll_data;
    epoll_data.events = EPOLLIN | EPOLLET;
    epoll_data.data.ptr = nullptr;

    for(int thread_index = 0; thread_index < thread_count; ++thread_index) {
        auto tempfd = eventfd(1, EFD_SEMAPHORE);
        terminate_executor termateexecutor(*this, tempfd);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        close(tempfd);
    }

    auto ret = close(epollfd);
    if (ret != 0) {
        glog.log<log_t::EVENT_DIST_EXIT_EPOLL_CLOSE_FAILED>(ret);
    }
}

} // namespace rohit