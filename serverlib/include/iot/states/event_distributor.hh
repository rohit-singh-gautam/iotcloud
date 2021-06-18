////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/core/log.hh>
#include <unordered_map>
#include <sys/epoll.h>

namespace rohit {

enum class event_t : uint32_t {
    IN = EPOLLIN,
    OUT = EPOLLOUT,
    IO = IN | OUT,
    IN_HUP = EPOLLRDHUP,
    HUP = EPOLLHUP,
    IOH = IO | HUP,
    POLLPRI = EPOLLPRI,
    ERROR = EPOLLERR,

    // We will fill in only events those are required
};

constexpr bool operator==(const event_t lhs, const event_t rhs) {
    const uint32_t ilhs = static_cast<uint32_t>(lhs);
    const uint32_t irhs = static_cast<uint32_t>(rhs);
    
    return (ilhs & irhs) == irhs;
}

class thread_context {
    logger<false> cxtlog;
public:
    thread_context() {}

    template<log_t ID, typename... ARGS>
    constexpr void log(const ARGS&... args) {
        cxtlog.log<ID, ARGS...>(args...);
    }
};

class event_executor {
private:
    friend class event_distributor;

    // This is pure virtual function can be called only from event_distributor
    virtual void execute(thread_context &ctx, const event_t event) = 0;

}; // class event_executor

class event_distributor {
public:
    static constexpr int max_event_size = 1000000;
    static constexpr int max_thread_supported = 64;

private:
    int epollfd;
    size_t thread_count;
    pthread_t pthread[max_thread_supported];

    bool is_terminate;

    // This is a loop will keep on executing
    // till it exit
    static void *loop(void *pevtdist);

public:
    event_distributor(const int thread_count = 0, const int max_event_size = event_distributor::max_event_size);

    // event_executor memory will be used directly
    // clean up is responsibility of event_executor
    // itself.
    inline err_t add(const int fd, const event_t event, event_executor &executor) const {
        epoll_event epoll_data;
        epoll_data.events = static_cast<uint32_t>(event) | EPOLLET;
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

    inline size_t get_thread_count() const { return thread_count; }

    void terminate();
    
}; // class event_distributor

} // namespace rohit