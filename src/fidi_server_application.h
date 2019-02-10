// fidi_server_application.h ---  -*- mode: c++; -*-

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
/// This file contains the fidi (φίδι) server application. This class
/// holds the port the server listens on, does command line argument
/// handling, and initializes the HTTP server application.

// Code:

#ifndef FIDI_SERVER_APPLICATION_H
#  define FIDI_SERVER_APPLICATION_H

#  include <Poco/AutoPtr.h>
#  include <Poco/ConsoleChannel.h>
#  include <Poco/FileChannel.h>
#  include <Poco/FormattingChannel.h>
#  include <Poco/Logger.h>
#  include <Poco/Message.h>
#  include <Poco/Net/HTTPRequestHandler.h>
#  include <Poco/Net/HTTPRequestHandlerFactory.h>
#  include <Poco/Net/HTTPServer.h>
#  include <Poco/Net/ServerSocket.h>
#  include <Poco/PatternFormatter.h>
#  include <Poco/Types.h>
#  include <Poco/Util/HelpFormatter.h>
#  include <Poco/Util/IntValidator.h>
#  include <Poco/Util/Option.h>
#  include <Poco/Util/OptionException.h>
#  include <Poco/Util/OptionSet.h>
#  include <Poco/Util/ServerApplication.h>
#  include <iostream>
#  include <string>
#  include <vector>

#  include "src/fidi_request_handler_factory.h"

namespace fidi {
  /// \brief The fidi (φίδι) HTTP server application
  ///
  /// This is the core of the HTTP server application.
  class FidiServerApplication : public Poco::Util::ServerApplication {
   public:
    /// \brief Default constructor
    ///
    /// Initializes the base class, and initializes internal state
    FidiServerApplication() :
        Poco::Util::ServerApplication(),
        help_requested_(false),
        port_(9001){};

    /// The copy constructor is not used, so decluttering.
    FidiServerApplication(const FidiServerApplication&) = delete;
    /// The assignment operator is also unused..
    FidiServerApplication& operator=(const FidiServerApplication&) = delete;

    /// The move operations are unused, and cleaned up.
    ///
    /// The move operators are implicitly disabled, since there are no
    /// declarations and the compiler will not provide a version since we are
    /// defining copy constructors above. However, cleaning up explicitly also
    /// prevents derived classes from implementing the move operations. This is
    /// not a problem currently, since these are not needed, and removing them
    /// simplifies the generated code, and reduces its size.
    FidiServerApplication(FidiServerApplication&&) = delete;
    FidiServerApplication& operator=(FidiServerApplication&&) = delete;

    /// Destructor. The data members clean themselves
    virtual ~FidiServerApplication(){};

   protected:
    /// \brief The main entry point for the server
    ///
    /// After initialization and command line parsing control passes
    /// to this method. If the usage message or the version had not
    /// been requested on the command line, this starts the web
    /// server, waits for an interrupt, and shuts the server down.
    ///
    /// \param[in] args A vector of command line argument pairs (ignored)
    int main(const std::vector<std::string>& args);

    /// \brief Initialize the server (mostly logging
    ///
    /// This method first calls the base class initialize method, then
    /// sets up two channels, one to the console, and another to a
    /// file. It sets up one logger for each channel. The logger
    /// associated with the log file only gets warnings and above,
    ///
    /// \param[in] self A reference to the application server
    void initialize(Poco::Util::Application& self);

    /// \brief Undo the effects of the initialization
    ///
    /// Calls the base class uninitialize method, and then shuts down
    /// the logger
    void uninitialize();

    /// \brief define command line options that this server handles
    ///
    /// In addition to the command line options handled by the base
    /// server application, this method adds --help, --version, and
    /// --port options.
    ///
    /// \param[in, out] options The option set to add options to
    void defineOptions(Poco::Util::OptionSet& options);

    /// \brief Respond to the command line option --help
    ///
    /// \param[in] name the name of the option (help, ignored)
    /// \param[in] value (ignored)
    void HandleHelp(const std::string& name, const std::string& value);

    /// \brief Respond to the command line option --version
    ///
    /// \param[in] name the name of the option (version, ignored)
    /// \param[in] value (ignored)
    void HandleVersion(const std::string& name, const std::string& value);

    /// \brief Set the port the server listens on based on the --port option
    ///
    /// \param[in] name the name of the option (port, ignored)
    /// \param[in] value A port number in string form
    void set_port(const std::string& name, const std::string& value);

    /// \brief Set the logging directory based on --log-dir option
    ///
    /// \param[in] name the name of the option (log-fir, ignored)
    /// \param[in] value The path to an xisting directory
    void SetLogDirectory(const std::string& name, const std::string& value);

    /// \brief Set the log file name based on --log-file option
    ///
    /// \param[in] name the name of the option (log-fir, ignored)
    /// \param[in] value File name for the log file (created if needed).
    void SetLogFile(const std::string& name, const std::string& value);

   private:
    /// Internal helper function to create a console logger
    void CreateConsoleLogger(void);

    /// Internal helper function to create a file logger
    void CreateFileLogger(void);

    bool help_requested_;          ///< Stores where --help was on the
                                   ///< command line
    Poco::UInt16 port_    = 9001;  ///< The port the server listens on
    std::string  log_dir_ = ".";   ///< The directory used for logging, default
                                   ///< current working directgory
    std::string log_file_ = "fidi_server.log";  ///< The log file name
  };
}  // namespace fidi

#endif /* FIDI_SERVER_APPLICATION_H */

//
// fidi_server_application.h ends here
