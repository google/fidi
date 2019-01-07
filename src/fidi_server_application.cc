// fidi_server_application.cc ---  -*- mode: c++; -*-

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
/// This file has the methods for the fidi (φίδι) HTTP server
/// application class.

// Code:

#include "src/fidi_server_application.h"

int
fidi::FidiServerApplication::main(const std::vector<std::string>&) {
  Poco::Net::HTTPServer server(new FidiRequestHandlerFactory,
                               Poco::Net::ServerSocket(port_),
                               new Poco::Net::HTTPServerParams);
  if (!help_requested_) {
    server.start();
    Poco::Logger::get("ConsoleLogger").information("Fidi Server Started");
    Poco::Logger::get("FileLogger").information("Fidi Server Started");
    // Wait for a control C
    waitForTerminationRequest();
    Poco::Logger::get("ConsoleLogger")
        .information("Fidi Server Shutting Down...");
    Poco::Logger::get("FileLogger").information("Fidi Server Shutting Down...");
    server.stop();
  }
  return Poco::Util::Application::EXIT_OK;
}

void
fidi::FidiServerApplication::initialize(Poco::Util::Application& self) {
  loadConfiguration();
  Poco::Util::ServerApplication::initialize(self);
  // set up two channel chains - one to the
  // console and the other one to a log file.
  Poco::AutoPtr<Poco::PatternFormatter> pattern_formatter_p(
      new Poco::PatternFormatter("[%O] %s: %p: %t"));
  Poco::AutoPtr<Poco::FormattingChannel> console_formatting_channel_p(
      new Poco::FormattingChannel(pattern_formatter_p));
  Poco::AutoPtr<Poco::ConsoleChannel> console_channel_p(
      new Poco::ConsoleChannel());
  console_formatting_channel_p->setChannel(console_channel_p);
  console_formatting_channel_p->open();

  Poco::AutoPtr<Poco::PatternFormatter> pattern_formatter2_p(
      new Poco::PatternFormatter("%Y-%m-%d %H:%M:%S.%c %U:%u%N[%P]:%s:%q:%t"));
  Poco::AutoPtr<Poco::FormattingChannel> file_formatting_channel_p(
      new Poco::FormattingChannel(pattern_formatter2_p));
  Poco::AutoPtr<Poco::FileChannel> file_channel_p(
      new Poco::FileChannel("fidi_server.log"));
  file_formatting_channel_p->setChannel(file_channel_p);
  file_formatting_channel_p->open();

  // create two Logger objects - one for
  // each channel chain.
  Poco::Logger& console_logger =
      Poco::Logger::create("ConsoleLogger", console_formatting_channel_p,
                           Poco::Message::PRIO_INFORMATION);
  Poco::Logger& file_logger = Poco::Logger::create(
      "FileLogger", file_formatting_channel_p, Poco::Message::PRIO_WARNING);

  console_logger.information("Fidi Server initialized.");
  file_logger.information("Fidi Server initialized.");  // this goes nowhere
}

void
fidi::FidiServerApplication::uninitialize() {
  Poco::Util::ServerApplication::uninitialize();
  Poco::Logger::shutdown();
}

void
fidi::FidiServerApplication::defineOptions(Poco::Util::OptionSet& options) {
  Poco::Util::ServerApplication::defineOptions(options);
  options.addOption(
      Poco::Util::Option("help", "h", "display argument help information")
          .required(false)
          .repeatable(false)
          .callback(Poco::Util::OptionCallback<fidi::FidiServerApplication>(
              this, &fidi::FidiServerApplication::HandleHelp)));

  options.addOption(
      Poco::Util::Option("version", "v", "display version number")
          .required(false)
          .repeatable(false)
          .callback(Poco::Util::OptionCallback<fidi::FidiServerApplication>(
              this, &fidi::FidiServerApplication::HandleVersion)));

  options.addOption(
      Poco::Util::Option("port", "p", "local port to listen on")
          .required(false)
          .repeatable(false)
          .argument("<port_number>")
          .binding("server.port")
          .validator(new Poco::Util::IntValidator(
              0, std::numeric_limits<unsigned short int>::max()))
          .callback(Poco::Util::OptionCallback<fidi::FidiServerApplication>(
              this, &fidi::FidiServerApplication::set_port)));
}

void
fidi::FidiServerApplication::HandleVersion(const std::string&,
                                           const std::string&) {
  std::cout << PACKAGE_NAME << " version " << PACKAGE_VERSION << "\n";
  stopOptionsProcessing();
  help_requested_ = true;
}

void
fidi::FidiServerApplication::HandleHelp(const std::string&,
                                        const std::string&) {
  Poco::Util::HelpFormatter help_formatter(options());

  help_formatter.setCommand(commandName());
  help_formatter.setUsage("OPTIONS");
  help_formatter.setHeader(
      "fidi service mocker (this instance mocks a single node in the service "
      "being mocked).");
  help_formatter.format(std::cout);
  stopOptionsProcessing();
  help_requested_ = true;
}

void
fidi::FidiServerApplication::set_port(const std::string&,
                                      const std::string& value) {
  // The validator above should ensure this is indeed an int
  port_ = static_cast<Poco::UInt16>(std::stoi(value));
}

//
// fidi_server_application.cc ends here
