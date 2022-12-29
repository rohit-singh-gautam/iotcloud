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

namespace rohit {

#define STATE_DEFINITION_END

#define STATE_ENTRY_LIST \
    STATE_ENTRY(STATE_UNKNOWN, "Unknown state") \
    \
    STATE_ENTRY(EVENT_DIST_NONE, "Not yet started") \
    STATE_ENTRY(EVENT_DIST_EPOLL_WAIT, "Epoll waiting for Event") \
    STATE_ENTRY(EVENT_DIST_EPOLL_TERMINATE, "Epoll Terminated") \
    STATE_ENTRY(EVENT_DIST_EPOLL_ERROR, "Epoll error") \
    STATE_ENTRY(EVENT_DIST_EPOLL_PROCESSING, "Epoll processing") \
    STATE_ENTRY(EVENT_DIST_EPOLL_CLOSE, "Epoll close") \
    STATE_ENTRY(EVENT_DIST_EPOLL_EXECUTE, "Epoll execute") \
    \
    STATE_ENTRY(SOCKET_PEER_ACCEPT, "Accept not completed, happens with SSL") \
    STATE_ENTRY(SOCKET_PEER_EVENT, "Next operation is based on event type") \
    STATE_ENTRY(SOCKET_PEER_READ, "Next operation is READ") \
    STATE_ENTRY(SOCKET_PEER_WRITE, "Next operation is WRITE") \
    STATE_ENTRY(SOCKET_PEER_CLOSE, "Next operation is CLOSE") \
    STATE_ENTRY(SOCKET_PEER_READ_CLOSE, "Next operation is READ CLOSE") \
    STATE_ENTRY(SOCKET_PEER_CLOSED, "Socket is close no further operation must be performed on this") \
    STATE_ENTRY(HTTP2_SETTINGS_WRITE, "HTTP2 write settings as this is first packet") \
    STATE_ENTRY(HTTP2_NEXT_MAGIC, "HTTP2 next frame is a magic frame") \
    STATE_ENTRY(HTTP2_FIRST_FRAME, "HTTP2 this is first frame without connection preface") \
    STATE_ENTRY(SERVEREVENT_MOVED, "SERVEREVENT is moved to another object") \
    \
    STATE_DEFINITION_END


} // namespace rohit

