// fidi_request_handler.h ---  -*- mode: c++; -*-

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
/// This file contains the request handling class for the fidi (φίδι)
/// HTTP server, and apart from the parser, does most of the heavy
/// lifting.

// Code:

#ifndef FIDI_REQUEST_HANDLER_H
#  define FIDI_REQUEST_HANDLER_H
#  include <Poco/Net/HTTPRequestHandler.h>
#  include <Poco/Net/HTTPResponse.h>
#  include <Poco/Net/HTTPServerRequest.h>
#  include <Poco/Net/HTTPServerResponse.h>
#  include <Poco/Util/ServerApplication.h>
#  include <iostream>
#  include "src/fidi_app_driver.h"

namespace fidi {
  /// \brief This class handles HTTP requests made to  fidi (φίδι)
  class FidiRequestHandler : public Poco::Net::HTTPRequestHandler {
   public:
    /// The default constructor. Calls the base class constructor
    FidiRequestHandler() : driver_(){};

    /// The copy constructor is not used, so decluttering.
    FidiRequestHandler(const FidiRequestHandler &) = delete;
    /// The assignment operator is also unused..
    FidiRequestHandler &operator=(const FidiRequestHandler &) = delete;

    /// The move operations are unused, and cleaned up.
    ///
    /// The move operators are implicitly disabled, since there are no
    /// declarations and the compiler will not provide a version since we are
    /// defining copy constructors above. However, cleaning up explicitly also
    /// prevents derived classes from implementing the move operations. This is
    /// not a problem currently, since these are not needed, and removing them
    /// simplifies the generated code, and reduces its size.
    FidiRequestHandler(FidiRequestHandler &&) = delete;
    FidiRequestHandler &operator=(FidiRequestHandler &&) = delete;

    /// Destructor -- nothing needs to be done
    virtual ~FidiRequestHandler(){};

    /// \brief this is the workhorse for request handling
    ///
    /// First, this method calls the contained parser to parse the
    /// input request. If there are parse errors, it returns the error
    /// messages and set the response code to HTTP_BAD_REQUEST. Next,
    /// this runs sanity checks. and treats warnings in the sanity
    /// check the same as it did for parse errors (return
    /// HTTP_BAD_REQUEST).
    ///
    /// After than, this sets up a priority queue, a thread pool, and a
    /// task manager to handle calls in parallel and in sequence. This
    /// creates multiple instances of the fidi::AppCaller class to
    /// actually make downstream calls.
    ///
    /// \param[in] req The HTTP request
    /// \param[in, out] resp The HTTP response
    virtual void handleRequest(Poco::Net::HTTPServerRequest & req,
                               Poco::Net::HTTPServerResponse &resp);

   private:
    int       count_ = 0;  ///< The number of requests handled
    AppDriver driver_;     ///< The HTTP server parser driver
  };
}  // namespace fidi
#endif /* FIDI_REQUEST_HANDLER_H */

//
// fidi_request_handler.h ends here
