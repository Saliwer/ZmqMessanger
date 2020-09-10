#include "DlgZMQ.h"

namespace ZmqMessanger
{
  ///////////////////////////////////////////////////////////////
  /////                      ZMQ                             ////
  ///////////////////////////////////////////////////////////////
  zmq::context_t* ZMQ::m_context  = nullptr;
  ZMQ*            ZMQ::m_instance = nullptr;

  ZMQ::ZMQ()
  {
    m_context = new zmq::context_t(1);
  }

  ZMQ::~ZMQ()
  {
    delete m_context;
  }

  ZMQ* ZMQ::Instance()
  {
    if (!m_instance)
      m_instance = new ZMQ();
    return m_instance;
  }

  socket_ptr_t ZMQ::CreateSocket(int socket_type)
  {
    return std::make_shared<zmq::socket_t>(*m_context, socket_type);
  }
  
}//end of namespace
