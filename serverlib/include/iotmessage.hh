#pragma once

#include "iotclouderror.hh"
#include <algorithm>
#include <string>

namespace iotcloud {

constexpr uint8_t hexToValue(const char c) {
    return c > '9' ? c - 'a' + 10 : c - '0';
}

constexpr char valueToHex(const uint8_t c) {
    return c > 9 ? c + 'a' - 10 : c + '0';
}

class guid_t {
private:
    static const constexpr std::size_t guid_size = 16;
    static const constexpr std::size_t guid_string_size = 36;
    uint8_t guid_store[guid_size];

    inline error_t toStore(const char *guid) {
        size_t index = 0;
        size_t guidIndex = 0;
        // f81d4fae-7dec-11d0-a765-00a0c91e6bf6
        for(;index < 4;) {
            guid_store[index++] = hexToValue(guid[guidIndex]) * 16 + hexToValue(guid[guidIndex+1]);
            guidIndex += 2;
        }

        if (guid[guidIndex++] != '-') return error_t::GUID_BAD_STRING_FAILURE;

        for(;index < 6;) {
            guid_store[index++] = hexToValue(guid[guidIndex]) * 16 + hexToValue(guid[guidIndex+1]);
            guidIndex += 2;
        }

        if (guid[guidIndex++] != '-') return error_t::GUID_BAD_STRING_FAILURE;

        for(;index < 8;) {
            guid_store[index++] = hexToValue(guid[guidIndex]) * 16 + hexToValue(guid[guidIndex+1]);
            guidIndex += 2;
        }

        if (guid[guidIndex++] != '-') return error_t::GUID_BAD_STRING_FAILURE;

        for(;index < 10;) {
            guid_store[index++] = hexToValue(guid[guidIndex]) * 16 + hexToValue(guid[guidIndex+1]);
            guidIndex += 2;
        }

        if (guid[guidIndex++] != '-') return error_t::GUID_BAD_STRING_FAILURE;

        for(;index < 16;) {
            guid_store[index++] = hexToValue(guid[guidIndex]) * 16 + hexToValue(guid[guidIndex+1]);
            guidIndex += 2;
        }

        return error_t::SUCCESS;
    }

public:
    inline guid_t() {}
    inline guid_t(const uint8_t *guid_binary) { std::copy(guid_binary, guid_binary + guid_size, guid_store); }

    // Below function is mosty for testing
    inline guid_t(const std::string &guid) {
        if (guid.length() != 36) throw exception_t(exception_t::GUID_BAD_STRING_FAILURE);
        error_t err = toStore(guid.c_str());
        if (err != error_t::SUCCESS) throw exception_t(err);
    }

    inline guid_t(const char *guid) {
        error_t err = toStore(guid);
        if (err != error_t::SUCCESS) throw exception_t(err);
    }

    inline bool operator==(const guid_t rhs) const { 
        for(size_t index = 0; index < guid_size; index++) {
            if (guid_store[index] != rhs.guid_store[index]) return false;
        }
        return true;
    }

    inline bool operator!=(const guid_t rhs) const { return !(*this != rhs); }

    inline const std::string to_string() const {
        std::string str(guid_string_size, '\0');

        size_t index = 0;
        size_t guidIndex = 0;

        for(;index < 4;) {
            str[guidIndex++] = valueToHex((guid_store[index] & 0xf0) >> 4);
            str[guidIndex++] = valueToHex(guid_store[index++] & 0x0f);
        }

        str[guidIndex++] = '-';

        for(;index < 6;) {
            str[guidIndex++] = valueToHex((guid_store[index] & 0xf0) >> 4);
            str[guidIndex++] = valueToHex(guid_store[index++] & 0x0f);
        }

        str[guidIndex++] = '-';

        for(;index < 8;) {
            str[guidIndex++] = valueToHex((guid_store[index] & 0xf0) >> 4);
            str[guidIndex++] = valueToHex(guid_store[index++] & 0x0f);
        }

        str[guidIndex++] = '-';

        for(;index < 10;) {
            str[guidIndex++] = valueToHex((guid_store[index] & 0xf0) >> 4);
            str[guidIndex++] = valueToHex(guid_store[index++] & 0x0f);
        }

        str[guidIndex++] = '-';

        for(;index < 16;) {
            str[guidIndex++] = valueToHex((guid_store[index] & 0xf0) >> 4);
            str[guidIndex++] = valueToHex(guid_store[index++] & 0x0f);
        }

        return str;
    }

    inline operator const std::string() const { return to_string(); }
};

inline std::ostream& operator<<(std::ostream& os, const guid_t &guid) {
    return os << guid.to_string();
}


// Fixed size structure
typedef uint16_t operation_internal_type;
typedef uint16_t operation_value_internal_type;

class operation_t {
public:
    enum operation_internal_t : operation_internal_type {
        /*CONNECTION_ESTABLISH,
        KEEP_LIVE,
        REGISTER,
        REMOVE,*/
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

    inline error_t add(const guid_t &source, const operation_t &operation, const operation_value_internal_type &value) {
        if (command_count >= MAX_COMMAND) return error_t::MESSAGE_COMMAND_LIMIT_FAILURE;
        commands[command_count++] = command_t(source, operation, value);
        return error_t::SUCCESS;
    }

    inline size_t length() { 
        return sizeof(message_base_t) + sizeof(command_count) + sizeof(source) +
                sizeof(command_t) * command_count;
    }
    
    friend std::ostream& operator<<(std::ostream& os, const message_command_t &message);
};

std::ostream& operator<<(std::ostream& os, const message_command_t &message);

}