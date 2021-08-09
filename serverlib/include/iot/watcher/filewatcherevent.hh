////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include <iot/states/event_distributor.hh>
#include <sys/inotify.h>
#include <unordered_set>
#include <unordered_map>

namespace rohit {

class filewatcherevent : public event_executor {
private:
    event_distributor &evtdist;
    const int inotifyfd;
    std::vector<int> watch_fds;
    std::unordered_set<std::string> folderlist;
    std::unordered_map<int, std::string> foldermap;
    
public:
    inline filewatcherevent(event_distributor &evtdist)
                : evtdist(evtdist), inotifyfd(inotify_init1(IN_NONBLOCK)), watch_fds(), folderlist()
    {
        if (inotifyfd == -1) {
            glog.log<log_t::FILEWATCHER_EVENT_CREATE_FAILED>();
            throw exception_t(err_t::FILEWATCHER_EVENT_CREATE_FAILED);
        }   
    }

    inline void init() {
        evtdist.add(inotifyfd, EPOLLIN, *this);
    }

    inline ~filewatcherevent() {
        close(inotifyfd);
    }

    inline err_t add_folder(const std::string folder) {
        if (folderlist.find(folder) != folderlist.end()) {
            return err_t::SUCCESS;
        }

        int watch_fd = inotify_add_watch(
                    inotifyfd, folder.c_str(),
                    IN_CREATE | IN_DELETE | IN_CLOSE_WRITE | IN_MOVED_FROM | IN_MOVED_TO );

        if (watch_fd == -1) {
            glog.log<log_t::FILEWATCHER_ADD_FOLDER_FAILED>(errno);
            return err_t::FILEWATCHER_ADD_FOLDER_FAILED;
        }

        folderlist.insert(folder);
        foldermap.insert(std::make_pair(watch_fd, folder));

        return err_t::SUCCESS;
    }

private:
    inline void execute(thread_context &ctx, const uint32_t poll_event) override {
        if ((poll_event & EPOLLIN) == 0) {
            ctx.log<log_t::FILEWATCHER_ONLY_READ_SUPPORTED>();
            return;
        }
        if ( !evtdist.pause(ctx) ) {
            return;
        }

        // Waiting so that all events are collected
        std::this_thread::sleep_for(std::chrono::nanoseconds(config::filewatcher_wait_in_ns));
        
        char buf[4096]
               __attribute__ ((aligned(__alignof__(struct inotify_event))));
        const struct inotify_event *event;
        ssize_t len;

        while(true) {
            len = read(inotifyfd, buf, sizeof(buf));
            if (len == -1 && errno != EAGAIN) {
                ctx.log<log_t::EVENT_SERVER_HELPER_READ_FAILED>(errno);
                break;
            }

            if (len == -1) {
                // Non blocking call, hence returning
                break;
            }

            for (char *ptr = buf; ptr < buf + len;
                       ptr += sizeof(struct inotify_event) + event->len) {

                event = (const struct inotify_event *) ptr;

                /* Print event type. */

                if (event->mask & IN_CREATE)
                    printf("IN_CREATE: ");
                if (event->mask & IN_DELETE)
                    printf("IN_DELETE: ");
                if (event->mask & IN_CLOSE_WRITE)
                    printf("IN_CLOSE_WRITE: ");
                if (event->mask & IN_CLOSE_NOWRITE)
                    printf("IN_CLOSE_NOWRITE: ");
                if (event->mask & IN_MOVED_FROM)
                    printf("IN_MOVED_FROM: ");
                if (event->mask & IN_MOVED_TO)
                    printf("IN_MOVED_TO: ");

                /* Print the name of the watched directory. */
                std::cout << foldermap[event->wd] << "/";

                /* Print the name of the file. */
                if (event->len)
                    std::cout << event->name;

                /* Print type of filesystem object. */
                if (event->mask & IN_ISDIR)
                    printf(" [directory]\n");
                else
                    printf(" [file]\n");
            } // for
        } // while

        evtdist.resume(ctx);
    } // execute
};

} // namespace rohit