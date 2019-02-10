// fidi_app_driver.cc ---  -*- mode: c++; -*-

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
/// This file provides the implementation of the methods for the
/// parser driver for the fidi (φίδι) HTTP server app. Specifically,
/// it provides the parse helper method and the execute method; the
/// former populates the provate data structures of the driver, and
/// the latter takes action based on the internal data so gathered.

// Code:
#include "src/fidi_app_driver.h"
#include <cassert>
#include <cctype>
#include <chrono>  // std::chrono:
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>  // std::this_thread::sleep_for

bool       fidi::AppDriver::healthy_ = true;
std::mutex fidi::AppDriver::health_mtx_;

fidi::AppDriver::~AppDriver() {
  delete (scanner_);
  scanner_ = nullptr;
  delete (parser_);
  parser_ = nullptr;
}

void
fidi::AppDriver::ParseHelper(std::istream &stream) {
  Poco::Logger::get("FileLogger").trace("Start parsing");

  delete (parser_);
  parse_errors_.clear();
  nerrors_ = 0;

  try {
    fidi::Driver::ParseHelper(stream);
    parser_ = new fidi::Parser((*scanner_) /* scanner */, (*this) /* driver */);
  } catch (std::bad_alloc &ba) {
    nerrors_++;
    parse_errors_.append("Failed to allocate parser: (")
        .append(ba.what())
        .append(")");
    Poco::Logger::get("FileLogger").error(parse_errors_);
    throw;
  }
  if (!parser_->parse() || nerrors_ != 0) {
    Poco::Logger::get("FileLogger").error(parse_errors_);
  }
  return;
}

std::string
fidi::AppDriver::GetUrl(std::string &node_name) {
  std::string url;
  auto        nodes_it = nodes_.find(node_name);

  // If the url was not specified, make it from hostname and port
  auto node_attr_it = nodes_it->second.find("url");
  if (node_attr_it != nodes_it->second.end()) {
    url = node_attr_it->second;
  } else {
    // The sanity check passed; so we know both hostname and port
    // exist
    url = "http://";
    url.append(nodes_it->second.find("hostname")->second)
        .append(":")
        .append(nodes_it->second.find("port")->second);

    // If no path is specfied, we use /fidi
    node_attr_it = nodes_it->second.find("path");
    if (node_attr_it != nodes_it->second.end()) {
      url.append(node_attr_it->second);
    } else {
      url.append("/fidi");
    }
  }
  return url;
}

bool
fidi::AppDriver::get_health(void) {
  bool is_healthy;
  health_mtx_.lock();
  is_healthy = healthy_;
  health_mtx_.unlock();
  return is_healthy;
}

std::ostream &
fidi::AppDriver::Execute(std::ostream &stream) {
  long timeout_sec  = 0;
  long timeout_usec = 0;

  // Create a threadpool (FIXME: make max threads a config)
  Poco::ThreadPool  tp(16,     // Min threads
                      1024);  // Max threads
  Poco::TaskManager tm(tp);
  Poco::Logger::get("ConsoleLogger").information("Handle request");

  // The first thing is to handle the specific things for this request
  if (top_attributes_.find("response") != top_attributes_.end()) {
    int code = std::stoi(top_attributes_["response"]);
    (*resp_).setStatus(static_cast<Poco::Net::HTTPResponse::HTTPStatus>(code));
  }

  if (top_attributes_.find("predelay") != top_attributes_.end()) {
    std::this_thread::sleep_for(
        std::chrono::milliseconds(std::stol(top_attributes_["predelay"])));
  }

  if (top_attributes_.find("timeout_sec") != top_attributes_.end()) {
    timeout_sec = std::stol(top_attributes_["timeout_sec"]);
  }

  if (top_attributes_.find("timeout_usec") != top_attributes_.end()) {
    timeout_usec = std::stol(top_attributes_["timeout_usec"]);
  }

  // OK. Now to deal with all out calls
  while (!edge_attributes_.empty()) {
    auto downstream_call_sequence_number =
        edge_attributes_.top().edge_attr.second;

    while (!edge_attributes_.empty() &&
           downstream_call_sequence_number ==
               edge_attributes_.top().edge_attr.second) {
      struct fidi::AppDriver::EdgeDetails call_details(edge_attributes_.top());
      // Sanity check passed, so we know the node details exist
      std::string url = GetUrl(call_details.name);

      // Handle multiple repetitions of the call
      auto reps = edge_attributes_.top().edge_attr.first;
      if (reps < 1) { reps = 1; }
      for (int i = 1; i <= reps; ++i) {
        std::string taskname(call_details.name);
        taskname.append("_").append(std::to_string(reps));
        tm.start(new AppCaller(
            taskname, url, timeout_sec, timeout_usec,
            node_glob_ + call_details.blob));  // Task Manager takes over
      }
      // Done with this call, on to the next one in this sequence
      edge_attributes_.pop();
    }
    // Done for this sequence point. Wait for all outstanding calls
    tm.joinAll();
  }

  // All the calls are done. First, let us log messages
  if (top_attributes_.find("log_trace") != top_attributes_.end()) {
    Poco::Logger::get("FileLogger").trace(top_attributes_["log_trace"]);
  }

  if (top_attributes_.find("log_debug") != top_attributes_.end()) {
    Poco::Logger::get("FileLogger").debug(top_attributes_["log_debug"]);
  }

  if (top_attributes_.find("log_information") != top_attributes_.end()) {
    Poco::Logger::get("FileLogger")
        .information(top_attributes_["log_information"]);
  }

  if (top_attributes_.find("log_notice") != top_attributes_.end()) {
    Poco::Logger::get("FileLogger").notice(top_attributes_["log_notice"]);
  }

  if (top_attributes_.find("log_warning") != top_attributes_.end()) {
    Poco::Logger::get("FileLogger").warning(top_attributes_["log_warning"]);
  }

  if (top_attributes_.find("log_error") != top_attributes_.end()) {
    Poco::Logger::get("FileLogger").error(top_attributes_["log_error"]);
  }

  if (top_attributes_.find("log_critical") != top_attributes_.end()) {
    Poco::Logger::get("FileLogger").critical(top_attributes_["log_critical"]);
  }

  if (top_attributes_.find("log_fatal") != top_attributes_.end()) {
    Poco::Logger::get("FileLogger").fatal(top_attributes_["log_fatal"]);
  }

  if (top_attributes_.find("healthy") != top_attributes_.end()) {
    health_mtx_.lock();
    if (top_attributes_["healthy"].compare("true") == 0) {
      healthy_ = true;
    } else {
      healthy_ = false;
    }
    health_mtx_.unlock();
  }

  // Now for the second part of the delay
  if (top_attributes_.find("postdelay") != top_attributes_.end()) {
    std::this_thread::sleep_for(
        std::chrono::milliseconds(std::stol(top_attributes_["postdelay"])));
  }
  return (stream);
}

void
fidi::AppDriver::set_resp(Poco::Net::HTTPServerResponse &response) {
  resp_ = &response;
}
//
// fidi_driver.cc ends here
