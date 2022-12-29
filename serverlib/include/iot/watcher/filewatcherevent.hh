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
#include <sys/inotify.h>
#include <unordered_set>
#include <unordered_map>
#include <dirent.h>
#include <unistd.h>

namespace rohit {

template <typename eventreceiver>
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
            log<log_t::FILEWATCHER_EVENT_CREATE_FAILED>();
            throw exception_t(err_t::FILEWATCHER_EVENT_CREATE_FAILED);
        }   
    }

    inline void init() {
        evtdist.add(inotifyfd, EPOLLIN, this);
    }

    inline ~filewatcherevent() {
        ::close(inotifyfd);
    }

    // This function is called for each event
    void receive_event(const std::string &filename, uint32_t eventmask) {
        static_cast<eventreceiver *>(this)->receive_event(filename, eventmask);
    }

    // This function is called for finalizing event
    // Ideal for situation where full cleanup
    // Is done for each event
    void receive_event_finalize(const std::string &watchfolder) {
        static_cast<eventreceiver *>(this)->receive_event_finalize(watchfolder);
    }

    inline err_t add_folder(const std::string folder) {
        if (folderlist.find(folder) != folderlist.end()) {
            return err_t::SUCCESS;
        }

        DIR * dir = opendir(folder.c_str());

        if (!dir) {
            // Returning failure for non directory
            // TODO: return specific error
            return err_t::GENERAL_FAILURE;
        }

        int watch_fd = inotify_add_watch(
                    inotifyfd, folder.c_str(),
                    IN_CREATE | IN_DELETE | IN_CLOSE_WRITE | IN_MOVED_FROM | IN_MOVED_TO );

        if (watch_fd == -1) {
            log<log_t::FILEWATCHER_ADD_FOLDER_FAILED>(errno);
            return err_t::FILEWATCHER_ADD_FOLDER_FAILED;
        }

        folderlist.insert(folder);
        foldermap.insert(std::make_pair(watch_fd, folder));

        while(1) {
            dirent *child = readdir(dir);
            if (!child) break;

            if ( !strcmp(child->d_name, ".") || !strcmp(child->d_name, "..") ) {
                // Ignore current and parent directory
                continue;
            }
            
            switch(child->d_type) {
            case DT_DIR: {
                add_folder(folder + "/" + std::string(child->d_name));
                break;
            }
            
            default:
                // Other file types we will ignore
                break;
            }
        }


        if (closedir(dir)) {
            // TODO: return errno specific error
            return err_t::GENERAL_FAILURE;
        }

        return err_t::SUCCESS;
    }

private:
    inline void execute() override {
        if ( !evtdist.pause() ) {
            return;
        }

        // Waiting so that all events are collected
        std::this_thread::sleep_for(std::chrono::nanoseconds(config::filewatcher_wait_in_ns));

        std::vector<std::string> watchfolder_list;
        
        char buf[4096]
               __attribute__ ((aligned(__alignof__(struct inotify_event))));
        const struct inotify_event *event;
        ssize_t len;

        std::vector<int> remove_folders;

        while(true) {
            len = read(inotifyfd, buf, sizeof(buf));
            if (len == -1 && errno != EAGAIN) {
                log<log_t::EVENT_SERVER_HELPER_READ_FAILED>(errno);
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

                auto &watchfolder = foldermap[event->wd];
                watchfolder_list.push_back(watchfolder);

                if constexpr (config::debug) {
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
                    std::cout << watchfolder << "/";
                    /* Print the name of the file. */
                    if (event->len)
                        std::cout << event->name;

                    /* Print type of filesystem object. */
                    if (event->mask & IN_ISDIR)
                        printf(" [directory]\n");
                    else
                        printf(" [file]\n");
                }

                std::string childfolder = watchfolder + "/";
                childfolder += event->name;
                if (event->mask & IN_ISDIR) {
                    if (event->mask & (IN_CREATE | IN_MOVED_TO))
                        add_folder(watchfolder + "/" + event->name);
                    else if(event->mask & (IN_DELETE | IN_MOVED_FROM)) {
                        for(auto folder: foldermap) {
                            if (folder.second.starts_with(childfolder)) {
                                remove_folders.push_back(folder.first);
                            }
                        }
                    }

                } else
                    receive_event(childfolder , event->mask);
            } // for
        } // while

        for(auto watchfolder: watchfolder_list) {
            receive_event_finalize(watchfolder);
        }

        for(auto folder: remove_folders) {
            inotify_rm_watch(inotifyfd, folder);
            foldermap.erase(folder);
        }

        evtdist.resume();
    } // execute


    void flush() override { }

    void close() override {
        ctx.delayed_free(this);
    }
};

} // namespace rohit