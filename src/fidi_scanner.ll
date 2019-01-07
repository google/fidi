/* fidi_scanner.hh ---  -*- mode: bison; -*-
 *
 * Copyright 2018-2019 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Code:
 */

%{
  /** \file
   *  \ingroup inputhandling
   *
   * This file defines the scanner for handling the fidi (φίδι) input
   * request. This is a pretty simple scanner, apart from needing to
   * handle the recursive definition of downstream calls as a blob, and
   * not parse it fully. While handling the blob token we need to count
   * brackets, incrementing the bracket count on open brackets, and
   * decrementing it when we encounter a close bracket, and finishing
   * off the blob token when the open bracket count reaches zero.
   */

#include <string>
#include "src/config.h"

/* Implementation of yyFlexFidiFlexLexer */
#include "src/fidi_flex_lexer.h"
#undef  YY_DECL
#define YY_DECL int fidi::FidiFlexLexer::yylex( fidi::Parser::semantic_type * const lval, fidi::Parser::location_type *loc )

/* typedef to make the returns for the tokens shorter */
using token = fidi::Parser::token;
typedef fidi::Parser::token token;
typedef fidi::Parser::token_type token_type;


/* define yyterminate as this instead of NULL */
#define yyterminate() return( token::END )

/* update location on matching */
#define YY_USER_ACTION loc->step(); loc->columns(yyleng);

%}

/*** Flex Declarations and Options ***/
/* enable c++ scanner class generation */
%option c++

/* enable scanner to generate debug output. disable this for release
 * versions. */
%option debug

 /* accept 8-bit input  */
%option 8bit
 /* warn about inconsistencies */
%option warn
/* don't create default echo-all rule
%option nodefault */

%option yyclass="fidi::FidiFlexLexer"

/*  no support for include files is planned */
%option noyywrap

/* enables the use of start condition stacks */
%option stack

%x EDGEDEF EDGEATTR INBRACKETS

BLANK  [[:blank:]\r\n]
NUMBER [[:digit:]]
STRING \"[^"]*\"
IDENT  [a-zA-Z]([a-zA-Z0-9_]*)

%%

%{          /** Code executed at the beginning of yylex **/
            yylval = lval;
%}

<INITIAL>"["                {return token::OBRACKET;}
<INITIAL>"-"                {return token::DASH;}
<INITIAL>">"                {BEGIN(EDGEDEF); return token::ARROW;}
<INITIAL>"source"           {return token::SOURCE;}

<INITIAL,EDGEDEF>"="        {return token::EQUALS;}
<INITIAL,EDGEDEF>{BLANK}*   {}
<EDGEDEF>"sequence"         {return token::SEQUENCE;}
<EDGEDEF>"repeat"           {return token::REPEAT;}
<INITIAL,EDGEDEF>{IDENT}    {yylval->build< std::string >( yytext );
                             return token::IDENT;}
<EDGEDEF>"["                {BEGIN(EDGEATTR);bracket_count++; }

<EDGEATTR>([^\[\]]|{BLANK})+  yymore();
<EDGEATTR>"["               bracket_count++; yymore();
<EDGEATTR>"]"               {bracket_count--;
                              if(bracket_count > 0) {yymore(); }
                              else{yylval->build< std::string >( yytext );
                                    BEGIN(INITIAL); return token::BLOB; }
                             }
<INITIAL,EDGEDEF>"]"        {return token::CBRACKET;}
<INITIAL,EDGEDEF>","        {return token::COMMA;}

<INITIAL,EDGEDEF>{NUMBER}+  {yylval->build<int>( std::atoi(yytext) );
                             return token::NUMBER;}
<INITIAL>{STRING}           {yylval->build< std::string >( yytext );
                             return token::STRING;}
<INITIAL>. {/* pass all other characters up to bison */
#ifdef HAVE_BISON_WITH_EXCEPTIONS
     throw fidi::Parser::syntax_error(*loc, "Invalid character: "
                               + std::string(yytext, yyleng));
#else
     return static_cast<token_type>(*yytext);
#endif
           }

%%  /*** Additional Code ***/
