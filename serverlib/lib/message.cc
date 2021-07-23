////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iot/message.hh>
#include <sstream>

namespace rohit {

const char * operation_t::displayString[] = {
#define MESSAGE_OPERATION_ENTRY(x) #x,
    MESSAGE_OPERATION_LIST
#undef MESSAGE_OPERATION_ENTRY
};

const char * message_base_t::displayString[] = {
#define MESSAGE_CODE_ENTRY(x) #x,
    MESSAGE_CODE_LIST
#undef MESSAGE_CODE_ENTRY
};

std::ostream& operator<<(std::ostream& os, const command_t &command) {
     return os  << command.device 
                << ":" << command.component 
                << ":" << command.operation 
                << ":" << command.value;
}

const std::string command_t::to_string() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
}

std::ostream& operator<<(std::ostream& os, const message_base_t &message) {
    message_code_t code = message;
    if (code > message_code_t::COMMAND) {
        return os << "Bad message " << (int) code << std::endl;
    }
    switch(message) {
    case message_code_t::UNKNOWN:
    case message_code_t::SUCCESS:
        os << message.to_string() << std::endl;
        break;

    case message_code_t::COMMAND:
        message_command_t &commandMessage = (message_command_t &)message;
        os << commandMessage;
        break;
    }
    
    return os;
};

std::ostream& operator<<(std::ostream& os, const message_command_t &message) {
    os << ((message_base_t &)message).to_string() << std::endl;
    for (uint32_t index=0; index < message.command_count; ++index) {
        os << "\t" << message.commands[index] << std::endl;
    }
    return os;
}

} // namespace rohit