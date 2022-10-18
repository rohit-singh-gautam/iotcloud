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

#include <http2.hh>

namespace rohit::http::v2 {

std::ostream& operator<<(std::ostream& os, const header_request &header_request) {
    const http_header_request &http11request = header_request;

    os << "Stream Identifier: " << header_request.stream_identifier << std::endl
        << "Weight: " << (unsigned int)header_request.weight << std::endl;
    
    if (header_request.error != frame::error_t::NO_ERROR) {
        os << "Error: " << header_request.error << std::endl;
    }

    return os << http11request;
}

} // namespace rohit::http::v2