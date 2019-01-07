// fidi_lint_driver.h ---

#ifndef FIDI_LINT_DRIVER_H
#  define FIDI_LINT_DRIVER_H

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
/// This file implements the parser driver class for the fidi (φίδι)
/// linter, derived from the base driver class. It holds a reference
/// to the parser, and provides the parse helper and execute methods.

// Code:

#  include "src/fidi_driver.h"

namespace fidi {

  /// \brief fidi (φίδι) linter parser driver class
  ///
  /// This class is derived from the Driver class, that does most of
  /// the heavy lifting of parsing. This class adds to the
  /// parse_helper method. The base class just created a new scanner,
  /// here we create a new parser that uses the base class scanner,
  /// and actually perform the parsing.
  ///
  /// This class also provides an implementation of the execute
  /// method, which inspects the internal data structures populated by
  /// the parser and generates a graph description in dot(1)
  /// format. The execute method also runs sanity checks, and issues
  /// diagnostics for both the syntax checking done by the parser as
  /// well as the sanity check errors, if any.
  ///
  /// Since this is a linter, the execute method also recursively
  /// creates parser drivers for each of the payloads for the calls in
  /// the request, and appends the errors and warnings to the top
  /// level list.
  class LintDriver : public Driver {
   public:
    /// The default constructor
    LintDriver() : Driver(), parser_(nullptr){};

    /// \brief Constructor explicitly setting the data members
    ///
    /// This constructor explicitly sets the caller name, the name of
    /// this node, as well as the sequence of calls that have already
    /// been processed before the current request (which implies this
    /// is not the top level request). This is the constructor used to
    /// create the parser drivers for downstream requests.
    ///
    /// \param[in] c The name of the node that sent this request.
    /// \param[in] n Name of the current node, for the graph
    /// \param[in] s The sequence string of calls leading up to this node
    LintDriver(const std::string &c, const std::string &n,
               const std::string &s) :
        Driver() {
      caller_          = c;
      name_            = n;
      global_sequence_ = s;
    };

    /// The copy constructor is unused, and deleed
    LintDriver(const LintDriver &) = delete;

    /// The assignment operator is also not used.
    LintDriver &operator=(const LintDriver &) = delete;

    /// The move operations are unused, and cleaned up.
    ///
    /// The move operators are implicitly disabled, since there are no
    /// declarations and the compiler will not provide a version since we are
    /// defining copy constructors above. However, cleaning up explicitly also
    /// prevents derived classes from implementing the move operations. This is
    /// not a problem currently, since these are not needed, and removing them
    /// simplifies the generated code, and reduces its size.
    LintDriver(LintDriver &&) = delete;
    LintDriver &operator=(LintDriver &&) = delete;

    /// Destructor
    ///
    /// Cleans up the stored references to the scanner and the parser.
    virtual ~LintDriver();

    /// \brief The method where the guts of the linters work is done.
    ///
    /// This is the core of the linter. This method first runs sanity
    /// checks, and collects the warnings, if any. It then walks
    /// through the internal data structures (look to the base class
    /// Driver for details) and creates a dot graph. When processing
    /// edges, it creates a new parser driver for each payload, and
    /// calls the driver to recursively do what it has done. It
    /// collects the errors and warnings from the sub parsing drivers.
    ///
    /// \param[in,out] stream output stream where the dot graph is written to.
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

   private:
    fidi::Parser *parser_ = nullptr;  ///< A reference to the parser
                                      ///< created for handling this
                                      ///< request
  };

}  // namespace fidi

#endif /* FIDI_LINT_DRIVER_H */

//
// fidi_lint_driver.h ends here
