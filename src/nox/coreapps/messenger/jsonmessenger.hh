#ifndef JSONMESSENGER_HH__
#define JSONMESSENGER_HH__

/** Server port number for \ref vigil::jsonmessenger.
 */
#define JSONMESSENGER_PORT 2703
/** Server port number for SSL connection in \ref vigil::jsonmessenger.
 */
#define JSONMESSENGER_SSL_PORT 1303
/** Enable TCP connection.
 */
#define ENABLE_TCP_JSONMESSENGER true
/** Enable SSL connection.
 */
#define ENABLE_SSL_JSONMESSENGER false
/** Send message when new connections.
 */
#define JSONMSG_ON_NEW_CONNECTION true
/** Idle interval to trigger echo request (in s).
 */
#define JSONMESSENGER_ECHO_IDLE_THRESHOLD 0
/** Echo request non reply threshold to drop connection.
 */
#define JSONMESSENGER_ECHO_THRESHOLD 3

#include "json_object.hh"
#include "messenger_core.hh"
#include "msgpacket.hh"
#include <boost/shared_ptr.hpp>

namespace vigil
{
  using namespace vigil::container; 

  /** \ingroup noxevents
   * \brief Structure hold JSON message
   *
   * Copyright (C) Stanford University, 2010.
   * @author ykk
   * @date February 2010
   * @see jsonmessenger
   */
  struct JSONMsg_event : public Msg_event
  {
    /** Constructor.
     * @param msg Msg event use to initialize JSONMsg event
     */
    JSONMsg_event(const Msg_event* msg);

    /** Destructor.
     */
    ~JSONMsg_event()
    {}

    /** Static name required in NOX.
     */
    static const Event_name static_get_name() 
    {
      return "JSONMsg_event";
    }

    /** Reference to JSON object
     */
    boost::shared_ptr<json_object> jsonobj;
  };

  /** \ingroup noxcomponents
   * \brief Class through which to interact with NOX using JSON.
   *
   * TCP and SSL port can be changed at commandline using
   * tcpport and sslport arguments for golems respectively. 
   * port 0 is interpreted as disabling the server socket.  
   * E.g.,
   * ./nox_core -i ptcp:6633 jsonmessenger=tcpport=11222,sslport=0
   * will run TCP server on port 11222 and SSL server will be disabled.
   *
   * Received messages are sent to other components via the JSONMsg_event.
   * This allows extension of the messaging system, with no changes of
   * jsonmessenger.
   *
   * The expected basic message format is a dictionary as follows.
   * <PRE>
   * {
   *  "type": <string | connect, disconnect, ping, echo>
   * }
   * </PRE>
   * The rest of the dictionary can be application specific.
   *
   * Copyright (C) Stanford University, 2009.
   * @author ykk
   * @date Feburary 2010
   * @see messenger_core
   */
  class jsonmessenger : public message_processor
  {
  public:
    /** \brief Constructor.
     * Start server socket.
     * @param c context as required by Component
     * @param node Xercesc DOMNode
     */
    jsonmessenger(const Context* c, const xercesc::DOMNode* node);

    /** Destructor.
     * Close server socket.
     */
    virtual ~jsonmessenger() 
    { };
    
    /** Configure component
     * Register events..
     * @param config configuration
     */
    void configure(const Configuration* config);

    /** Start component.
     * Create jsonmessenger_server and starts server thread.
     */
    void install();

    /** Get instance of jsonmessenger (for python)
     * @param ctxt context
     * @param scpa reference to return instance with
     */
    static void getInstance(const container::Context* ctxt, 
			    vigil::jsonmessenger*& scpa);

    /** Function to do processing for messages received.
     * @param msg message event for message received
     */
    void process(const Msg_event* msg);

    /** Send echo request.
     * @param sock socket to send echo request over
     */
    void send_echo(Async_stream* sock);

    /** Function to do processing for block received.
     * @param buf pointer toblock received
     * @param dataSize size of block
     * @param data pointer to current message data (block not added)
     * @param currSize size of current message
     * @param sock reference to socket
     * @return length to copy
     */
    ssize_t processBlock(uint8_t* buf, ssize_t& dataSize,
			 uint8_t* data, ssize_t currSize, 
			 Msg_stream* sock);

    /** Function to determine message is completed.
     * @param data pointer to current message data (block not added)
     * @param currSize size of current message
     * @param sock reference to message stream
     * @return if message is completed (i.e., can be posted)
     */
    bool msg_complete(uint8_t* data, ssize_t currSize, Msg_stream* sock);

    /** Process string type, i.e., print in debug logs.
     * @param e event to be handled
     * @return CONTINUE always
     */
    Disposition handle_message(const Event& e);

    /** Reply to echo request.
     * @param echoreq echo request
     */
    void reply_echo(const JSONMsg_event& echoreq);

  private:
    /** \brief Object to count braces for JSON
     */
    struct counters
    {
      /** \brief Reset values
       */
      void reset();

      /** \brief Check if counter are balanced
       * @return if balanced
       */
      bool balanced();

      /** Number of outstanding left square brackets
       */
      ssize_t brackets;
      /** Number of outstanding left braces
       */
      ssize_t braces;
    };

    /** Reference to messenger_core.
     */
    messenger_core* msg_core;
    /** Memory allocated for \ref vigil::bookman messages.
     */
    boost::shared_array<uint8_t> raw_msg;
    /** Reference to msgpacket
     */
    msgpacket* msger;
    /** Counters for connections
     */
    hash_map<Msg_stream*, counters> sock_counters;
    /** TCP port number.
     */
    uint16_t tcpport;
    /** SSL port number.
     */
    uint16_t sslport;
  };
}

#endif
