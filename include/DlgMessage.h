#ifndef __DLG_MESSAGE_H__
#define __DLG_MESSAGE_H__

#include <zmq.hpp>
#include <string>
#include <string.h>
#include <vector>
#include <cstdint>
#include <memory>


#include "Config.h"
#include "Debug.h"
#include "DlgZMQ.h"

namespace ZmqMessanger {

  enum MessageType
  {
    EMPTY_MESSAGE = 0,
    JOIN_SERVICE,
    CREATE_SERVICE,
    DELETE_SERVICE,
    BROADCAST_MESSAGE,
    PRIVATE_MESSAGE,
    HEARTBEAT_PING,
    HEARTBEAT_PONG
  };
  
  class message_type_t
  {
    MessageType m_type;
  public:
    message_type_t();
    message_type_t(MessageType type);
    ~message_type_t();
    const MessageType* data() const;
    size_t size() const;
    void clear() { m_type = EMPTY_MESSAGE; }
    void SetType(MessageType type) { m_type = type; }
    bool operator==(MessageType type) const;
    bool operator!=(MessageType type) const;
  };

  class DlgMessage
  {   
    enum MessagePart
    {
      FROM_ADDRESS = 0,
      TO_ADDRESS = 1,
      MESSAGE_TYPE,
      MESSAGE_BODY,
      MAX_MESSAGE_PART
    };

    using byte_array_t = std::vector<uint8_t>;
    std::string    m_to;
    std::string    m_from;
    message_type_t m_type;
    byte_array_t   m_body;
  public:
    DlgMessage(const std::string& to, const std::string& from,
	       MessageType type, const std::vector<uint8_t>& body);

    DlgMessage(const std::string& to, const std::string& from,
	       MessageType type, const char* str);

    DlgMessage(const std::string& to, const std::string& from,
	       MessageType type, const std::string& str);
    
    DlgMessage();

    ~DlgMessage();

    void SetMessageType(MessageType type);
    void SetFromAddress(const std::string& from);
    void SetToAddress(const std::string& to);
    void SetMessageBody(const std::vector<uint8_t>& body);
    void SetMessageBody(void* body, size_t size);
    void SetMessageBody(const char* str);
    void SetMessageBody(const std::string& str);
      
    const MessageType& GetMessageType() const;
    const std::string& GetFromAddress() const;
    const std::string& GetToAddress() const;
    const std::vector<uint8_t>& GetMessageBinaryBody() const;
    const char* GetMessageStrBody() const;
    void Clear();
	
    bool Recv(socket_ptr_t socket);
    bool Send(socket_ptr_t socket);
  private:
    void print_message();
    std::string get_message_type(MessageType type);
  };

  
  using message_ptr_t = std::shared_ptr<DlgMessage>;
}//end of namespace ZmqDialog


#endif
