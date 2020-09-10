#ifndef __DLG_SERVICE_H__
#define __DLG_SERVICE_H__

#include <string>
#include <list>
#include <queue>
#include <map>
#include <algorithm>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <memory>
#include <zmq.hpp>
#include <sys/time.h>

#include "DlgMessage.h"
#include "Exception.h"
#include "DlgZMQ.h"
#include "Config.h"

namespace ZmqMessanger
{
  int64_t current_time();
  
  struct aUser
  {
    std::string m_id;
    int64_t    m_expiry;

    aUser(const std::string& name, int64_t expiry) :
    m_id(name), m_expiry(expiry)
    {
    }
    
  };
  
  class DlgBroker
  {
    using user_ptr_t = std::shared_ptr<aUser>;
    using thread_ptr_t = std::shared_ptr<std::thread>;
    using MsgHandler = bool (DlgBroker::*)(message_ptr_t msg);
    using MsgHandlers = std::map<MessageType, MsgHandler>;
    
    std::string                m_address;
    socket_ptr_t               m_socket;
    thread_ptr_t               m_recv_thread;
    thread_ptr_t               m_send_thread;
    std::mutex                 m_mutex;
    std::atomic<bool>          m_stop;
    std::condition_variable    m_condition;
    std::list<user_ptr_t>      m_users;
    std::queue<message_ptr_t>  m_msgs;
    MsgHandlers                m_handlers;
    
  public:
    DlgBroker(const std::string &server_address);
    ~DlgBroker();
    
    const std::string& GetAddress() const { return m_address; }
    bool HasUsers() const { return !m_users.empty(); }
    bool AddUser(const std::string& name);
    
  private:
    void recieve_thread();
    void send_thread();
    
    bool join_service_callback(message_ptr_t);
    bool broadcast_message_callback(message_ptr_t);
    bool private_message_callback(message_ptr_t);
  };
  
  class DlgService
  {
    using broker_ptr_t = std::shared_ptr<DlgBroker>;
    
    std::string  m_name;
    broker_ptr_t m_broker;

  public:
    DlgService(const std::string& name, const std::string& server_address);
    ~DlgService();
    bool HasUsers();
    bool AddUser(message_ptr_t msg);
    const std::string& GetName() const { return m_name; }
    
    
  };
  
}//end of namespace

#endif
