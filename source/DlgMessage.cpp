#include "DlgMessage.h"

namespace ZmqMessanger
{
  ///////////////////////////////////////////////////////////////
  //////               message_type_t                      //////
  /////////////////////////////////////////////////////////////// 

  message_type_t::message_type_t() : m_type(EMPTY_MESSAGE)
  {
  }
  
  message_type_t::message_type_t(MessageType type) : m_type(type)
  {
  }

  message_type_t::~message_type_t()
  {
  }

  const MessageType* message_type_t::data() const
  {
    return &m_type;
  }

  size_t message_type_t::size() const
  {
    return sizeof(m_type);
  }

  bool message_type_t::operator==(MessageType type) const
  {
    return m_type == type;
  }

  bool message_type_t::operator!=(MessageType type) const
  {
    return m_type != type;
  }

  
  ///////////////////////////////////////////////////////////////
  //////                 DlgMessage                        //////
  ///////////////////////////////////////////////////////////////
  DlgMessage::DlgMessage(const std::string& to,
			 const std::string& from,
			 MessageType type,
			 const std::vector<uint8_t>& body) :
    m_to(to), m_from(from), m_type(type), m_body(body)
  {
  }
  
  DlgMessage::DlgMessage(const std::string& to,
			 const std::string& from,
			 MessageType type,
		        const char* str) :
    m_to(to), m_from(from), m_type(type)
  {
    size_t size = strlen(str);
    m_body.resize(size);
    memcpy(m_body.data(), str, size);
    m_body.push_back('\0');
  }

  DlgMessage::DlgMessage(const std::string& to,
			 const std::string& from,
			 MessageType type,
			 const std::string& str) :
    DlgMessage(to, from, type, str.c_str())
  {
  }
    

  DlgMessage::DlgMessage()
  {
  }

  DlgMessage::~DlgMessage()
  {
  }
  
  void DlgMessage::SetMessageType(MessageType type)
  {
    m_type.SetType(type);
  }
  
  void DlgMessage::SetFromAddress(const std::string& from)
  {
    m_from = from;
  }
  
  void DlgMessage::SetToAddress(const std::string& to)
  {
    m_to = to;
  }
  
  void DlgMessage::SetMessageBody(const std::vector<uint8_t>& body)
  {
    m_body = body;
  }
  
  void DlgMessage::SetMessageBody(void* body, size_t size)
  {
    m_body.resize(size);
    memcpy(m_body.data(), body, size);
  }

  void DlgMessage::SetMessageBody(const char* str)
  {
    size_t size = strlen(str);
    m_body.resize(size);
    memcpy(m_body.data(), str, size);
    m_body.push_back('\0');
  }

  void DlgMessage::SetMessageBody(const std::string& str)
  {
    m_body.resize(str.size() + 1);
    memcpy(m_body.data(), str.data(), str.size());
    m_body.push_back('\0');
  }
  
  const MessageType& DlgMessage::GetMessageType() const
  {
    return *m_type.data();
  }
  
  const std::string& DlgMessage::GetFromAddress() const
  {
    return m_from;
  }
  
  const std::string& DlgMessage::GetToAddress() const
  {
    return m_to;
  }
  
  const std::vector<uint8_t>& DlgMessage::GetMessageBinaryBody() const
  {
    return m_body;
  }

  const char* DlgMessage::GetMessageStrBody() const
  {
    
    return reinterpret_cast<const char*>(m_body.data());
  }
  
  void DlgMessage::Clear()
  {
    m_to.clear();
    m_from.clear();
    m_body.clear();
    m_type.clear();
  }
  
  bool DlgMessage::Recv(socket_ptr_t socket)
  {
    if (!socket)
      return false;
    Clear();

    zmq::message_t message;
    MessagePart msg_part = FROM_ADDRESS;
    int sockID;
    size_t size_sock = sizeof(sockID);
    socket->getsockopt(ZMQ_TYPE, &sockID, &size_sock);
    bool flag = (sockID == ZMQ_ROUTER) ? true : false;
    
    do{
      if (!socket->recv(&message)){
      	Print(DBG_LEVEL_ERROR, "DlgMessage::Recv(): "
	      "Couldn't recieve message.\n");
	return false;
      }
      
      if (flag){
	flag = false;
	continue;
      }
      
      switch(msg_part){

      case FROM_ADDRESS:
	m_from.resize(message.size());
        m_from.assign((char*)message.data());
	break;
	
      case TO_ADDRESS:
	m_to.resize(message.size());
        m_to.assign((char*)message.data());
	break;

      case MESSAGE_TYPE:
	m_type.SetType(*(MessageType*)message.data());
	break;

      case MESSAGE_BODY:
	m_body.resize(message.size());
	memcpy(m_body.data(), message.data(), message.size());
	break;

      default:
	Print(DBG_LEVEL_ERROR, "DlgMessage::Resv(): "
	      "Trying to recieve uknown part of message.\n");
	return false;
      }

      msg_part = static_cast<MessagePart>(msg_part + 1);
    }while(message.more());
    print_message();
    return true;
  }

  bool DlgMessage::Send(socket_ptr_t socket)
  {
    if (!socket)
      return false;
    
    MessagePart msg_part = static_cast<MessagePart>(0);
    while(msg_part != MAX_MESSAGE_PART){   
      if ((int)msg_part >= (int)MAX_MESSAGE_PART){
	Print(DBG_LEVEL_ERROR, "DlgMessage::Send(): "
	      "Message is too long.\n");
	return false;
      }      
      zmq::message_t* message;
      
      switch(msg_part){

      case FROM_ADDRESS:
	if (m_from.empty()){
	  Print(DBG_LEVEL_ERROR, "DlgMessage::Send(): "
		"Address of sender is empty.\n");
	  return false;
	}
        message = new zmq::message_t(m_from.data(), m_from.size() + 1);
	break;
	
      case TO_ADDRESS:
	if (m_to.empty()){
	  Print(DBG_LEVEL_ERROR, "DlgMessage::Send(): "
		"Address of destination is empty.\n");
	  return false;
	}
        message = new zmq::message_t(m_to.data(), m_to.size() + 1);
	break;      

      case MESSAGE_TYPE:
	message = new zmq::message_t(m_type.data(), m_type.size());
	break;
	
      case MESSAGE_BODY:
	message = new zmq::message_t(m_body.data(), m_body.size());
	break;
	
      default:
	Print(DBG_LEVEL_ERROR, "DlgMessage::Send(): "
	      "Trying to send uknown part of message.\n");
	return false;	
      }
	   
      socket->send(*message,
		   ((int)msg_part == (int)(MAX_MESSAGE_PART - 1)) ?
		   0 : ZMQ_SNDMORE);
      delete message;
      msg_part = static_cast<MessagePart>(msg_part + 1);
    }
    return true;
  }


  void DlgMessage::print_message()
  {
    Print(DBG_LEVEL_DEBUG, "*******MESSAGE INFORMATION********\n");
    if (!m_to.empty())
    Print(DBG_LEVEL_DEBUG,
	  "Destination address: '%s'\n", GetToAddress().c_str());

    if (!m_from.empty())
      Print(DBG_LEVEL_DEBUG,
	    "Source address: '%s'\n", GetFromAddress().c_str());

    Print(DBG_LEVEL_DEBUG,
	  "Message type: '%s'\n",
	  get_message_type(GetMessageType()).c_str());
 
    Print(DBG_LEVEL_DEBUG,
	  "Message body with size %d\n",
	  GetMessageBinaryBody().size());
    Print(DBG_LEVEL_DEBUG, "Message str body: '%s'\n",
	  GetMessageStrBody());
    Print(DBG_LEVEL_DEBUG, "***********************************\n"); 
  }

  std::string DlgMessage::get_message_type(MessageType type)
  {
    switch(type){
    case EMPTY_MESSAGE: return "EMPTY_MESSAGE";
    case JOIN_SERVICE: return "JOIN_SERVICE";
    case CREATE_SERVICE: return "CREATE_SERVICE";
    case DELETE_SERVICE: return "DELETE_SERVICE";
    case BROADCAST_MESSAGE: return "BROADCAST_MESSAGE";
    case PRIVATE_MESSAGE: return "PRIVATE_MESSAGE";
    case HEARTBEAT_PING: return "HEARTBEAT_PING";
    case HEARTBEAT_PONG: return "HEARTBEAT_PONG";
    default: return "UNKNOWN_TYPE";
    }
  }
  
}// end of namespace ZmqMeesanger

