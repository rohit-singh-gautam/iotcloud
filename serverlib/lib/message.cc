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

#include <iot/message.hh>
#include <sstream>

std::ostream& operator<<(std::ostream& os, const rohit::message::Operation::Code &operation) {
    return os << to_string(operation);
}

std::ostream& operator<<(std::ostream& os, const rohit::message::CommandEntry &command) {
    return os  << command.GetDevice() 
                << ":" << command.GetComponent()
                << ":" << command.GetOperation()
                << ":" << command.GetOperationData();
}

std::ostream& operator<<(std::ostream& os, const rohit::message::Base &message) {
    rohit::message::Code code = message;
    if (code > rohit::message::Code::COMMAND) {
        return os << "Bad message " << static_cast<int>(code) << std::endl;
    }
    switch(message) {
    case rohit::message::Code::UNKNOWN:
    case rohit::message::Code::SUCCESS:
        os << message.to_string() << std::endl;
        break;

    case rohit::message::Code::COMMAND: {
        auto &commandMessage = static_cast<const rohit::message::Command &>(message);
        os << commandMessage.to_string();
        break;
    }
    
    default:
        break;
    }
    
    return os;
};

std::ostream& operator<<(std::ostream& os, const rohit::message::Command &message) {
    os << message.to_string() << std::endl;
    for (auto &messageEntry: message) {
        os << "\t" << messageEntry << std::endl;
    }
    return os;
}

namespace rohit {

const char * message::Base::displayString[] = {
#define MESSAGE_CODE_ENTRY(x) #x,
    MESSAGE_CODE_LIST
#undef MESSAGE_CODE_ENTRY
};

const std::string rohit::message::CommandEntry::to_string() const {
    std::stringstream ss;
    ss << *this;
    return ss.str();
}

} // namespace rohit