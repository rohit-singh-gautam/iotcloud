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
    auto ret = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
    if (ret != 0)
        ctx.log<log_t::EVENT_DIST_NO_THREAD_CANCEL>(ret);

    ctx.log<log_t::EVENT_DIST_LOOP_CREATED>();
    while(true) {
        epoll_event event;
        ret = epoll_wait(pevtdist->epollfd, &event, 1, std::numeric_limits<int>::max());

        if (ret == -1) {
            if (errno == EINTR || errno == EINVAL) {
                if (pevtdist->is_terminate) pthread_exit(nullptr);
            }

            ctx.log<log_t::EVENT_DIST_LOOP_WAIT_INTERRUPTED>(errno);
            sleep(1);
            // Check again if terminated
            if (pevtdist->is_terminate) pthread_exit(nullptr);
            continue;
        } else {
            ctx.log<log_t::EVENT_DIST_EVENT_RECEIVED>(event.events);
        }

        event_executor *executor = static_cast<event_executor *>(event.data.ptr);
        executor->execute(ctx, event.events);
    }

    return nullptr;
}

void event_distributor::wait() {
    for (size_t cpu_index = 0; cpu_index < this->thread_count; ++cpu_index) {
        auto ret = pthread_join(pthread[cpu_index], NULL);
        if (ret != 0) {
            glog.log<log_t::EVENT_DIST_EXIT_THREAD_JOIN_FAILED>(ret);
        } else {
            glog.log<log_t::EVENT_DIST_EXIT_THREAD_JOIN_SUCCESS>();
        }
    }
}

class terminate_executor : event_executor {
private:
    event_distributor &evtdist;
    int terminatefd;
public:
    terminate_executor(event_distributor &evtdist, int terminatefd) : evtdist(evtdist), terminatefd(terminatefd) {
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