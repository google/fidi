// -*- mode: c++; -*-
// fidi_app.cc ---

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
/// \ingroup app
///
/// This is the starting point for the actual fidi (φίδι) HTTP server
/// application. This is a very thin wrapper around the POCO Util’s
/// ServerApplication class, and defers all the work to that class.

// Code:

#include "src/fidi_server_application.h"

/// \brief  Main function
///
/// \details Creates a HTTP server class instance, and passes in the
/// command line options to the classes run method.
///
/// \param[in]  argc number of arguments
/// \param[in]  argv An array of character pointers containing the arguments
///
/// \return an integer that is returned by the HTTP server application.
int
main(const int argc, char **argv) {
  fidi::FidiServerApplication app;
  return app.run(argc, argv);
}

//
// fidi_app.cc ends here
