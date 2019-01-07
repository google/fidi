// fidi_driver.h ---  -*- mode: c++; -*-

#ifndef FIDI_DRIVER_H
#  define FIDI_DRIVER_H

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
/// To support a pure interface with the parser (and the scanner) the
/// technique of the “parsing context” is convenient: a structure
/// containing all the data to exchange.  Since, in addition to simply
/// launch the parsing, there are several auxiliary tasks to execute
/// we use a fully blown “parsing driver” class. This is the common
/// abstract base class that manages input parsing, the derived
/// classes provide the actions taken after a succesfull parse.

// Code:

#  include <cstddef>
#  include <functional>
#  include <istream>
#  include <queue>
#  include <set>
#  include <string>
#  include <vector>

#  include "src/fidi_flex_lexer.h"
#  include "src/fidi_parser.hh"

namespace fidi {

  /// \brief A abstract base class to drive input parsing
  ///
  /// This is the common abstract base class that manages input
  /// parsing, the derived classes provide the actions taken after a
  /// succesfull parse. This class contains all the data structures
  /// that hold the results of parsing the requet, so executing the
  /// actions based on the request can be delegated to the derived
  /// classes.
  ///
  /// Some data members exist for passing information to sub
  /// parsers. This is usually only done by the lint checker, since
  /// that has to fully parse the request, not just the top level.
  ///
  /// This also contains an instance of the scanner.
  class Driver {
   public:
    /// The default construvtor
    ///
    /// This takes no parameters, and just sets the data members to
    /// the default (empty) state.
    Driver() :
        parse_errors_(),
        nerrors_(0),
        caller_("Source"),
        name_("TopNode"),
        global_sequence_("1"),
        node_glob_(),
        top_attributes_(),
        nodes_(),
        edge_attributes_(),
        destinations_(),
        num_warnings_(0),
        warnings_() {}

    /// The copy constructor is not used, so decluttering
    Driver(const Driver &) = delete;

    /// The assignment operator is also unused.
    Driver &operator=(const Driver &) = delete;

    /// The move operations are unused, and cleaned up.
    ///
    /// The move operators are implicitly disabled, since there are no
    /// declarations and the compiler will not provide a version since we are
    /// defining copy constructors above. However, cleaning up explicitly also
    /// prevents derived classes from implementing the move operations. This is
    /// not a problem currently, since these are not needed, and removing them
    /// simplifies the generated code, and reduces its size.
    Driver(Driver &&) = delete;
    Driver &operator=(Driver &&) = delete;

    /// Destructor. All members have desdtructors.
    virtual ~Driver();

    /** \brief parse - parse from a file
     *
     * Check that the file exists, and then open an insput stream for
     * the file and pass it in to the parse_helper method.
     *
     * \param[in] filename - valid string with input file
     */
    void Parse(const char *const filename);

    /** \brief parse - parse from a c++ input stream
     *
     * Pass through the input stream to the parse_helper method.
     *
     * \param[in] iss - std::istream&, valid input stream
     */
    void Parse(std::istream &iss);

    /// \brief Handle attributes of the request itself
    ///
    /// This method takes a map of the request attributes and stashes a
    /// private copy away.
    ///
    /// \param[in] top_list A reference to the key-value attributes of the
    /// request
    void HandleTop(const std::map<std::string, std::string> &top_list);

    /// \brief Handle the node details, given a name and attribute list
    ///
    /// This method takes a node name, and attribute list and adds
    /// them to the private associative map of nodes. Since the
    /// grammar requires hostnames to be quoted (the grammar does not
    /// like periods), but the HTTP client library does not like
    /// quotes, this method strips off the single or double quotes
    /// around hostnames before adding them to the local stash.
    ///
    /// This method also appends the node definiton to the local blob
    /// variable; that is used to append the node details to each
    /// outgoing request, prepended to the outgoing request payload.
    ///
    /// \param[in] name The node name
    /// \param[in] node_list A reference to the key-value attributes of the node
    void HandleNode(const std::string &                       name,
                    const std::map<std::string, std::string> &node_list);

    /// \brief Handle the outgoing call details
    ///
    /// The call payload is almost identical to the top level request,
    /// but it is missing leading open bracket. This method prepends
    /// the opening open bracket to the payload before adding it to
    /// the local data structure.
    ///
    /// \param[in] name The name of the destination node
    /// \param[in] edge_list the repeat count and the sequence number
    /// \param[in] blob The call payload
    void HandleEdge(const std::string &       name,
                    const std::pair<int, int> edge_list,
                    const std::string &       blob);

    /// A virtual method instanciated by derived calsses to act on the parsed
    /// data
    virtual std::ostream &Execute(std::ostream &stream) = 0;
    // TODO: Consider implementing a request ID to be passed in here

    /// \brief Create a new scanner with the provided input stream
    ///
    /// This method deletes the old scanner, if any, and creates a new
    /// scanner passing in the new input stream. Please note that
    /// Derived classes override this method.
    ///
    /// \param[in,out] stream the input data to be parsed is in this stream
    virtual void ParseHelper(std::istream &stream);

    /// \brief Run a number of sanity checks on the parsed request
    ///
    /// Run a number of sanity checks on the request, for example:
    /// + Ensure that each host definition has either a URL or both host and
    ///   port attributes
    /// + Ensure that the port number is an unsigned small integer
    /// + Ensure that the host name does not have quotes (which slipped
    ///   sanitization
    /// + Ensure that call destination hosts are hosts for which definitions
    ///   exist
    /// + ensure that the request response code is specified
    /// + Validate that the request response code is numerical
    /// + Ansire that the request response code looks like a HTTP response
    /// + ensure that the predelay amount is an integer
    /// + ensure that the postdelay amount is an integer
    ///
    /// \param[out] error_message A string to append error messages to.
    /// \return int The number of errors encountered.
    int SanityChecks(std::string *error_message);

    /// \brief return the list of parse errors encountered
    ///
    /// The syntax errors discovered during parsing are stored
    /// locally. This method gives access to the current list of
    /// errors.
    ///
    /// \return pair<int, string> The number of errors  and associated messages
    std::pair<int, std::string>
    get_errors() {
      return std::make_pair(nerrors_, parse_errors_);
    }

    /// \brief return the list of sanity check warnings
    ///
    /// The errors discovered during sanity checking are stored
    /// locally. This method gives access to the current list of
    /// warnings.
    ///
    /// \return pair<int, string> The number of warnings and associated messages
    std::pair<int, std::string>
    get_warnings() {
      return make_pair(num_warnings_, warnings_);
    }

    std::string parse_errors_;  ///< A string containing all the
                                ///< parse error encountered
    int nerrors_;               ///< The number of parse errors seen

   protected:
    /// The call/edge details. Used as nodes in the priority queue
    struct EdgeDetails {
      std::string         name;       ///< Name of the destination node
      std::string         blob;       ///< Payload for the call
      std::pair<int, int> edge_attr;  ///< Repeat count and sequence number
    };

    /// \brief a class that compares struct EdgeDetails
    ///
    /// This class has a single method that can help order a set of
    /// EdgeDetails structure instances based on the sequence number.
    class EdgeComparison {
     public:
      /// Default constructor
      EdgeComparison() = default;

      /// \brief compare two struct EdgeDetails
      ///
      /// This method compares two struct EdgeDetails based on their
      /// sequence numbers, and weakly orders them in reverse
      /// sequence number order. The lower numbered sequences come
      /// first in the list.
      ///
      /// \param[in] a The first struct to compare
      /// \param[in] b The second struct to compare
      /// \return bool True if a is sorted later than b
      bool
      operator()(const struct EdgeDetails &a,
                 const struct EdgeDetails &b) const {
        return a.edge_attr.second > b.edge_attr.second;
      }
    };

    // The next three are different for sub parsing the
    // payloads. These are useful inly to the linter, since it needs
    // to do a full parse.
    std::string caller_          = "Source";   ///< The current caller
    std::string name_            = "TopNode";  ///< The name of the current node
    std::string global_sequence_ = "1";        ///< Period separated
                                               ///< Sequence numbers of
                                               ///< the upstream
                                               ///< requests leading up
                                               ///< to this one. Changes
                                               ///< for sub parsers

    std::string node_glob_;  ///< Stash all node definitions in
                             ///< a blob, to be prepended to the
                             ///< payload for making downstream
                             ///< calls.
    fidi::FidiFlexLexer *scanner_ =
        nullptr;  ///< Keep a pointer to the scanner.

    /// \brief The attributes pertaining to the top level request
    std::map<std::string, std::string> top_attributes_;

    /// The set of nodes and attributes
    std::map<std::string, std::map<std::string, std::string>> nodes_;

    /// \brief The set of calls/edges
    ///
    /// The calls are sorted into a priority queue, so they can be
    /// executeds in priority order.
    std::priority_queue<struct EdgeDetails, std::vector<struct EdgeDetails>,
                        EdgeComparison>
                          edge_attributes_;
    std::set<std::string> destinations_;  ///< The set of known destinations

    int         num_warnings_;  ///< The number of sanity check warnings found
    std::string warnings_;  ///< The warning messages associated with the sanity
                            ///< checking.
  };

} /* end namespace fidi */

#endif /* FIDI_DRIVER_H */

//
// fidi_driver.h ends here
