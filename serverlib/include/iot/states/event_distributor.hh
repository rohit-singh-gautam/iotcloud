////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/core/error.hh>
#include <iot/core/log.hh>
#include <unordered_map>
#include <sys/epoll.h>

namespace rohit {

class thread_context;

class event_executor {
private:
    friend class event_distributor;

    // This is pure virtual function can be called only from event_distributor
    virtual void execute(thread_context &ctx, const uint32_t event) = 0;

}; // class event_executor

class event_distributor {
public:
    static constexpr int max_event_size = 1000000;
    static constexpr int max_thread_supported = 64;

    static constexpr int event_wait_count = 1;

private:
    int epollfd;
    size_t thread_count;
    pthread_t pthread[max_thread_supported];

    bool is_terminate;

    // This is a loop will keep on executing
    // till it exit
    static void *loop(void *pevtdist);

    friend class terminate_executor;

public:
    event_distributor(const int thread_count = 0, const int max_event_size = event_distributor::max_event_size);

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

    inline size_t get_thread_count() const { return thread_count; }

    void wait();

    void terminate();
    
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