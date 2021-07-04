////////////////////////////////////////////////////////////
// Copyright 2021 Rohit Jairaj Singh (rohit@singh.org.in) //
// Private file do not read, copy, share or distribute    //
////////////////////////////////////////////////////////////

#pragma once

#include "core/error.hh"
#include "core/guid.hh"
#include <algorithm>
#include <string>

namespace rohit {

// Fixed size structure
typedef uint16_t operation_internal_type;
typedef uint16_t operation_value_internal_type;

class operation_t {
public:
    enum operation_internal_t : operation_internal_type {
        SWITCH,
        LEVEL_256,
        PWM_1024,
    };

    enum class operation_switch_t : operation_value_internal_type {
        OFF,
        ON,
    };

private:
    operation_internal_type value;
    static const std::string displayString[];

public:
    inline operation_t() {}
    inline operation_t(const operation_t &operation) : value(operation.value) {}
    inline operation_t(const operation_internal_t operation) : value(operation) {}

    inline constexpr operation_t& operator=(const operation_t &operation) {
        this->value = operation.value;
        return *this;
    }

    inline constexpr operator operation_internal_t() const { return static_cast<operation_internal_t>(value); }

    inline const std::string to_string() const {
        return displayString[value];
    }

    inline operator const std::string() const { return to_string(); }
};

inline std::ostream& operator<<(std::ostream& os, const operation_t &operation) {
    return os << operation.to_string();
}

class command_t {
private:
    guid_t      guid;
    operation_t operation;
    operation_value_internal_type  value;

public:
    inline command_t() {}
    inline command_t(const command_t &command) : guid(command.guid), operation(command.operation), value(command.value) {}
    inline command_t(const guid_t &guid, const operation_t &operation, const operation_value_internal_type &value)
                    : guid(guid), operation(operation), value(value) {}

    constexpr command_t& operator=(const command_t &command) {
        guid = command.guid;
        operation = command.operation;
        value = command.value;
        return *this;
    }
    
    // Not require to be optimised
    const std::string to_string() const;
    friend std::ostream& operator<<(std::ostream& os, const command_t &command);
};

std::ostream& operator<<(std::ostream& os, const command_t &command);

enum class message_code_t : uint16_t {
    UNKNOWN,
    CONNECT,
    KEEP_ALIVE,
    UNAUTHORIZED,
    SUCCESS,
    COMMAND,
};

class message_command_t;

class message_base_t {
protected:
    const message_code_t message_code;
    inline constexpr message_base_t(message_code_t message_code) : message_code(message_code) { }

    static const std::string displayString[];

    friend std::ostream& operator<<(std::ostream& os, const message_base_t &message);
    friend std::ostream& operator<<(std::ostream& os, const message_command_t &message);

public:
    inline constexpr message_code_t getMessageCode() {return message_code; }

    inline bool operator==(const message_base_t rhs) const { return message_code == rhs.message_code; }
    inline bool operator!=(const message_base_t rhs) const { return message_code != rhs.message_code; }
    inline bool operator==(const message_code_t rhs) const { return message_code == rhs; }
    inline bool operator!=(const message_code_t rhs) const { return message_code != rhs; }

    inline constexpr operator message_code_t() const { return message_code; }

    inline const std::string to_string() const {
        return displayString[(size_t)message_code];
    }

    inline operator const std::string() const { return to_string(); }
};

std::ostream& operator<<(std::ostream& os, const message_base_t &message);

class message_unknown_t : public message_base_t {
public:
    inline constexpr message_unknown_t() : message_base_t(message_code_t::UNKNOWN) { }
};

class message_keep_alive_t : public message_base_t {
public:
    inline constexpr message_keep_alive_t() : message_base_t(message_code_t::KEEP_ALIVE) { }
};

class message_success_t : public message_base_t {
public:
    inline constexpr message_success_t() : message_base_t(message_code_t::SUCCESS) { }
};

class message_command_t : public message_base_t {
public:
    static const constexpr size_t MAX_COMMAND = 16;

private:
    uint32_t command_count;
    guid_t source;
    command_t commands[MAX_COMMAND];

public:
    inline message_command_t() : message_base_t(message_code_t::COMMAND) {}
    inline message_command_t(const guid_t &source)
        : message_base_t(message_code_t::COMMAND), command_count(0), source(source) {}
    inline message_command_t(const char *guid_string)
        : message_base_t(message_code_t::COMMAND), command_count(0), source(to_guid(guid_string)) {}

    inline err_t add(const guid_t &source, const operation_t &operation, const operation_value_internal_type &value) {
        if (command_count >= MAX_COMMAND) return err_t::MESSAGE_COMMAND_LIMIT_FAILURE;
        commands[command_count++] = command_t(source, operation, value);
        return err_t::SUCCESS;
    }

    inline size_t length() { 
        return sizeof(message_base_t) + sizeof(command_count) + sizeof(source) +
                sizeof(command_t) * command_count;
    }
    
    friend std::ostream& operator<<(std::ostream& os, const message_command_t &message);
};

std::ostream& operator<<(std::ostream& os, const message_command_t &message);

}