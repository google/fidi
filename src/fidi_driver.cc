// fidi_lint_driver.cc ---  -*- mode: c++; -*-

// Copyright 2018-2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations under
// the License.

/// \file
/// \ingroup inputhandling
///
/// This file defines the methods for the base class of the fidi
/// (φίδι) input request paresing subsystem. Most of the work for
/// handling an input request is done here, and the resultant parse
/// tree is cached into internal data structures.

// Code:
#include "src/fidi_driver.h"
#include <cassert>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>

fidi::Driver::~Driver() {
  delete scanner_;
  scanner_ = nullptr;
}

void
fidi::Driver::Parse(const char *const filename) {
  assert(filename != nullptr);
  std::ifstream in_file(filename);
  if (!in_file.good()) {
    std::cerr << "Could not read from input file: " << filename << "\n";
    exit(EXIT_FAILURE);
  }
  ParseHelper(in_file);
  return;
}

void
fidi::Driver::Parse(std::istream &stream) {
  if (!stream.good() && stream.eof()) { return; }

  ParseHelper(stream);
  return;
}

void
fidi::Driver::HandleTop(const std::map<std::string, std::string> &top_list) {
  top_attributes_ = top_list;
}

void
fidi::Driver::HandleNode(const std::string &                       node_name,
                         const std::map<std::string, std::string> &node_list) {
  auto it = nodes_.find(node_name);
  if (it == nodes_.end()) {
    nodes_[node_name] = node_list;
  } else {
    it->second.insert(node_list.begin(), node_list.end());
  }
  // The grammar requires hostnames to be in double quotes, we remove those
  // here.
  auto node_attributes = nodes_[node_name];
  auto attributes_it   = node_attributes.find("hostname");
  if (attributes_it != node_attributes.end()) {
    std::size_t found = 0;
    while ((found = attributes_it->second.find('"', found)) !=
           std::string::npos) {
      attributes_it->second.erase(found, 1);
    }
    nodes_[node_name]["hostname"] = attributes_it->second;
  }
  node_glob_.append(node_name).append(" [\n");
  for (auto const &[key, value] : node_list) {
    node_glob_.append("  ").append(key).append(" = ").append(value).append(
        ",\n");
  }
  node_glob_.append("]\n");
}

void
fidi::Driver::HandleEdge(const std::string &       edge_name,
                         const std::pair<int, int> edge_list,
                         const std::string &       new_blob) {
  struct EdgeDetails new_edge = {edge_name, new_blob, edge_list};
  destinations_.emplace(edge_name);
  new_edge.blob.insert(0, "\n    [");
  edge_attributes_.push(new_edge);
}

void
fidi::Driver::ParseHelper(std::istream &stream) {
  delete scanner_;
  try {
    scanner_ = new fidi::FidiFlexLexer(&stream);
  } catch (std::bad_alloc &ba) {
    std::cerr << "Failed to allocate scanner: (" << ba.what() << ")\n";
    throw;
  }
  return;
}

int
fidi::Driver::SanityChecks(std::string *error_message) {
  int errors = 0;
  // Check to see is a value containing a number has trailing garbage
  auto check_num = [&](const std::string str_value,
                       const std::string err_top) -> int {
    size_t idx;
    try {
      int  number  = std::stoi(str_value, &idx);
      auto remains = str_value.substr(idx);
      if (!remains.empty()) {
        errors++;
        error_message->append(err_top)
            .append(" contains trailing garbage\n")
            .append("//  ")
            .append(std::to_string(number))
            .append("  ")
            .append(remains)
            .append("\n");
      }
      return number;
    } catch (const std::invalid_argument &e) {
      errors++;
      error_message->append(err_top)
          .append(" is not a valid integer\n")
          .append("//  ")
          .append(e.what())
          .append("\n");
    } catch (const std::out_of_range &e) {
      errors++;
      error_message->append(err_top)
          .append(" is out of range\n")
          .append("//  ")
          .append(e.what())
          .append("\n");
    } catch (const std::bad_alloc &e) {
      errors++;
      error_message->append(err_top)
          .append(" failed to allocate memory\n")
          .append("//  ")
          .append(e.what())
          .append("\n");
    } catch (...) {
      errors++;
      error_message->append(err_top).append(
          " encountered an unexpected exception\n");
    }
    return 0;
  };

  for (auto const &[id, node_attributes] : nodes_) {
    if (node_attributes.find("url") == node_attributes.end()) {
      if ((node_attributes.find("hostname") == node_attributes.end()) ||
          (node_attributes.find("port") == node_attributes.end())) {
        errors++;
        error_message->append("// Node Definition for ")
            .append(id)
            .append(" must contain either\n")
            .append("// a url or both hostname and port attributes.\n");
      }
    }
    auto it = node_attributes.find("port");
    if (it != node_attributes.end()) {
      (void)check_num(it->second, "// Port definition ");
    }

    auto hostname_it = node_attributes.find("hostname");
    if (hostname_it != node_attributes.end()) {
      std::size_t found = 0;
      found             = hostname_it->second.find('"', found);
      if (found != std::string::npos) {
        errors++;
        error_message->append("// hostname should not contain double quotes\n")
            .append("// ")
            .append(hostname_it->second)
            .append("\n");
      }
    }
  }

  for (auto const &key : destinations_) {
    if (nodes_.find(key) == nodes_.end()) {
      errors++;
      error_message->append("// Destination node ")
          .append(key)
          .append(" not defined\n");
    }
  }

  auto it = top_attributes_.find("response");
  if (it != top_attributes_.end()) {
    auto response =
        check_num(it->second, "// Request response code specification ");
    if (response <= 0 || response >= 600) {
      errors++;
      error_message->append("// Request response code specification ")
          .append(std::to_string(response))
          .append("\n// does not seem like a HTTP response code\n");
    }
  } else {
    errors++;
    error_message->append("//  Request response code specification missing\n");
  }

  it = top_attributes_.find("predelay");
  if (it != top_attributes_.end()) {
    (void)check_num(it->second, "// Request pre-delay ");
  }

  it = top_attributes_.find("postdelay");
  if (it != top_attributes_.end()) {
    (void)check_num(it->second, "// Request post-delay ");
  }

  it = top_attributes_.find("timeout_sec");
  if (it != top_attributes_.end()) {
    (void)check_num(it->second, "// Request timeout whole seconds ");
  }

  it = top_attributes_.find("timeout_usec");
  if (it != top_attributes_.end()) {
    (void)check_num(it->second, "// Request timeout fractional microseconds ");
    long usec = std::stol(it->second);
    if (usec >= 1000000L) {
      errors++;
      error_message
          ->append(
              "Request timeout fractional microseconds should be less than 1 "
              "Million: ")
          .append(it->second);
    }
  }

  return errors;
}

//
// fidi_driver.cc ends here
