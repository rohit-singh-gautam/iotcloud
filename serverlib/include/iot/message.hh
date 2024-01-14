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

#include "core/error.hh"
#include "core/guid.hh"
#include <algorithm>
#include <string>

namespace rohit {
namespace message {

#define MESSAGE_CODE_LIST \
    MESSAGE_CODE_ENTRY(UNKNOWN) \
    MESSAGE_CODE_ENTRY(REGISTER) \
    MESSAGE_CODE_ENTRY(CONNECT) \
    MESSAGE_CODE_ENTRY(KEEP_ALIVE) \
    MESSAGE_CODE_ENTRY(UNAUTHORIZED) \
    MESSAGE_CODE_ENTRY(SUCCESS) \
    MESSAGE_CODE_ENTRY(COMMAND) \
    MESSAGE_CODE_ENTRY(BAD_REQUEST) \
    LIST_DEFINITION_END

#define MESSAGE_OPERATION_LIST \
    MESSAGE_OPERATION_ENTRY(SWITCH) \
    MESSAGE_OPERATION_ENTRY(LEVEL) \
    MESSAGE_OPERATION_ENTRY(RED) \
    MESSAGE_OPERATION_ENTRY(GREEN) \
    MESSAGE_OPERATION_ENTRY(BLUE) \
    MESSAGE_OPERATION_ENTRY(GAMMA) \
    LIST_DEFINITION_END

enum class Code : uint16_t {
#define MESSAGE_CODE_ENTRY(x) x,
    MESSAGE_CODE_LIST
#undef MESSAGE_CODE_ENTRY
};

namespace Operation {
// Fixed size structure
typedef uint16_t internal_type;
typedef uint16_t value_internal_type;

enum class Switch : value_internal_type {
    OFF,
    ON,
};

enum class Code : internal_type {
#define MESSAGE_OPERATION_ENTRY(x) x,
    MESSAGE_OPERATION_LIST
#undef MESSAGE_OPERATION_ENTRY
};

constexpr const char *Code_displayString[]
{
#define MESSAGE_OPERATION_ENTRY(x) #x,
    MESSAGE_OPERATION_LIST
#undef MESSAGE_OPERATION_ENTRY
};

const std::string Code_displayStdString[]
{
#define MESSAGE_OPERATION_ENTRY(x) { #x },
    MESSAGE_OPERATION_LIST
#undef MESSAGE_OPERATION_ENTRY
};

inline const std::string &to_string(const Code &value) {
    const auto index = static_cast<internal_type>(value);
    constexpr auto max_index = std::size(Code_displayStdString);
    if (index < max_index)
        return Code_displayStdString[index];
    else return Code_displayStdString[0];
}

} // namespace operation

struct CommandEntry {
private:
    guid_t                          device;
    int32_t                         component;
    Operation::Code                 operation;
    Operation::value_internal_type  value;

public:
    constexpr CommandEntry() {}
    constexpr CommandEntry(const CommandEntry &command) : 
            component(command.component), operation(command.operation), value(command.value) {}
    constexpr CommandEntry(
            const guid_t &device,
            const int32_t component,
            const Operation::Code &operation,
            const Operation::value_internal_type &value)
                    : device(device), component(component), operation(operation), value(value) {}

    constexpr CommandEntry& operator=(const CommandEntry &command) {
        device      = command.device;
        component   = command.component;
        operation   = command.operation;
        value       = command.value;
        return *this;
    }

    constexpr auto &GetDevice() const { return device; }
    constexpr auto GetComponent() const { return component; }
    constexpr auto GetOperation() const { return operation; }
    constexpr auto GetOperationData() const { return value; }
    
    // Not require to be optimised
    const std::string to_string() const;
} __attribute__((packed));

struct Command;

struct Base {
protected:
    const Code code;
    constexpr Base(Code code) : code(code) { }

    static const char * displayString[];

public:
    constexpr Code getMessageCode() const {return code; }

    constexpr bool operator==(const Base rhs) const { return code == rhs.code; }
    constexpr bool operator!=(const Base rhs) const { return code != rhs.code; }
    constexpr bool operator==(const Code rhs) const { return code == rhs; }
    constexpr bool operator!=(const Code rhs) const { return code != rhs; }

    constexpr operator Code() const { return code; }

    inline const std::string to_string() const {
        return displayString[(size_t)code];
    }

    inline operator const std::string() const { return to_string(); }
} __attribute__((packed));

struct Unknown : public Base {
public:
    constexpr Unknown() : Base(Code::UNKNOWN) { }
} __attribute__((packed));

struct BadRequest : public Base {
public:
    constexpr BadRequest() : Base(Code::BAD_REQUEST) { }
} __attribute__((packed));

struct Connect256 : public Base {
private:
    uint8_t     ephemeral_public_key[256/8];
    uint8_t     iv[96/8];
public:
    constexpr Connect256() : Base(Code::CONNECT) { }
} __attribute__((packed));

struct KeepAlive : public Base {
public:
    constexpr KeepAlive() : Base(Code::KEEP_ALIVE) { }
} __attribute__((packed));

struct Success : public Base {
public:
    constexpr Success() : Base(Code::SUCCESS) { }
} __attribute__((packed));

struct Command : public Base {
public:
    static const constexpr size_t MAX_COMMAND = 16;

private:
    uint32_t     command_count;
    CommandEntry commands[MAX_COMMAND];

public:
    constexpr Command() :
        Base(Code::COMMAND),
        command_count(0),
        commands() {}

    template <typename VALUE_TYPE>
    constexpr err_t add(
            const guid_t &device,
            const int32_t component,
            const Operation::Code &operation, 
            const VALUE_TYPE &value) 
    {
        if (command_count >= MAX_COMMAND) return err_t::MESSAGE_COMMAND_LIMIT_FAILURE;
        commands[command_count++] = CommandEntry { device, component, operation, static_cast<Operation::value_internal_type>(value) };
        return err_t::SUCCESS;
    }

    constexpr size_t length() const { 
        return sizeof(Base) + sizeof(command_count) +
                sizeof(CommandEntry) * command_count;
    }

    bool verify() const { return command_count <= MAX_COMMAND; }

    CommandEntry const *begin() const { return commands; }
    CommandEntry const *end() const { return commands + command_count; }
} __attribute__((packed));

struct Register : public Base {
public:
    uint32_t    model;
};

struct RegisterResponse : public Base {
public:
    uint32_t    model;
    guid_t      device_id;

};

} //namespace message
} //namespace rohit


std::ostream& operator<<(std::ostream& os, const rohit::message::CommandEntry &command);
std::ostream& operator<<(std::ostream& os, const rohit::message::Command &message);
std::ostream& operator<<(std::ostream& os, const rohit::message::Operation::Code &operation);
std::ostream& operator<<(std::ostream& os, const rohit::message::Base &message);

namespace rohit {
    using ::operator<<;
namespace message {
    using ::operator<<;
} //namespace message
} //namespace rohit