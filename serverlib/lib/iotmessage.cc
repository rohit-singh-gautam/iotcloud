////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#include <iot/message.hh>
#include <sstream>

namespace rohit {

const std::string operation_t::displayString[] = {
    "SWITCH",
    "LEVEL_256",
    "PWM_1024",
};

const std::string message_base_t::displayString[] = {
    "UNKNOWN",
    "SUCCESS",
    "COMMAND",
};

std::ostream& operator<<(std::ostream& os, const command_t &command) {
    os << command.guid << ":" << command.operation << ":";
    switch(command.operation) {
    case operation_t::SWITCH:
        os << (command.value == (operation_value_internal_type)operation_t::operation_switch_t::OFF ? "OFF" : "ON");
        break;
    case operation_t::LEVEL_256:
    case operation_t::PWM_1024:
    default:
        os << command.value;
        break;
    }
    return os;
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
    os << ((message_base_t &)message).to_string() << ":" << message.source << std::endl;
    for (uint32_t index=0; index < message.command_count; ++index) {
        os << "\t" << message.commands[index] << std::endl;
    }
    return os;
}

} // namespace rohit