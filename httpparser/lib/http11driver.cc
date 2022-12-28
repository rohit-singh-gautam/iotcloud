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

#include <http11driver.hh>
#include <iot/core/config.hh>
#include <sstream>

rohit::http11driver::~http11driver()
{
   delete(scanner);
   scanner = nullptr;
   delete(parser);
   parser = nullptr;
}

rohit::err_t rohit::http11driver::parse(std::string &text) {
    std::stringstream textstream(text);
    return parse_internal(textstream);
}

rohit::err_t rohit::http11driver::parse_internal(std::istream &textstream) {
    delete(scanner);
    try
    {
        scanner = new http11scanner( &textstream );
    }
    catch( std::bad_alloc &exception )
    {
        if (rohit::config::debug) {
            std::cerr << "Failed to allocate scanner: (" <<
                exception.what() << ")\n";
        }
        throw rohit::exception_t(rohit::err_t::HTTP11_PARSER_MEMORY_FAILURE);
    }

    delete(parser); 
    try
    {
        parser = new rohit::parser(*scanner, *this);
    }
    catch( std::bad_alloc &ba )
    {
        std::cerr << "Failed to allocate parser: (" << 
            ba.what() << "), exiting!!\n";
        exit( EXIT_FAILURE );
    }

    scanner->BEGIN_REQUEST();
    const int accept( 0 );
    if( parser->parse() != accept )
    {
        return rohit::err_t::HTTP11_PARSER_FAILURE;
    }
    return rohit::err_t::SUCCESS;
}

