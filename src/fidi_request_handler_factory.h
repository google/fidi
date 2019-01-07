// fidi_request_handler_factory.h ---  -*- mode: c++; -*-

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
/// This class provides a factory for creating a fidi (φίδι) request
/// handler.

// Code:
#ifndef FIDI_REQUEST_HANDLER_FACTORY_H
#  define FIDI_REQUEST_HANDLER_FACTORY_H

#  include <Poco/Net/HTTPRequestHandlerFactory.h>
#  include "src/fidi_request_handler.h"

// The UNUSED macro won't work for arguments which contain
// parenthesis, so if you have an argument like float (*coords)[3] you
// can't do, float UNUSED((*coords)[3]) or float (*UNUSED(coords))[3],
// This is the only downside to the UNUSED macro I found so far, in
// these cases I fall back to (void)coords;
#  ifdef __GNUC__
#    define UNUSED(x) UNUSED_##x __attribute__((__unused__))
#  else
#    define UNUSED(x) (void)UNUSED_##x
#  endif

#  ifdef __GNUC__
#    define UNUSED_FUNCTION(x) __attribute__((__unused__)) UNUSED_##x
#  else
#    define UNUSED_FUNCTION(x) UNUSED_##x
#  endif

namespace fidi {
  /// \brief required request handler factory for the application server
  ///
  /// This implements the interface required by the server
  /// application, and implements the createRequestHandler method.
  class FidiRequestHandlerFactory
      : public Poco::Net::HTTPRequestHandlerFactory {
   public:
    /// The default constructor
    FidiRequestHandlerFactory() = default;

    /// The copy constructor is not used, so decluttering.
    FidiRequestHandlerFactory(const FidiRequestHandlerFactory&) = delete;
    /// The assignment operator is also unused..
    FidiRequestHandlerFactory& operator=(const FidiRequestHandlerFactory&) =
        delete;

    /// The move operations are unused, and cleaned up.
    ///
    /// The move operators are implicitly disabled, since there are no
    /// declarations and the compiler will not provide a version since we are
    /// defining copy constructors above. However, cleaning up explicitly also
    /// prevents derived classes from implementing the move operations. This is
    /// not a problem currently, since these are not needed, and removing them
    /// simplifies the generated code, and reduces its size.
    FidiRequestHandlerFactory(FidiRequestHandlerFactory&&) = delete;
    FidiRequestHandlerFactory& operator=(FidiRequestHandlerFactory&&) = delete;

    /// Destructor -- we have no data members
    virtual ~FidiRequestHandlerFactory(){};

    /// \brief This is the one required method.
    /// \param[in] UNUSED We ignore the HTTP request object
    /// \return FidiRequestHandler We just return a new request handler
    virtual Poco::Net::HTTPRequestHandler*
    createRequestHandler(const Poco::Net::HTTPServerRequest& UNUSED(req)) {
      return new FidiRequestHandler;
    }
  };
}  // namespace fidi
#endif /* FIDI_REQUEST_HANDLER_FACTORY_H */
//
// fidi_request_handler_factory.h ends here
