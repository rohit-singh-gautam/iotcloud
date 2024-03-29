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

%require "3.2"
%debug
%language "C++"

%define api.namespace { rohit }
%define api.parser.class {parser}
%define api.value.type variant
%define parse.error verbose
%locations
%defines

%code requires{
    #include <http11.hh>
    namespace rohit {
        class http11scanner;
        class http11driver;
    }
}

%parse-param { http11scanner &scanner  }
%parse-param { http11driver &driver  }

%code{
#include <http11driver.hh>

#undef  YY_DECL
#define YY_DECL int rohit::http11scanner::yylex(rohit::parser::semantic_type * const lval, rohit::parser::location_type *loc)

#undef yylex
#define yylex scanner.yylex
}

%token                          COLON
%token <http_header_request::METHOD>    METHOD
%token <http_header::VERSION>   VERSION
%token <http_header::FIELD>     FIELD
%token <std::string>            FIELD_CUSTOM
%token <std::string>            FIELD_VALUE
%token                          CONNECTION
%token                          UPGRADE
%token <std::string>            HTTP_SETTINGS
%token <std::string>            PATH
%token                          NEWLINE
%token <std::string>            IPADDRESS
%token                          SPACE
%token <int>                    INTEGER
%token <std::string>            TEXT
%token <char>                   CHAR
%token                          END                 0               "EOF"

%%

start:
    request_header
;

request_header:
    request_line_end | request_line fields NEWLINE { YYACCEPT; }
;

request_line:
    METHOD SPACE PATH SPACE VERSION NEWLINE { 
        driver.header.method = $1;
        driver.header.fields.insert(std::make_pair(http_header::FIELD::Path, $3));
        driver.header.version = $5;
    }
;

request_line_end:
    METHOD SPACE PATH SPACE VERSION NEWLINE NEWLINE { 
        driver.header.method = $1;
        driver.header.fields.insert(std::make_pair(http_header::FIELD::Path, $3));
        driver.header.version = $5;
        YYACCEPT;
    }
;

fields:
    field | fields field
;

field:
    standard_field | custom_field
;

standard_field:
    FIELD FIELD_VALUE NEWLINE     { driver.header.fields.insert(std::make_pair($1, $2)); }
;

custom_field:
    FIELD_CUSTOM FIELD_VALUE NEWLINE { /* Ignoring */ }

%%

void 
rohit::parser::error( const location_type &l, const std::string &err_message )
{
   std::cerr << "Error: " << err_message << " at " << l << std::endl;
}