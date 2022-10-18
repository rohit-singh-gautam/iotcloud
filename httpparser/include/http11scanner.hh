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

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include <http11parser.hh>
#include <location.hh>

namespace rohit {

class http11scanner : public yyFlexLexer{
protected:
   using yyFlexLexer::yy_push_state;
   using yyFlexLexer::yy_pop_state;
   using yyFlexLexer::yy_top_state;

public:
   using yyFlexLexer::yyFlexLexer;
   
   virtual ~http11scanner() {};

   using FlexLexer::yylex;

   int yylex(parser::semantic_type * const lval, parser::location_type *location);

   void BEGIN_REQUEST();

   void END();

private:
   rohit::parser::semantic_type *yylval = nullptr;

};

} // namespace rohit