#ifndef __DLG_SERVER_H__
#define __DLG_SERVER_H__

#include <map>
#include <zmq.hpp>
#include <thread>
#include <list>
#include <memory>
#include <atomic>
#include <algorithm>

#include "Config.h"
#include "DlgService.h"
#include "DlgMessage.h"
#include "Exception.h"
#include "DlgZMQ.h"

namespace ZmqMessanger
{
  class DlgServer
  {
    using MsgHandler = bool (DlgServer::*)(message_ptr_t msg);
    using MsgHandlers = std::map<MessageType, MsgHandler>;
    using service_ptr_t = std::shared_ptr<DlgService>;
    using thread_ptr_t = std::shared_ptr<std::thread>;

    std::string               m_address;
    socket_ptr_t              m_socket;
    thread_ptr_t              m_thread;
    MsgHandlers               m_handlers;
    std::list<service_ptr_t>  m_services;
    std::atomic<bool>         m_stop;
    
  public:
    explicit DlgServer(const char* address);
    explicit DlgServer(const std::string& address);
    ~DlgServer();
    DlgServer() = delete;

    void Start();
    void Stop();

  private:
    void main_thread();

    bool join_service_callback(message_ptr_t msg);
    bool create_service_callback(message_ptr_t msg);
    bool delete_service_callback(message_ptr_t msg);
  };

}//end of namespace

#endif
