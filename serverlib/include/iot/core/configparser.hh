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

#include <iot/core/types.hh>
#include <unordered_map>
#include <string>
#include <assert.h>

namespace rohit {

struct commandline_option {
    const char ch;
    const std::string name;
    const std::string value_help;
    const std::string help;
    const type_identifier type_id;
    void *const variable;
    const bool required;

    template <typename T>
    commandline_option(
        const char ch,
        const char *name,
        const char *value_help,
        const char *help,
        T &variable,
        T default_value,
        bool required = false)
        : ch(ch), name(name), value_help(value_help), help(help), type_id(what_type<T>::value),
            variable(&variable), required(required) {
        assert(ch != '\0' || name != nullptr);
        variable = default_value;
    }

    template <typename T>
    commandline_option(
        const char *name,
        const char *value_help,
        const char *help,
        T &variable,
        T default_value,
        bool required = false)
        : ch('\0'), name(name), value_help(value_help), help(help), type_id(what_type<T>::value),
            variable(&variable), required(required) {
        assert(name != nullptr);
        variable = default_value;
    }

    template <typename T>
    commandline_option(
        const char ch,
        const char *name,
        const char *value_help,
        const char *help,
        T &variable)
        : ch(ch), name(name), value_help(value_help), help(help), type_id(what_type<T>::value),
            variable(&variable), required(true) {
        assert(ch != '\0' || name != nullptr);
    }

    template <typename T>
    commandline_option(
        const char *name,
        const char *value_help,
        const char *help,
        T &variable)
        : ch('\0'), name(name), value_help(value_help), help(help), type_id(what_type<T>::value),
            variable(&variable), required(true) {
        assert(name != nullptr);
    }

    inline commandline_option(
        const char ch,
        const char *name,
        const char *help,
        bool &variable)
        : ch(ch), name(name), value_help(), help(help), type_id(type_identifier::bool_t),
            variable(&variable), required(false) {
        assert(ch != '\0' || name != nullptr);
        variable = false;
    }

    inline commandline_option(
        const char *name,
        const char *help,
        bool &variable)
        : ch('\0'), name(name), value_help(), help(help), type_id(type_identifier::bool_t),
            variable(&variable), required(false) {
        assert(name != nullptr);
        variable = false;
    }

    inline const std::string get_short_description() const {
        std::string ret;
        if (!required) ret += "[";
        if (ch != '\0') {
            ret += "-";
            ret.push_back(ch);
            if (!name.empty()) ret += "/";
        }
        if (!name.empty()) ret += "--" + name;
        if (!value_help.empty() && type_id != type_identifier::bool_t) {
            ret += " <";
            ret += value_help;
            ret += ">";
        }
        if (!required) ret += "]";
        ret += " ";
        return ret;
    }

    inline const std::string get_parameter_description() const {
        std::string ret = "\t" + get_short_description();
        ret += "\n\t\t" + help + "\n";
        return ret;
    }
};

class commandline {
private:
    std::string name;
    const std::string short_description;
    const std::string summary;
    std::unordered_map<std::string, commandline_option *> options_name;
    std::unordered_map<char, commandline_option *> options_ch;
    std::vector<commandline_option> options;

    bool help;

public:
    commandline(
        const char *short_description,
        const char *summary,
        std::initializer_list<commandline_option> options_in) 
            : short_description(short_description), summary(summary), options(options_in)
    {
        options.push_back({'h', "help", "Display this help", help});
        for(commandline_option &value: options) {
            if (!value.name.empty()) options_name.insert(std::make_pair(value.name, &value));
            if (value.ch != '\0') options_ch.insert(std::make_pair(value.ch, &value));
        }
    }

    bool parser(int argc, char *argv[]);

    const std::string usage() const;
    const std::string get_name() const {
        return name;
    }

};

}