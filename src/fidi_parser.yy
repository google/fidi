/*fid			i_parser.yy -*- mode: bison -*- */
/**
 * \file fidi_parser.yy Contains the parser source
 */

/* Copyright 2018-2019 Google LLC
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
 */

/*Code:*/

/* Require bison 3.0 or later */
%require "3.0"

 /* Use te C++ skeleton file */
%skeleton "lalr1.cc"

 /* Allow the parsing to be traced at will. Add debug output code to
  * generated parser. disable this for release versions. */
%define parse.trace

 /* write out a header file containing the token defines */
%defines

/* Set the namespace the parser is defined in */
%define api.namespace {fidi}

/* set the parser's class identifier */
%define parser_class_name {Parser}
/* increase verbosity of error messages */
%define parse.error verbose
/* Keep track of locations */
%locations

%code requires{
  /** \file
   *  \ingroup inputhandling
   *
   * This file defines the grammar for handling the fidi (φίδι) input
   * request. This sets the name of the parser class, and configures the
   * parser to be called with pointers to the scanner and the parser
   * driver for the common data structures.
   *
   * The grammar is also pretty simple. Each statement either defines a
   * node (φίδι instance parameters [either URL or host/port pair]), or
   * it defines an originating (source) request, and the cascade of
   * requests that hapen in response.
   *
   */

   namespace fidi {
     class FidiFlexLexer;
     class Driver;
   }
 #include <map>
}

/* Pass in the scanner and the driver as parameters */
%parse-param { FidiFlexLexer &scanner  }
%parse-param { Driver        &driver  }

%code{
   #include <iostream>
   #include <sstream>
   #include <cstdlib>
   #include <fstream>
   #include <string>

#include "src/config.h"

/* include for all driver functions */
#include "src/fidi_driver.h"

#undef yylex
#define yylex scanner.yylex
}

/* Use full object classes for the parser */
%define api.value.type variant
%define parse.assert

/*** BEGIN FIDI - Change the fidi grammar's tokens below ***/
%type  <std::string>                        name
%type  <std::string>                        value
%type  <std::pair<int,int>>                 edgeattr
%type  <std::pair<std::string,std::string>> attr
%type  <std::map<std::string,std::string>>  attrlist
%type  <std::map<std::string,std::string>>  inputlist
%type  <int>                                sequencerule
%type  <int>                                repeatrule

%token                                      END    0     "end of file"
%token                                      OBRACKET
%token                                      CBRACKET
%token                                      EQUALS
%token                                      COMMA
%token                                      DASH
%token                                      ARROW
%token                                      SOURCE
%token                                      REPEAT
%token                                      SEQUENCE
%token <std::string>                        IDENT
%token <std::string>                        STRING
%token <std::string>                        BLOB
%token <int>                                NUMBER
/*** END FIDI - Change the fidi grammar's tokens above ***/

%% /*** Grammar ***/
%start rulelist;
rulelist:       %empty                /*Nothing*/
        |       rulelist rule
        |       rulelist error         {driver.nerrors_++;};
rule:           noderule
        |       toprule;
toprule:        OBRACKET inputlist CBRACKET { driver.HandleTop($2);};
noderule:       name OBRACKET attrlist CBRACKET { driver.HandleNode($1, $3);};
attrlist:       %empty                  {}
        |       attrlist attr           {$1.insert($2); $$ = $1;}
        |       attrlist error          {$$ = $1; driver.nerrors_++;};
attr:           name EQUALS value COMMA {$$.first = $1; $$.second = $3;};
value:          STRING                  {$$ = $1;}
        |       IDENT                   {$$ = $1;}
        |       NUMBER                  {$$ = std::to_string($1);};
name:           IDENT                   {$$ = $1;};
inputlist:      %empty                  {}
        |       inputlist attr          {$1.insert($2); $$ = $1;}
        |       inputlist edgerule      {$$ = $1;}
        |       inputlist error         {$$ = $1; driver.nerrors_++;};
edgerule:       DASH ARROW name edgeattr BLOB { driver.HandleEdge($3, $4, $5);};
edgeattr:       %empty                  {}
        |       edgeattr  repeatrule    {$$.first  = $2;
                                         if($1.second){$$.second = $1.second;};}
        |       edgeattr  sequencerule  {$$.second = $2;
                                         if($1.first){$$.first = $1.first;};};
repeatrule:     REPEAT    EQUALS NUMBER {$$ = $3;};
sequencerule:   SEQUENCE  EQUALS NUMBER {$$ = $3;};
%%

void
fidi::Parser::error( const location_type &l, const std::string &err_message )
{
  driver.parse_errors_.append(err_message).append(" at ");
  driver.parse_errors_.append(std::to_string(l.begin.line)).append(".");
  driver.parse_errors_.append(std::to_string(l.begin.column)).append("\n");
}

/*fidi_parser.yy ends here*/
