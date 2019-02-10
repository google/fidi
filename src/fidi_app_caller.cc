// fidi_app_caller.cc ---

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
/// This file contains the code to make downstream calls for the fidi
/// (φίδι) HTTP server.

// Code:

#include "src/fidi_app_caller.h"

void
fidi::AppCaller::runTask() {
  Poco::Logger::get("FileLogger")
      .information("Making call to " + url_ + "\n\t" + payload_);
  Poco::Logger::get("ConsoleLogger")
      .information("Making call to " + url_ + "\n\t" + payload_);
  try {
    Poco::URI                    uri(url_);
    Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
    if (timeout_sec_ > 0 || timeout_usec_ > 0) {
      session.setTimeout(Poco::Timespan(timeout_sec_, timeout_usec_));
    }

    // prepare path
    std::string path(uri.getPathAndQuery());
    if (path.empty()) path = "/";

    Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_POST, path,
                               Poco::Net::HTTPMessage::HTTP_1_1);
    req.setContentType("application/x-www-form-urlencoded");
    req.setChunkedTransferEncoding(true);

    req.setContentLength(payload_.length());

#if defined(DEBUG)
    req.write(std::cout);  // print out request for debugging
#endif

    std::ostream& os = session.sendRequest(req);
    os << payload_;
    Poco::Net::HTTPResponse res;
    std::string             rbody;
    session.receiveResponse(res) >> rbody;

    Poco::Logger::get("FileLogger").information(res.getReason());
    Poco::Logger::get("ConsoleLogger").information(res.getReason());
  } catch (Poco::Exception& ex) {
    Poco::Logger::get("FileLogger").error(ex.displayText());
    Poco::Logger::get("ConsoleLogger").error(ex.displayText());
  }
}
//
// fidi_app_caller.cc ends here
