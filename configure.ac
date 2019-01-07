#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Copyright 2018-2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

AC_PREREQ([2.69])
AC_INIT([fidi], [1.0.0], [srivasta@google.com])
AC_CONFIG_SRCDIR([src/fidi_app_caller.cc])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_AUX_DIR([build-aux])

dnl Use a m4 macro directory
AC_CONFIG_MACRO_DIRS([third_party/autoconf-archive])

m4_include([third_party/autoconf-archive/ax_compare_version.m4])
m4_include([third_party/autoconf-archive/ax_prog_bison_version.m4])

# Checks for programs.
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_YACC
AM_CONDITIONAL([HAVE_BISON], [test "$YACC" = 'bison -y'])
AC_PROG_LEX
AC_PROG_RANLIB
AM_PROG_AR

AC_PROG_SED
AC_PROG_GREP

AC_PATH_PROG([BISON],[bison])
AX_PROG_BISON_VERSION([3.2.0],
  [AC_DEFINE([HAVE_BISON_WITH_EXCEPTIONS], [1],
  [Define to 1 if you can use syntax_error exceptions in the scanner])],
  [])

# Do we have Doxygen for th docs?
AC_CHECK_PROGS([DOXYGEN], [doxygen])
if test -z "$DOXYGEN";
   then AC_MSG_WARN([Doxygen not found - continuing without Doxygen support])
fi
# If so, generate the Doxygen file
AM_CONDITIONAL([HAVE_DOXYGEN],
[test -n "$DOXYGEN"])AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([docs/Doxyfile])])

# Checks for libraries.

AC_LANG_PUSH(C++)
AC_CHECK_HEADERS([Poco/URI.h])
AC_CHECK_HEADERS([Poco/Util/Units.h])
AC_CHECK_HEADERS([Poco/Net/HTTPStreamFactory.h])

saved_ldflags="${LDFLAGS}"
LDFLAGS="${LDFLAGS} -lPocoFoundation"
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <Poco/URI.h>
#include <iostream>
using Poco::URI;]],
  [[URI uri1("http://www.google.com/");
  std::cout << "Scheme:    " << uri1.getScheme() << std::endl;
  URI uri2; uri2.setScheme("https")]])],
    [LIBS="$LIBS -lPocoFoundation"],
    [AC_MSG_FAILURE([Poco Foundation Library is not installed.])])
LDFLAGS="${saved_ldflags}"

saved_ldflags="${LDFLAGS}"
LDFLAGS="${LDFLAGS} -lPocoNet"
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <Poco/Net/HTTPStreamFactory.h>
using Poco::Net::HTTPStreamFactory;;]],
  [[HTTPStreamFactory::registerFactory();]])],
    [LIBS="$LIBS -lPocoNet"],
    [AC_MSG_FAILURE([Poco Net Library is not installed.])])
LDFLAGS="${saved_ldflags}"

saved_ldflags="${LDFLAGS}"
LDFLAGS="${LDFLAGS} -lPocoUtil"
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <Poco/Util/Units.h>
#include <iostream>
using namespace Poco::Util::Units::Constants;
using namespace Poco::Util::Units::Values;]],
  [[std::cout << "One mile is " << km(mile(1))   << std::endl;]])],
    [LIBS="$LIBS -lPocoUtil"],
    [AC_MSG_FAILURE([Poco Util Library is not installed.])])
LDFLAGS="${saved_ldflags}"


# Checks for header files.
AC_CHECK_HEADERS([unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK

dnl Enable Automake, do not insist on Authors and ChangeLog and such
AM_INIT_AUTOMAKE([1.13  -Wall -Werror])
AC_CONFIG_FILES([Makefile])

# Required. Do final output
AC_OUTPUT
