// fidi_request_handler.cc ---  -*- mode: c++; -*-

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
/// This file provides the implementation of the request handling
/// functionality for the fidi (φίδι) HTTP server. This creates
/// multiple instances of the fidi::AppCaller class to actually make
/// downstream calls. This sets up a priority queue, a threadpool, and
/// a task manager to handle calls in parallel and in sequence.

// Code:

#include "src/fidi_request_handler.h"

void
fidi::FidiRequestHandler::handleRequest(Poco::Net::HTTPServerRequest & req,
                                        Poco::Net::HTTPServerResponse &resp) {
  std::ostream &response_stream = resp.send();
  Poco::Logger::get("ConsoleLogger")
      .information("Request from " + req.clientAddress().toString());
  bool failed = false;
  resp.setChunkedTransferEncoding(true);
  resp.setContentType("text/html");
  Poco::URI uri(req.getURI());

  if (uri.getPath().compare("/healthz") == 0) {
    Poco::Logger::get("FileLogger").trace("Healthz");
    // TODO: Check for and set a not OK status if we are not healthy
    resp.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
    return;
  }
  response_stream << "<html><head><title>Fidi  (φίδι) -- a service mock "
                     "instance\n</title></head>\n"
                     "<body>\n"
                     "<h1>Hello world!</h1>\n"
                     "<p>Count: "
                  << ++count_
                  << "</p>\n"
                     "<p>Method: "
                  << req.getMethod()
                  << "</p>\n"
                     "<p>URI: "
                  << req.getURI() << "</p>\n";
  try {
    driver_.Parse(req.stream());
  } catch (std::bad_alloc &ba) {
    std::cerr << "Got memory error: " << ba.what() << "\n";
    std::cerr.flush();
    return;
  }  // Fail fast on OOM

  auto parse_errors = driver_.get_errors();
  if (parse_errors.first != 0) {
    // take action to return errors
    // Set response, generate error message
    resp.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    response_stream << "    <h2>Parse Syntax Errors</h2>\n\n\n"
                    << parse_errors.second;
    failed = true;
  }
  std::string warning_message;
  int         warning = driver_.SanityChecks(&warning_message);
  if (warning) {
    resp.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    response_stream << "    <h2>Parse  Errors</h2>\n\n\n" << warning_message;
    failed = true;
  }
  if (!failed) {
    driver_.set_resp(resp);
    try {
      driver_.Execute(response_stream);
    } catch (std::bad_alloc &ba) {
      std::cerr << "Got memory error: " << ba.what() << "\n";
      std::cerr.flush();
      return;
    }  // Fail fast on OOM
  }
  response_stream << "</body></html>";
  response_stream.flush();

  Poco::Logger::get("FileLogger")
      .information("Response sent for count=" + std::to_string(count_) +
                   " and URI=" + req.getURI() + "\n");
}

//
// fidi_request_handler.cc ends here
