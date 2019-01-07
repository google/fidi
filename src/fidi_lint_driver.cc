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
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/// \file
/// \ingroup lint
///
/// This file provides the implementation of the methods for the
/// parser driver for the fidi (φίδι) linter. Specifically, It
/// provides the parse helper method and the execute method; the
/// former populates the Private data structures of the driver, and
/// the latter takes action based on the internal data so gathered.

// Code:
#include "src/fidi_lint_driver.h"

#include <cassert>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

fidi::LintDriver::~LintDriver() {
  delete (scanner_);
  scanner_ = nullptr;
  delete (parser_);
  parser_ = nullptr;
}

void
fidi::LintDriver::ParseHelper(std::istream &stream) {
  delete (parser_);
  try {
    fidi::Driver::ParseHelper(stream);
    parser_ = new fidi::Parser((*scanner_) /* scanner */, (*this) /* driver */);
  } catch (std::bad_alloc &ba) {
    std::cerr << "Failed to allocate parser: (" << ba.what()
              << "), exiting!!\n";
    exit(EXIT_FAILURE);
  }
  const int accept(0);
  parser_->set_debug_level(0);
  if (parser_->parse() != accept || nerrors_ != 0) {
    std::cerr << "Parse failed!! with " << nerrors_ << " errors.\n"
              << parse_errors_ << std::endl;
  }
  return;
}

/// \brief Handle payloads of the calls at the top level
///
/// This static function creates a new lint driver, and creates a new
/// input string stream from the payload. and parses the blob. It
/// emits diagnostics for the syntax errors from the sub parser.
///
/// \param[in,out] stream Output stream to write dot graph segments to.
/// \param[in] caller The name of the caller for this call/edge
/// \param[in] name The name of the destination node
/// \param[in] blob The payload for the call we will parse
/// \param[in] sequence_number The edge label so far
/// \param[in,out] sub_warnings Where sanity check errors are reported
static std::ostream &
HandleBlob(std::ostream &stream, const std::string &caller,
           const std::string &name, const std::string &blob,
           const std::string &          sequence_number,
           std::pair<int, std::string> &sub_warnings) {
  fidi::LintDriver   sub_driver(caller, name, sequence_number);
  std::istringstream iss(blob);
  sub_driver.Parse(iss);
  if (sub_driver.nerrors_ != 0) {
    std::cerr << "Parse failed!! with " << sub_driver.nerrors_ << " errors.\n"
              << sub_driver.parse_errors_ << std::endl;
  };
  // run sanity check, and generate more of the graph
  sub_driver.Execute(stream);
  sub_warnings = sub_driver.get_warnings();

  return stream;
}

std::ostream &
fidi::LintDriver::Execute(std::ostream &stream) {
  // Run sanity checks
  num_warnings_ = SanityChecks(&warnings_);

  // At the top level request we need to emit the graph preamble. If
  // we are an inferior prasing process, which means we are parsing
  // the payload for a higher level request, we skip this part, so as
  // to not duplicate the preamble and the node details.
  if (caller_.compare("Source") == 0) {
    stream << "digraph fidi {\n  node [shape=record];\n";
    for (auto const &[node, attributes] : nodes_) {
      stream << "  " << node << " [ label=\"{";
      for (auto const &[label, value] : attributes) {
        std::string val(value);
        // This is probably not needed, but defensive programming
        auto found = val.find('"');
        while (found != std::string::npos) {
          val.replace(found, 1, "'");
          found = val.find('"', found);
        }
        stream << label << "=" << val << "|";
      }
      stream << node << "}\" ];\n";
    }
  }

  // We now add the edge for this request, and add the top level edge
  // attributes.
  stream << "\n  " << caller_ << " -> " << name_;
  stream << " [ label=\"" << global_sequence_ << "\"]\n";
  for (auto const &[key, value] : top_attributes_) {
    stream << "     // " << key << " = " << value << ",\n";
  }
  stream << std::endl;

  // We now walk through the calls we have to make
  while (!edge_attributes_.empty()) {
    auto current_sequence = edge_attributes_.top().edge_attr.second;

    while (!edge_attributes_.empty() &&
           current_sequence == edge_attributes_.top().edge_attr.second) {
      // Handle the edge, and call handle blob to process the payload
      std::pair<int, std::string> sub_warnings;
      auto                        node = edge_attributes_.top();
      std::string                 new_sequence(global_sequence_);
      new_sequence.append(".").append(std::to_string(current_sequence));
      HandleBlob(stream, name_, node.name, node_glob_ + node.blob, new_sequence,
                 sub_warnings);
      if (sub_warnings.first) {
        num_warnings_ += sub_warnings.first;
        warnings_.append(sub_warnings.second);
      }
      edge_attributes_.pop();
    }
  }
  if (caller_.compare("Source") == 0) { stream << "\n}\n"; }
  if (num_warnings_) {
    std::cerr << "Found " << num_warnings_
              << " non-syntax errors in the input.\n"
              << warnings_;
    stream << warnings_;
  }
  return (stream);
}

//
// fidi_driver.cc ends here
