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

#include <iot/core/log.hh>

namespace rohit {
#define CONFIG_LIST \
    CONFIG_ENTRY(CONFIG_LOG, "log") \
    LIST_DEFINITION_END

enum class config_t : uint16_t {
#define CONFIG_ENTRY(x, y) x,
        CONFIG_LIST
#undef CONFIG_ENTRY
};

constexpr const char *config_t_string[] = {
#define CONFIG_ENTRY(x, y) {y},
    CONFIG_LIST
#undef CONFIG_ENTRY
};

struct config_log {
    module_t module;
    logger_level level;
};

} //namespace rohit