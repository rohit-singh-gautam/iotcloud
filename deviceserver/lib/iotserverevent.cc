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

#include <iotserverevent.hh>

namespace rohit {

void read_register(message_base_t *, write_function writeFunction)
{
    write_success_request(writeFunction);
}

void read_connect(message_base_t *, write_function writeFunction)
{
    write_success_request(writeFunction);
}

void read_command(message_command_t *base, write_function writeFunction) {
    if (base->verify())
    {
        for(auto command: *base)
        {
            switch(command.GetOperation())
            {
                case operation_t::SWITCH:
                    write_success_request(writeFunction);
                    break;
                
                case operation_t::LEVEL:
                    write_success_request(writeFunction);
                    break;
            }
        }
    }
    else
    {
        write_bad_request(writeFunction);
    }
}

} // namespace rohit