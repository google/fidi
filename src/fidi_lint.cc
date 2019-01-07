// -*- mode: c++; -*-
// fidi_lint.cc ---

// Copyright 2018-2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/// \file
/// \ingroup lint
///
/// This is the main file of the fidi (φίδι) lint tool. Here we
/// implement commands line parsing, and then either put out a help
/// screen or create a parser and check the input.

// Code:

#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "src/fidi_lint_driver.h"

/// \brief  Main function
///
/// \details Implement command line parsing. --help or --version are
/// terminal options, in the sense that we print out the help text or
/// version number and then exit. Otherwise, first, create a parser.
/// Secondly, determine if a filename has been provided, or we should
/// read from ther standard input; and then pass the appropriate input
/// stream to the parser, and then perform the syntax and sanity
/// checks.  Finally, it generates a graph description in dot(1)
/// format, which can then be processed to create a graph of the
/// request and the cascade of the resulting requests that it
/// generates, to provide a visual depiction of the requested
/// behaviour.
///
/// \param[in]  argc number of arguments
/// \param[in]  argv An array of character pointers containing the arguments
///
/// \return an integer 0 upon exit success
int
main(const int argc, const char **argv) {
  fidi::LintDriver driver;

  try {
    if (argc == 1) {
      driver.Parse(std::cin);
    } else if (argc == 2) {
      /* simple help menu */
      if ((std::strncmp(argv[1], "-h", 2) == 0) ||
          (std::strncmp(argv[1], "--h", 3) == 0)) {
        std::cout << PACKAGE_NAME << " lint usage\n\n"
                  << "use cat to  pipe a file to std::cin\n"
                  << "or give a filename to validate a file\n"
                  << "    fidi_lint input.txt\n"
                  << "    cat input.txt | fifi_lint\n\n"
                  << "Use -v or --version to get the version\n"
                  << "    fidi_lint -v\n"
                  << "    fidi_lint --version\n\n"
                  << "use -h or --help to get this menu\n";
        return (EXIT_SUCCESS);
      }

      if ((std::strncmp(argv[1], "-v", 2) == 0) ||
          (std::strncmp(argv[1], "--v", 3) == 0)) {
        std::cout << PACKAGE_NAME << " version " << PACKAGE_VERSION << "\n";
        return (EXIT_SUCCESS);
      }

      if (std::strncmp(argv[1], "-", 1) == 0) {
        // Support - as synonym for file stdin
        driver.Parse(std::cin);
      } else {
        struct stat sb;
        if (lstat(argv[1], &sb) == -1) {
          std::cerr << "Unknown file or option: " << argv[1] << std::endl
                    << "Usage\n"
                    << "    fidi_lint input.txt\n"
                    << "    cat input.txt | fifi_lint\n"
                    << "use cat to  pipe to the standard input.\n"
                    << "just give a filename to validate a file\n"
                    << "use -h to get this menu\n";
          return (EXIT_FAILURE);
        }
        driver.Parse(argv[1]);
      }
    } else {
      std::cerr << "Unknown arguments. We expect 0 or 1.\n"
                << "Usage\n"
                << "    fidi_lint input.txt\n"
                << "    cat input.txt | fifi_lint\n"
                << "use cat to  pipe to the standard input.\n"
                << "just give a filename to validate a file\n"
                << "use -h to get this menu\n";
      return (EXIT_FAILURE);
    }
  } catch (std::bad_alloc &ba) {
    std::cerr << "Failed to allocate scanner: (" << ba.what() << ")\n";
    return (EXIT_FAILURE);
  }

  // At this point, we have run the parser
  if (driver.nerrors_ != 0) {
    std::cerr << "Proceeding despite failures. "
              << "The graph is likely inaccurate." << std::endl;
  }
  driver.Execute(std::cout) << std::endl;
  if (driver.nerrors_ != 0) { return (EXIT_FAILURE); }

  return (EXIT_SUCCESS);
}

//
// fidi_lint.cc ends here
