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

# Tell automake to put the object file for src/fidi_*.cpp in dir src/
AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS =

AM_CPPFLAGS = -pedantic -Wall -Wextra -Wundef -Wdate-time -D_FORTIFY_SOURCE=2       \
              -Dgets=DONT_USE_GETS -Dlint

OPT_FLAGS = -g -O2

AM_CFLAGS = -std=c11 -pedantic -Wextra  -Wall -Wformat=2 -Wno-format-extra-args     \
	    -Wformat-overflow=2 -Winit-self -Wmissing-include-dirs -Wnested-externs \
	    -Wmissing-declarations -Wmissing-prototypes  -Wswitch-default           \
	    -Wswitch-enum  -Wunused -Wstrict-overflow=5 -Wfloat-equal -Wundef       \
	    -Wnull-dereference -Wuninitialized -Wchkp -Wshadow  -Wrestrict          \
	    -Wunsafe-loop-optimizations -Wbad-function-cast -Wcast-qual             \
	    -Wredundant-decls -Wcast-align -Wwrite-strings -Wconversion -fno-common \
	    -Waggregate-return -Wpacked -Wpadded -Wunreachable-code -Winline -Wvla  \
	    -Wstack-protector -fstack-protector -Wlogical-op -Wold-style-definition \
	    -Wunsuffixed-float-constants -Wstrict-prototypes -fshort-enums          \
	    $(OPT_FLAGS)

AM_CXXFLAGS = -std=c++17 -Wconversion -Wshadow -Wcast-align  -Wcast-qual            \
              -Wwrite-strings -Wmissing-declarations -Wpointer-arith                \
              -Wnon-virtual-dtor -Wctor-dtor-privacy -Wstrict-null-sentinel         \
              -Wfloat-equal -Wuninitialized -Woverloaded-virtual                    \
              -Wunused-local-typedefs -fshort-enums -fno-common                     \
	      -fstack-protector-strong  -Wformat -Werror=format-security            \
	      $(OPT_FLAGS)

EXTRA_CPP_WARNINGS = -Weffc++ -Wold-style-cast

ACLOCAL_AMFLAGS = -I third_party/autoconf-archive --install

CLEANFILES     = src/config.h src/.dirstamp src/stamp-h1 position.hh location.hh   \
                 src/*~ src/fidi_scanner.cc src/fidi_scanner.cc src/fidi_parser.hh \
                 src/fidi_parser.cc src/cscope.out libtool stack.hh                \
                 docs/doxyfile.stamp src/fidi_scanner.ccc config.log
DISTCLEANFILES = aclocal.m4 configure src/config.h.in Makefile.in

include src/SubMakefile.am
include docs/SubMakefile.am

clean-local:
	rm -rf $(top_srcdir)/docs/html $(top_srcdir)/docs/latex

distclean-local:
	rm -rf $(top_srcdir)/.deps $(top_srcdir)/src/.deps
	rm -rf $(top_srcdir)/autom4te.cache $(top_srcdir)/build-aux
