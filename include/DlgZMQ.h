#ifndef __DLG_ZMQ_H__
#define __DLG_ZMQ_H__

#include <zmq.hpp>
#include <memory>


namespace ZmqMessanger{
  
  using socket_ptr_t = std::shared_ptr<zmq::socket_t>;
  class ZMQ
  {
    static zmq::context_t* m_context;
    static ZMQ*            m_instance;
    ZMQ();
  public:
    static ZMQ* Instance();
    ~ZMQ();
    zmq::context_t* Context() { return m_context; }
    socket_ptr_t CreateSocket(int socket_type);
  };
  
}

#endif
