/* index.h ---  -*- mode: c++;  -*-
 *
 * Copyright 2018-2019 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Code:
 */

#ifndef INDEX_H
#define INDEX_H

/** \file
 *  \brief A non-code file to organize the Doxygen documentation.
 *
 * This file exists only to organize the documentation, including
 * defining the document's modules, and also adding in details of the
 * design.
 */

/** \mainpage The service mock application fidi (φίδι)
 * φίδι(fidi) n (plural φίδια) -- snake, serpent
 *
 * The common problem where mocking is used is when the user has a bit of
 * software under test, and wants to simulate or mock away everything that is
 * not the system under test (including the infrastructure pieces). The inverse
 * problem, where infrastructure is what is being tested (say, like your
 * monitoring, logging, or alerting system) and you want to mock away the
 * software that is running on the infrastructure? φίδι (fidi) is designed to
 * operate in that space.
 *
 * \section obj Objectives
 *
 * The goal for fidi (φίδι) is to model some aspects of arbitrarily
 * complex distributed client-server services, and simulate the
 * behaviour of the service’s business logic in presence of external
 * stimuli (without actually containing any real business logic or
 * complexity). The behaviour that is mocked by fidi (φίδι) is remote
 * calls, return codes, response size, and latency. This way fidi can
 * model the availability and performance profiles of the server,
 * without needing to recreate the business logic.  The behaviour of
 * each instance of fidi (φίδι) is determined by the request it
 * receives. Each instance of the fidi (φίδι) application can mock a
 * node (process) in the complex, client server service that it is
 * mocking. Different instances of fidi (φίδι) talk to other instances
 * of themselves, like a snake (φίδι) eating its tail.
 *
 * fidi (φίδι) also simulates fault injection symptoms, which allows
 * the simulation of normal behaviour as well as incidents and
 * recovery. The motivation is to test the behaviour of the
 * infrastructure, monitoring, alerting and logging systems, while
 * mocking an actual service.
 *
 * \section reqs Requirements and Design
 *
 * There are two non-code pages of interest:
 * - \subpage requirements
 * - \subpage design
 *
 * \section cdoc Code Documentation
 *
 * The code for fidi is divided into three components
 * - \ref inputhandling
 * - \ref lint
 * - \ref app
 */

/** \page requirements Requirements and Scale
 *
 * fidi (φίδι) must:
 *
 * + Not contain any business logic or sensitive data.
 * + Be able to model complex data flows that change over time.
 * + Be able to model arbitrarily complex interconnections between nodes in the
 * service.
 *     - Be able to simulate interconnects that happen in any combination, in
 * sequence and/or in parallel.
 * + Be able to massage the latency and error profile of any interconnection at
 * will.
 * + Enable all interconnections and fault injections to be specified on the
 * fly.
 * + Validate the input client request for correctness (independent validation,
 *   prior to run time).
 * + Have multiple instances able to coexist on a machine (to simulate complex
 *   apps on only a single machine, or a complex service that feels multiple
 *   boundaries with independent connections on a single instance).
 *
 * \section  userstories User Stories
 *
 * 1.  As an experiment runner, I want to be able to model different calling
 *     patterns and interactions between components of the service at will, or
 *      at least by each request to the system.
 *     1.  A corollary is that as an experimenter I want to be able to
 *         reconfigure the interconnections between components (request/data
 *         flow) at each request, simulating how a service may behave
 *          differently based on inputs/external stimuli.
 * 1.  As an experiment runner, I want to simulate the behaviour of the business
 *     logic of the service in response to stimuli and error conditions.
 *     1.  As an experiment runner, I want to be able to inject faults into the
 *         simulated service components. This includes, but is not limited to
 *         specifying the return code and injecting latency increases for any
 *         interaction in the request flow.
 *     1.  As an experiment runner, I should be able to specify the response
 *         code, latency, response size, and memory consumed by any component
           for each interaction in the flow.
 * 1.  As an evaluator, I want to model the business logic behaviour of a
 *     complex, multi-level service.
 *     1.  As an evaluator, I should be able to specify a series of interactions
 *         between the components of the system for each request, including
 *         serial or parallel interactions between components, and the
 *         performance characteristics of each of the interactions.
 * 1.  As an evaluator looking at gap analysis for the solutions deployed around
 *     the simulated service, I want to be able to script normal behaviour of
 *     the service, simulate an incident that affects one or all components of
 *     the service, and recovery, and set up any number of diverse incidents
 *     over time.
 *     1.  As an evaluator, I want to be able to simulate changing behaviour of
 *         the service over time, to reflect the real service changes of
 *         behaviour. This means that I should be able to specify, at an
 *         abstract level, the performance of each component of the simulated
 *         service (for example, performance, load on the machine, size of the
 *         responses, etc.)
 *
 * \section cache Exemplar Service
 *
 * fidi (φίδι) should be able to mock this caching service
 *
 * \startuml "An example caching service"
 *    autonumber
 *    actor Client
 *    boundary Frontend
 *    entity Cache
 *    database Backend
 *
 *    Client -> Frontend: Flow A
 *    Client -> Frontend: Flow B
 *    Client -> Frontend: Flow C
 *
 *    Frontend -> Cache: Flow A
 *    Frontend -> Cache: Flow B
 *    Cache -> Frontend: Flow A
 *    Frontend -> Cache: Flow C
 *
 *    Frontend -> Client: Flow A
 *    Cache -> Frontend: Flow B
 *    Frontend -> Client: Flow B
 *
 *    Cache -> Frontend: Flow C
 *    Frontend -> Backend: Flow C
 *    Backend -> Frontend: Flow C
 *    Frontend -> Client: Flow C
 * \enduml
 */

/** \page design The design document for fidi (φίδι)
 *
 * At the simplest level: this is a simple HTTP application that talks
 * to other instances of itself. The number and targets of these calls
 * are defined by the input request. Indeed, all aspects of the
 * applications behaviour are controlled by the input request, so the
 * behaviour modeled by the mocked service maybe changed by each
 * successive request.  The request can also specify the
 * return code, and optionally the delay before and after making the calls, and
 * the size of random text returned as a response.
 *
 * Each downstream call specification has an integer sequence number,
 * and a repetition count. Downstream calls section of the request
 * contains the full flow of calls further downstream, to whatever
 * depth necessary, nested within the payload. While parsing the
 * request, fidi (φίδι) need only parse only the parts of the request
 * that instance itself needs to make calls to the next level, the
 * details of the nested calls can be passed as a blob in the request
 * made to the next level downstream service mock application.
 *
 * Calls are made by fidi (φίδι) in sequence order, with a sequence
 * point in between successive sequence numbers (so strictly
 * serially). Calls with the same sequence number are made in
 * parallel, with a synchronization mechanism to complete all calls
 * before the calls with the next sequence number are made. If a call
 * has a repetition count, **repetition count** number of calls are
 * made in parallel.
 *
 * To recap, fidi (φίδι) needs to:
 * 1. parse the request.
 * 1. Handle the request response.
 *     1. Set the response code.
 *     2. Modify the response it sends back.
 *     3. Mock memory utilization.
 * 1. make one or more calls in a specified sequence, to other fidi (φίδι)
 *    instances, passing through the (nested) unpacked request content.
 *
 * Stated visually:
 *
 * \startuml "Sequence diagram for fidi"
 * actor Client #green
 * boundary Server #green
 * control Request
 * entity Parser
 * entity Call
 *
 * Client -> Server : Request
 * activate Server
 *
 * loop Over Requests
 *    Server -> Request: Add <<createReq>>
 *    activate Request #FFBBBB
 *
 *    create Parser
 *    Request -> Parser : Parse Request
 *    activate Parser
 *
 *    Parser -> Request : Done
 *    deactivate Parser
 *
 *    Request -> Request : Add Latency
 *    Request -> Request : Create Response
 *    activate Request #DarkSalmon
 *
 *    create Call
 *    loop Over Sequences
 *       loop Over Calls
 *          Request -> Call : Add Call
 *          activate Call #FFBBBB
 *
 *          Call -> Call : Make Call
 *          Call -> Request : done
 *          deactivate Call
 *       end
 *       Request -> Request : Synchronize
 *    end
 *    deactivate Request
 *
 *    Request -> Server : Request Finished
 *    deactivate Request
 *    destroy Parser
 *    destroy Call
 *
 *    destroy Request
 *
 *    Server -> Client
 * end
 * deactivate Server
 * \enduml
 *
 * And here is a flow chart
 *
 * \image html activity.png "Activity diagram for fidi"
 */

/** \defgroup fidi The service mock application fidi (φίδι)
 *
 * This is the top level for all the components of fidi (φίδι).
 */

/** \defgroup inputhandling The request parser for fidi (φίδι)
 *  \ingroup fidi
 *
 * The input language parser is the core of fidi (φίδι). It consists
 * of a simple scanner and a parser, and is used by both the lint and
 * HTTP application components. It implements a simple language parser
 * that is specific to fidi (φίδι). Writing a full grammar allows for
 * adjustments to the language to be easier, it isolates the input
 * request parsing from the rest of the application, and adding in
 * error recovery rules to the grammar allows the error diagnostics
 * for the parsing to be more thorough. The latter would be harder to
 * implement were a general purpose markup language used for the
 * requests. Most of the input handling code is shared between the
 * linter and the HTTP application (all except the execute method).
 *
 * Each instance of fidi_app mocks the behaviour of a single server in
 * the service being mocked by fidi (φίδι); The number and targets of
 * these calls are defined by the input request. Indeed, all aspects
 * of the application’s behaviour are controlled by the input request,
 * so the behaviour modeled by the mocked service maybe changed by each
 * successive request.
 *
 * \section nodes Hosts/Nodes
 *
 * The request must specify the attributes of every instance of the
 * fidi_app that is involved in the interaction defined in the
 * request, either a source node or a destination node for HTTP
 * requests. The host/node specification has a name, followed by
 * square brackets containing a list of comma separated attribute
 * key-value pairs
 * \code
 *        .client    [ hostname = "127.0.0.1", port = 8001, ]
 * \endcode
 * At a minimum, the node attributes must contain either a URL
 * key-value pair, or both host and port definitions, so that the HTTP
 * client request can be made to the host/node.
 *
 * \section edges Calls/Edges
 *
 * Each call is enclosed by square brackets, and differs from the node
 * definition in that the requests are not named.
 *
 * \subsection parameter Request Parameters
 *
 * The request contains request parameters, which are comma separated
 * key value pairs.  The following keys have significance:
 *
 * \subsubsection k1 predelay
 *
 * The value should be an unsigned integer that represents the number
 * of milliseconds the fidi_app shall sleep on handling the request
 * before taking any other action.
 *
 * \subsubsection k2 postdelay
 *
 * The value should be an unsigned integer that represents the number
 * of milliseconds the fidi_app shall sleep after all other actions
 * for the request have been performed, and just before the call
 * returns.
 *
 * \subsubsection k3 response
 *
 * The value should be a legal HTTP response code (sanity checked to
 * be a small integer less than 600).
 *
 * \subsubsection k4 size
 *
 * The value should be an unsigned integer that represents the number
 * of bytes in the returned response.
 *
 * \subsubsection k5 memory
 *
 * The value should be an unsigned integer that represents the number
 * of bytes of memory the fidi_app shall allocate during handling of
 * the request.
 *
 * \subsubsection k6 log_trace
 *
 * A string (preferably single line) to be (possibly) logged to the log file by
 * fidi (φίδι) as a trace log.  By default these messages are ignored.
 *
 * \subsubsection k7 log_debug
 *
 * A string (preferably single line) to be (possibly) logged to the log file by
 * fidi (φίδι) as a debug log.
 *
 * \subsubsection k8 log_information
 *
 * A string (preferably single line) to be (possibly) logged to the log file by
 * fidi (φίδι) as a information log.
 *
 * \subsubsection k9 log_notice
 *
 * A string (preferably single line) to be (possibly) logged to the log file by
 * fidi (φίδι).
 *
 * \subsubsection k10 log_warning
 *
 * A string (preferably single line) to be (possibly) logged to the log file by
 * fidi (φίδι).
 *
 * \subsubsection k11 log_error
 *
 * A string (preferably single line) to be (possibly) logged to the log file by
 * fidi (φίδι).
 *
 * \subsubsection k12 log_critical
 *
 * A string (preferably single line) to be (possibly) logged to the log file by
 * fidi (φίδι).
 *
 * \subsubsection k13 log_fatal
 *
 * A string (preferably single line) to be (possibly) logged to the log file by
 * fidi (φίδι).
 *
 * \subsubsection k14 timeout_sec
 *
 * This represents the number of whole seconds of time to set as a timeout on
 * downstream HTTP requests.
 *
 * \subsubsection k15 timeout_usec
 *
 * This is the rest of the timeout time (a fraction of a second), represented as
   the number of microseconds.  It is always less than one million.
 *
 * \subsubsection k16 healthy
 *
 * The value is a boolean instructing the application to be healthy, or not,
 * when responding to future /healthz requests. This onlyu applies to health
 * checks, and does not affect the respnse to requests.
 *
 * \subsubsection k17  unresponsive_for_sec
 *
 * The value is a the number in whole seconds that the application node should
 * be unresponsive for (as in drop all requests, including health checks). This
 * can be used, for instance, to simulate a restart.
 *
 * \subsubsection k18  unresponsive_for_usec
 *
 * The value is a the number of fractional microseconds (less than a million)
 * that the application node should be unresponsive for, in addioton to the
 * unresponsive_for_sec value above.
 *
 * \subsection calledge Calls
 *
 * Interspersed in these attributes can be specifications for calls
 * the fidi_app should make
 * \code
 *         -> frontend repeat = 2 sequence = 1 [...]
 * \endcode
 *
 * Each downstream call is defined by the arrow symbol "->", followed
 * by the destination name (the name should already have been defined
 * as detailed in the nodes section above), and optionally a repeat
 * count, and/or a sequence number.  The default repeat count is 1,
 * and the default sequence number is 1 as well. This is followed by
 * unparsed text in square brackets; those are instructions for the
 * destination host to process. In fact, the content within the square
 * brackets are identical to the call/edge format we have defined
 * here.
 *
 * Any number of calls can be defined. Calls with the same sequence
 * number shall be made in parallel; repeated calls are always made in
 * parallel.
 */

/** \defgroup lint The linter for the input request for  fidi (φίδι)
 *  \ingroup fidi
 *
 * The lint application serves as a convenience tool to validate the
 * initial input request sent to the set of service mock application
 * instances. It also serves an an integration test of the parsing component,
 * which is most of the complexity of the web application as well.
 */

/** \defgroup app The HTTP application that does the heavy lifting for fidi
 * (φίδι) \ingroup fidi
 *
 * This is the core component for fidi (φίδι). This module contains
 * a radically simple web server, which uses a new parser to parse
 * each new request. It then uses a priority queue and a thread-pool to
 * make downstream HTTP calls, in series or in parallel, as requested.
 */

/* index.h ends here */

#endif /* INDEX_H */
