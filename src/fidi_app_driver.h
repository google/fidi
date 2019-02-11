// fidi_app_driver.h ---  -*- mode: c++; -*-

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
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.  See the License for the specific language governing
// permissions and limitations under the License.

/// \file
/// \ingroup app
///
/// This file implements the parser driver class for the fidi (φίδι)
/// HTTP server application, derived from the base driver class. It
/// holds a referenec to the parser, and provides the parse helper and
/// execute methods.

// Code:

#ifndef FIDI_APP_DRIVER_H
#  define FIDI_APP_DRIVER_H

#  include <chrono>
#  include <mutex>

#  include <Poco/Net/HTTPServerResponse.h>
#  include <Poco/TaskManager.h>
#  include <Poco/ThreadPool.h>

#  include "src/fidi_app_caller.h"
#  include "src/fidi_driver.h"

namespace fidi {

  /// \brief fidi (φίδι) HTTP server driver class
  ///
  /// This class is derived from the Driver class, that does most of
  /// the heavy lifting of parsing. This class adds to the
  /// parse_helper method. The base class just created a new scanner,
  /// here we create a new parser that uses the base class scanner,
  /// and actually perform the parsing.
  ///
  /// This class also provides an implementation of the execute
  /// method, which inspects the internal data structures populated by
  /// the parser and handles the request appropriately. The execute
  /// method also runs sanity checks, and sets the return code to 400
  /// and returns the diagnostics as the response. It also logs the
  /// errors.
  class AppDriver : public Driver {
   public:
    /// The default constructor
    AppDriver() : Driver(), parser_(nullptr), resp_(nullptr) {}

    /// The copy constructor is not used, so declutter.
    AppDriver(const AppDriver &src) = delete;

    /// The assignment operator is also not used, so cleaned up.
    AppDriver &operator=(const AppDriver &src) = delete;

    /// The move operations are implicitly unused, and cleaned up.
    ///
    /// Cleaning up explicitly also prevents derived classes from implementing
    /// the move operations. This is not a problem currently, since these are
    /// not needed, and removing them simplifies the generated code, and reduces
    /// its size.
    AppDriver(AppDriver &&) = delete;
    AppDriver &operator=(AppDriver &&) = delete;

    /// Destructor
    ///
    /// Cleans up the stored references to the scanner and the parser.
    virtual ~AppDriver();

    /// \brief The method where the guts of the work is done.
    ///
    /// The execute method creates a threadpool and a task manager to
    /// handle making downstream calls.
    ///
    /// + If there is a predelay attribute, sleep for the desgnated
    ///   number of millisecons
    /// + Set the response code
    /// + If there are calls to make, walk down the priority queue, and
    ///      - gather all requests at the same priority
    ///      - if there is no utl attribute, create the url from the
    ///        hostname and port
    ///      - Create a new AppCaller object, and pass it to the
    ///        task manager
    ///      - Wait for all tasks to complete
    ///      - repeat until there are no more calls in queue
    /// + If there is a post delay, sleep for the specified
    ///   milliseconds
    ///
    /// \param[in,out] stream output stream.
    std::ostream &Execute(std::ostream &stream);

    /// \brief run the parser in the input stream
    ///
    /// This method first runs the super classes parse_helper method,
    /// which creates a new scanner, and then deletes any existing
    /// parser, and creates a new one using the just created
    /// scanner. It then runs the parser, emitting diagnostics if
    /// parsing did not complete without errors.
    ///
    /// \param[in, out] stream the input stream with the request.
    void ParseHelper(std::istream &stream);

    /// set the response code
    void set_resp(Poco::Net::HTTPServerResponse &resp);
    /// \brief Is the application healthy right now?
    /// \return boolean true if the application is healthy
    bool get_health(void);

    /// \brief Is the application responding right now?
    /// \return boolean true if the application is responsive
    bool IsResponsive(void);

   private:
    fidi::Parser *parser_ = nullptr;  ///< A reference to the parser
                                      ///< created for handling this
                                      ///< request
    static bool       healthy_;  ///< Whether application is currently healthy
    static std::mutex health_mtx_;  ///< Lock for the shared boolean
    static std::chrono::steady_clock::time_point
        unresponsive_until_;  ///< Do not respond until this time

    Poco::Net::HTTPServerResponse *resp_ =
        nullptr;  ///< The response code for the request

    /// \brief Get the supplied URL or create one from host and port
    ///
    /// This internal helper function creates URL to make requests to for one of
    /// the hosts in the host list.
    ///
    /// \param[in] node_name The node identifier to create a URL for
    /// \return string The URL to amke the call to
    std::string GetUrl(std::string &node_name);
  };

}  // namespace fidi

#endif /* FIDI_APP_DRIVER_H */

//
// fidi_app_driver.h ends here
