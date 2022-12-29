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

#include <iot/core/log.hh>
#include <iot/config/message.hh>
#include <iot/core/configparser.hh>
#include <iot/core/version.h>

int main(int argc, char *argv[]) {
    std::string logconfig {};
    bool listmodule {false};
    bool listlevel {false};
    bool display_version {false};
    rohit::commandline param_parser(
        "Configure Server",
        "This tool is used to configure all device server",
        {
            {'c', "config", "module:level list", "Comma separated list of module:level, module=all for all module, level can be number or name, e.g. SYSTEM:6 or system:debug", logconfig},
            {'m', "module", "List module that configure", listmodule},
            {'l', "level", "List log level", listlevel},
            {'v', "version", "Display version", display_version}
        }
    );

    if (!param_parser.parser(argc, argv)) {
        std::cout << param_parser.usage() << std::endl;
        return EXIT_SUCCESS;
    }

    if (display_version) {
        std::cout << param_parser.get_name() << " " << IOT_VERSION_MAJOR << "." << IOT_VERSION_MINOR << std::endl;
    }

    if (listmodule) {
        std::cout << "List of Modules are: \n";
        size_t index {0};
        for(; index < static_cast<size_t>(rohit::module_t::MAX_MODULE); ++index) {
            std::cout << "ID " << index << " - " << rohit::module_t_string[index] << '\n';
        }
        std::cout << "ID " << index << " - all\n";
    }

    if (listlevel) {
        std::cout << "List of levels are: \n";
        size_t index {0};
        for(auto levelname: rohit::logger_level_string) {
            std::cout << "ID " << index << " - " << levelname << '\n';
            ++index;
        }
    }


    return 0;
}