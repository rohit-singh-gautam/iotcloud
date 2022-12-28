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

#include <iot/watcher/filewatcherevent.hh>
#include <iotfilemapping.hh>

namespace rohit::http {

class httpfilewatcher : public filewatcherevent<httpfilewatcher> {
public:
    using filewatcherevent::filewatcherevent;

    inline void receive_event(const std::string &, uint32_t) {
        // Do nothing here, TODO: Optimization for updating only what is changed
    }

    inline void receive_event_finalize(const std::string &) {
        webfilemap.flush_cache();
        webfilemap.update_folder();
    }

    using filewatcherevent::add_folder;
};

} // namespace rohit::http