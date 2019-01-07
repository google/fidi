// fidi_app_caller.h ---  -*- mode: c++; -*-

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
/// This file contains the class that handles makking downstream calls
/// for the fidi (φίδι) HTTP server, given either a URL, or a
/// host/port pair, and a payload.

// Code:

#ifndef FIDI_APP_CALLER_H
#  define FIDI_APP_CALLER_H

#  include <Poco/Exception.h>
#  include <Poco/Logger.h>
#  include <Poco/Net/HTTPClientSession.h>
#  include <Poco/Net/HTTPRequest.h>
#  include <Poco/Net/HTTPResponse.h>
#  include <Poco/Task.h>
#  include <Poco/TaskManager.h>
#  include <Poco/ThreadPool.h>
#  include <Poco/URI.h>
#  include <iostream>

namespace fidi {
  /// \brief A task manager task that makes HTTP client requests
  ///
  /// This class can be passed to the POCO taskmanager, and it exists
  /// to make a single HTTP request. It catches and logs any errors in
  /// making the call.
  class AppCaller : public Poco::Task {
   public:
    /// \brief Constructor
    ///
    /// Since the runtask method takes no arguments, all call specific
    /// details must be passed in through the constructor.
    ///
    /// \param[in] name The name for the task
    /// \param[in] dest The URL for the target server
    /// \param[in] content The body of the post request
    AppCaller(std::string &name, const std::string &dest,
              const std::string &content) :
        Poco::Task(name),
        url_(dest),
        payload_(content){};

    /// \brief Destructor
    ///
    /// All out members clean themselves
    virtual ~AppCaller(){};

    /// This class copy constructor is not used, so declutter.
    AppCaller(const AppCaller &) = delete;
    /// The assignment operation is also not used, so cleaned up.
    AppCaller &operator=(const AppCaller &) = delete;

    /// The move operations are unused, and cleaned up.
    ///
    /// The move operators are implicitly disabled, since there are no
    /// declarations and the compiler will not provide a version since we are
    /// defining copy constructors above. However, cleaning up explicitly also
    /// prevents derived classes from implementing the move operations. This is
    /// not a problem currently, since these are not needed, and removing them
    /// simplifies the generated code, and reduces its size.
    AppCaller(AppCaller &&) = delete;
    AppCaller &operator=(AppCaller &&) = delete;

    /// \brief Handle making a single downstream requests
    ///
    /// We make a single request per session for simplicity. So, currently,
    /// making a downstream HTTP call means
    /// + Create a new HTTP session
    /// + Create a new request
    /// + Make the call
    /// + Log the information
    virtual void runTask();

   private:
    const std::string url_;      ///< The URL we are makeing the request to
    const std::string payload_;  ///< The payload for the request
  };

}  // namespace fidi

#endif /* FIDI_APP_CALLER_H */

//
// fidi_app_caller.h ends here
