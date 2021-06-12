////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iot/states/event_distributor.hh>
#include <iot/core/log.hh>
#include <sys/epoll.h>
#include <limits>

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
    thread_context ctx;

    // This is infinite loop
    auto ret = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
    if (ret != 0)
        ctx.log<log_t::EVENT_DIST_NO_THREAD_CANCEL>(ret);

    while(true) {
        epoll_event event;
        ret = epoll_wait(pevtdist->epollfd, &event, 1, std::numeric_limits<int>::max());
        if (ret != 0) {
            if (ret == EINTR || ret == EINVAL) {
                if (pevtdist->is_terminate) pthread_exit(nullptr);
            }

            ctx.log<log_t::EVENT_DIST_LOOP_WAIT_INTERRUPTED>(ret);
            sleep(1);
            // Check again if terminated
            if (pevtdist->is_terminate) pthread_exit(nullptr);
            continue;
        }

        event_executor *executor = static_cast<event_executor *>(event.data.ptr);
        executor->execute(ctx);
    }

    return nullptr;
}

void event_distributor::terminate() {
    is_terminate = true;
    auto ret = close(epollfd);
    if (ret != 0) {
        glog.log<log_t::EVENT_DIST_EXIT_EPOLL_CLOSE_FAILED>(ret);
    }

    // Wait for fd to close
    // One second is sufficient as this server is designed for
    // very small request
    sleep(1);

    for (size_t cpu_index = 0; cpu_index < this->thread_count; ++cpu_index) {
        ret = pthread_cancel(pthread[cpu_index]);
        if (ret != 0) {
            glog.log<log_t::EVENT_DIST_EXIT_THREAD_CANCEL_FAILED>(ret);
        }
    }

    for (size_t cpu_index = 0; cpu_index < this->thread_count; ++cpu_index) {
        ret = pthread_join(pthread[cpu_index], NULL);
        if (ret != 0) {
            glog.log<log_t::EVENT_DIST_EXIT_THREAD_JOIN_FAILED>(ret);
        }
    }
}

} // namespace rohit