////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/states/event_distributor.hh>
#include <sys/eventfd.h>
#include <pthread.h>

namespace rohit {

class helperevent_executor : public event_executor {
public:
    enum help_event : uint64_t {
        NO_EVENT, // No event will be called for this, hence do not use this
        PAUSE_ON_LOCK,
    };

private:
    event_distributor &evtdist;
    const int evtfd;

    int pause_count;
    pthread_mutex_t pause_mutex;

public:
    inline helperevent_executor(event_distributor &evtdist)
                : evtdist(evtdist), evtfd(eventfd(1, EFD_NONBLOCK)), pause_count(0)
    {
        if (evtfd == -1) {
            glog.log<log_t::FILEWATCHER_EVENT_CREATE_FAILED>();
            throw exception_t(err_t::FILEWATCHER_EVENT_CREATE_FAILED);
        }

        pthread_mutex_init(&pause_mutex, nullptr);
    }

    ~helperevent_executor() {        
        close(evtfd);
        pthread_mutex_destroy(&pause_mutex);
    }

    inline void init() {
        evtdist.add(evtfd, EPOLLIN, *this);
    }

    // This is blocking event
    inline void write_event(const help_event event) {
        auto ret = write(evtfd, (void *)&event, sizeof(help_event));
        if (ret <= 0) {
            glog.log<log_t::EVENT_SERVER_HELPER_WRITE_FAILED>(errno);
            return;
        }
    }

    // This must be called from event_distributer loop
    // Returns true if pause from current thread
    inline bool pause_all_thread(thread_context &ctx) {
        ctx.log<log_t::EVENT_DIST_PAUSED_THREAD>((uint64_t)pthread_self());
        auto last_pause_count = __sync_fetch_and_add(&pause_count, 1);
        pthread_mutex_lock(&pause_mutex);
        if (last_pause_count == 0) { // This will protect from muliple thread calling this function
            const auto thread_count = evtdist.get_thread_count();
            size_t sent_count = 0;
            while(pause_count != thread_count && sent_count < thread_count * 2) {
                write_event(PAUSE_ON_LOCK);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            if (pause_count != thread_count) {
                ctx.log<log_t::EVENT_DIST_PAUSED_THREAD_FAILED>();
                return false;
            }
            return true;
        } else {
            --pause_count;
            pthread_mutex_unlock(&pause_mutex);
            return false;
        }
    }


    inline bool resume_all_thread(thread_context &ctx) {
        --pause_count;
        pthread_mutex_unlock(&pause_mutex);
        ctx.log<log_t::EVENT_DIST_RESUMED_THREAD>((uint64_t)pthread_self());

        return true;
    }

private:
    inline void pause(thread_context &ctx) {
        ctx.log<log_t::EVENT_DIST_PAUSED_THREAD>((uint64_t)pthread_self());
        auto last_pause_count = __sync_fetch_and_add(&pause_count, 1);
        pthread_mutex_lock(&pause_mutex);
        --pause_count; // This is already syncronized
        pthread_mutex_unlock(&pause_mutex);
        ctx.log<log_t::EVENT_DIST_RESUMED_THREAD>((uint64_t)pthread_self());
    }

    inline void execute(thread_context &ctx, const uint32_t poll_event) override {
        if ((poll_event & EPOLLIN) == 0) {
            ctx.log<log_t::EVENT_SERVER_HELPER_ONLY_READ_SUPPORTED>();
            return;
        }

        help_event message;
        int len;

        len = read(evtfd, (void *)&message, sizeof(help_event));
        if (len == -1 && errno != EAGAIN) {
            ctx.log<log_t::EVENT_SERVER_HELPER_READ_FAILED>(errno);
            return;
        }

        switch(message) {
        case PAUSE_ON_LOCK: {
            pause(ctx);
            break;
        }
        default:
            ctx.log<log_t::EVENT_SERVER_HELPER_UNKNOWN>();
            break;
        }

    }
};

} // namespace rohit