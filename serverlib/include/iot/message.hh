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
    LIST_DEFINITION_END

enum class message_code_t : uint16_t {
#define MESSAGE_CODE_ENTRY(x) x,
    MESSAGE_CODE_LIST
#undef MESSAGE_CODE_ENTRY
};

// Fixed size structure
typedef uint16_t operation_internal_type;
typedef uint16_t operation_value_internal_type;

enum class operation_switch_t : operation_value_internal_type {
    OFF,
    ON,
};

enum class operation_t : operation_internal_type {
#define MESSAGE_OPERATION_ENTRY(x) x,
    MESSAGE_OPERATION_LIST
#undef MESSAGE_OPERATION_ENTRY
};

constexpr const char *operation_t_displayString[]
{
#define MESSAGE_OPERATION_ENTRY(x) #x,
    MESSAGE_OPERATION_LIST
#undef MESSAGE_OPERATION_ENTRY
};

inline const std::string to_string(const operation_t &value) {
    return { operation_t_displayString[static_cast<operation_internal_type>(value)] };
}


inline std::ostream& operator<<(std::ostream& os, const operation_t &operation) {
    return os << to_string(operation);
}

struct command_t {
private:
    guid_t                          device;
    int32_t                         component;
    operation_t                     operation;
    operation_value_internal_type   value;

public:
    constexpr command_t() {}
    constexpr command_t(const command_t &command) : 
            component(command.component), operation(command.operation), value(command.value) {}
    constexpr command_t(
            const guid_t &device,
            const int32_t component,
            const operation_t &operation,
            const operation_value_internal_type &value)
                    : device(device), component(component), operation(operation), value(value) {}

    constexpr command_t& operator=(const command_t &command) {
        device      = command.device;
        component   = command.component;
        operation   = command.operation;
        value       = command.value;
        return *this;
    }

    operation_t GetOperation() const { return operation; }

    
    // Not require to be optimised
    const std::string to_string() const;
    friend std::ostream& operator<<(std::ostream& os, const command_t &command);
} __attribute__((packed));

std::ostream& operator<<(std::ostream& os, const command_t &command);

struct message_command_t;

struct message_base_t {
protected:
    const message_code_t message_code;
    constexpr message_base_t(message_code_t message_code) : message_code(message_code) { }

    static const char * displayString[];

    friend std::ostream& operator<<(std::ostream& os, const message_base_t &message);
    friend std::ostream& operator<<(std::ostream& os, const message_command_t &message);

public:
    constexpr message_code_t getMessageCode() {return message_code; }

    constexpr bool operator==(const message_base_t rhs) const { return message_code == rhs.message_code; }
    constexpr bool operator!=(const message_base_t rhs) const { return message_code != rhs.message_code; }
    constexpr bool operator==(const message_code_t rhs) const { return message_code == rhs; }
    constexpr bool operator!=(const message_code_t rhs) const { return message_code != rhs; }

    constexpr operator message_code_t() const { return message_code; }

    inline const std::string to_string() const {
        return displayString[(size_t)message_code];
    }

    inline operator const std::string() const { return to_string(); }
} __attribute__((packed));

std::ostream& operator<<(std::ostream& os, const message_base_t &message);

struct message_unknown_t : public message_base_t {
public:
    constexpr message_unknown_t() : message_base_t(message_code_t::UNKNOWN) { }
} __attribute__((packed));

struct message_bad_request_t : public message_base_t {
public:
    constexpr message_bad_request_t() : message_base_t(message_code_t::BAD_REQUEST) { }
} __attribute__((packed));

struct message_connect256_t : public message_base_t {
private:
    uint8_t     ephemeral_public_key[256/8];
    uint8_t     iv[96/8];
public:
    constexpr message_connect256_t() : message_base_t(message_code_t::CONNECT) { }
} __attribute__((packed));

struct message_keep_alive_t : public message_base_t {
public:
    constexpr message_keep_alive_t() : message_base_t(message_code_t::KEEP_ALIVE) { }
} __attribute__((packed));

struct message_success_t : public message_base_t {
public:
    constexpr message_success_t() : message_base_t(message_code_t::SUCCESS) { }
} __attribute__((packed));

struct message_command_t : public message_base_t {
public:
    static const constexpr size_t MAX_COMMAND = 16;

private:
    uint32_t        command_count;
    command_t       commands[MAX_COMMAND];

public:
    constexpr message_command_t() :
        message_base_t(message_code_t::COMMAND),
        command_count(0),
        commands() {}

    template <typename VALUE_TYPE>
    constexpr err_t add(
            const guid_t &device,
            const int32_t component,
            const operation_t &operation, 
            const VALUE_TYPE &value) 
    {
        if (command_count >= MAX_COMMAND) return err_t::MESSAGE_COMMAND_LIMIT_FAILURE;
        commands[command_count++] = command_t(device, component, operation, (operation_value_internal_type)value);
        return err_t::SUCCESS;

    }

    constexpr size_t length() const { 
        return sizeof(message_base_t) + sizeof(command_count) +
                sizeof(command_t) * command_count;
    }

    bool verify() const { return command_count <= MAX_COMMAND; }

    command_t const *begin() const { return commands; }
    command_t const *end() const { return commands + command_count; }
    
    friend std::ostream& operator<<(std::ostream& os, const message_command_t &message);
} __attribute__((packed));

std::ostream& operator<<(std::ostream& os, const message_command_t &message);

struct message_register_device_t : public message_base_t {
public:
    uint32_t    model;
};

struct message_register_response_device_t : public message_base_t {
public:
    uint32_t    model;
    guid_t      device_id;

};

}