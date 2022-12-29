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

#include <iot/states/event_distributor.hh>
#include <sys/eventfd.h>
#include <pthread.h>
#include <atomic>

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

    std::atomic<size_t> pause_count = 0;
    pthread_mutex_t pause_mutex;

public:
    inline helperevent_executor(event_distributor &evtdist)
                : evtdist(evtdist), evtfd(eventfd(1, EFD_NONBLOCK))
    {
        if (evtfd == -1) {
            log<log_t::FILEWATCHER_EVENT_CREATE_FAILED>();
            throw exception_t(err_t::FILEWATCHER_EVENT_CREATE_FAILED);
        }

        pthread_mutex_init(&pause_mutex, nullptr);
    }

    ~helperevent_executor() {        
        ::close(evtfd);
        pthread_mutex_destroy(&pause_mutex);
    }

    inline void init() {
        evtdist.add(evtfd, EPOLLIN, this);
    }

    // This is blocking event
    inline void write_event(const help_event event) {
        auto ret = write(evtfd, (void *)&event, sizeof(help_event));
        if (ret <= 0) {
            log<log_t::EVENT_SERVER_HELPER_WRITE_FAILED>(errno);
            return;
        }
    }

    // This must be called from event_distributer loop
    // Returns true if pause from current thread
    inline bool pause_all_thread() {
        log<log_t::EVENT_DIST_PAUSED_THREAD>((uint64_t)pthread_self());
        auto last_pause_count = pause_count++;
        pthread_mutex_lock(&pause_mutex);
        if (last_pause_count == 0) { // This will protect from muliple thread calling this function
            const auto thread_count = evtdist.get_thread_count();
            size_t sent_count = 0;
            while(pause_count != thread_count && sent_count < thread_count * 2) {
                write_event(PAUSE_ON_LOCK);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            if (pause_count != thread_count) {
                log<log_t::EVENT_DIST_PAUSED_THREAD_FAILED>();
                return false;
            }
            return true;
        } else {
            --pause_count;
            pthread_mutex_unlock(&pause_mutex);
            return false;
        }
    }


    inline bool resume_all_thread() {
        --pause_count;
        pthread_mutex_unlock(&pause_mutex);
        log<log_t::EVENT_DIST_RESUMED_THREAD>((uint64_t)pthread_self());

        return true;
    }

private:
    inline void pause() {
        log<log_t::EVENT_DIST_PAUSED_THREAD>((uint64_t)pthread_self());
        pthread_mutex_lock(&pause_mutex);
        --pause_count; // This is already syncronized
        pthread_mutex_unlock(&pause_mutex);
        log<log_t::EVENT_DIST_RESUMED_THREAD>((uint64_t)pthread_self());
    }

    inline void execute() override {
        help_event message;
        int len;

        len = read(evtfd, (void *)&message, sizeof(help_event));
        if (len == -1 && errno != EAGAIN) {
            log<log_t::EVENT_SERVER_HELPER_READ_FAILED>(errno);
            return;
        }

        switch(message) {
        case PAUSE_ON_LOCK: {
            pause();
            break;
        }
        default:
            log<log_t::EVENT_SERVER_HELPER_UNKNOWN>();
            break;
        }
    }

    void flush() override { }

    void close() override {
        ctx.delayed_free(this);
    }
};

} // namespace rohit