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
  
  const MessageType DlgMessage::GetMessageType() const
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
  
  const std::vector<uint8_t>& DlgMessage::GetMessageBody() const
  {
    return m_body;
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
    MessagePart msg_part = static_cast<MessagePart>(0);
    
    do{
      if (!socket->recv(&message)){
      	Print(DBG_LEVEL_ERROR, "DlgMessage::Recv(): "
	      "Couldn't recieve message.\n");
	return false;
      }
      
      switch(msg_part){
      TO_ADDRESS:
	m_to.reserve(message.size());
        m_to.copy((char*)message.data(), message.size());
	break;

      FROM_ADDRESS:
	m_from.reserve(message.size());
        m_from.copy((char*)message.data(), message.size());
	break;

      MESSAGE_TYPE:
	m_type.SetType(*(MessageType*)message.data());
	break;

      MESSAGE_BODY:
	m_body.reserve(message.size());
	memcpy(m_body.data(), message.data(), message.size());
	break;

      default:
	Print(DBG_LEVEL_ERROR, "DlgMessage::Resv(): "
	      "Trying to recieve uknown part of message.\n");
	return false;
      }

      msg_part = static_cast<MessagePart>(msg_part + 1);
    }while(message.more());
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
      TO_ADDRESS:
	if (m_to.empty()){
	  Print(DBG_LEVEL_ERROR, "DlgMessage::Send(): "
	      "Address of destination is empty.\n");
	  return false;
	}	  
        message = new zmq::message_t(m_to.data(), m_to.size());
	break;

      FROM_ADDRESS:
	if (m_from.empty()){
	  Print(DBG_LEVEL_ERROR, "DlgMessage::Send(): "
	      "Address of sender is empty.\n");
	  return false;
	}	  
        message = new zmq::message_t(m_from.data(), m_from.size());
	break;
	
      MESSAGE_TYPE:
	message = new zmq::message_t(m_type.data(), m_type.size());
	break;

      MESSAGE_BODY:
	message = new zmq::message_t(m_body.data(), m_body.size());
	break;

      default:
	Print(DBG_LEVEL_ERROR, "DlgMessage::Send(): "
	      "Trying to send uknown part of message.\n");
	return false;	
      }
	   
      socket->send(message,
		   ((int)msg_part == (int)(MAX_MESSAGE_PART - 1)) ?
		   0 : ZMQ_SNDMORE);
      delete message;
      msg_part = static_cast<MessagePart>(msg_part + 1);
    }
    return true;
  }


  
}// end of namespace ZmqMeesanger

