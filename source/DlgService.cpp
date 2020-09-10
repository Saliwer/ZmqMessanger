#include "DlgService.h"

namespace ZmqMessanger
{
  int64_t current_time()
  {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t) (tv.tv_sec * 1000000 + tv.tv_usec);
  }
  
  ///////////////////////////////////////////////////////////////////
  /////                      DlgBroker                          /////
  ///////////////////////////////////////////////////////////////////

  DlgBroker::DlgBroker(const std::string& server_address)
  {
    try{
      m_socket = ZMQ::Instance()->CreateSocket(ZMQ_ROUTER);
      std::string endpoint = TRANSPORT_PROTOCOL +
	server_address.substr(0, server_address.find(':', 0) + 1) + "0";
      m_socket->bind(endpoint.data());
      char address[256];
      size_t size = sizeof(address);
      m_socket->getsockopt(ZMQ_LAST_ENDPOINT, &address, &size);
      //the return value of address has the follow format:
      //address = "tcp://address:port"
      //To skip "tcp://" part -> (address + 6)
      m_address = std::move(std::string(address + 6));
    }
    catch(zmq::error_t& e){
      Print(DBG_LEVEL_ERROR, "DlgBroker(): "
	    "zmq::exception %s\n", e.what());
      throw Exception("DlgBroker(): fatal error.");
    }
    catch(std::exception& e){
        Print(DBG_LEVEL_ERROR, "DlgBroker(): "
	      "std::exception %s\n", e.what());
        throw Exception("DlgBroker():  fatal error.");
    }
    catch(...){
        Print(DBG_LEVEL_ERROR, "DlgBroker(): unknown exeption.\n");
        throw Exception("DlgBroker(): fatal error.");
    }

    Print(DBG_LEVEL_DEBUG, "DlgBroker(): "
	  "Broker is bound to '%s' address.\n", m_address.c_str());
    m_handlers[JOIN_SERVICE] =
      &DlgBroker::join_service_callback;
    m_handlers[BROADCAST_MESSAGE] =
      &DlgBroker::broadcast_message_callback;
    m_handlers[PRIVATE_MESSAGE] =
      &DlgBroker::private_message_callback;
    m_stop = false;
    m_recv_thread =
      std::make_shared<std::thread>(&DlgBroker::recieve_thread, this);
    m_send_thread =
      std::make_shared<std::thread>(&DlgBroker::send_thread, this);
  }

  DlgBroker::~DlgBroker()
  {
    m_stop = true;
    m_condition.notify_all();
    if (m_recv_thread && m_recv_thread->joinable())
      m_recv_thread->join();
    if (m_send_thread && m_send_thread->joinable())
      m_send_thread->join();
  }

  bool DlgBroker::AddUser(const std::string& name)
  {
    auto it = std::find_if(m_users.begin(), m_users.end(),
			   [&name](user_ptr_t user)
			   {
			     return user->m_id == name;
			   });

    if (it != m_users.end()){
      Print(DBG_LEVEL_DEBUG, "DlgBroker::AddUser(): "
	    "User with name '%s' already exists.\n", name.c_str());
      return false;
    }

    user_ptr_t newUser =
      std::make_shared<aUser>(name, current_time() +
			      HEARTBEAT_INTERVAL * HEARTBEAT_LIVENESS);
    std::lock_guard<std::mutex> lck(m_mutex);
    m_users.push_back(newUser);
    return true;      
  }

  void DlgBroker::recieve_thread()
  {
    Print(DBG_LEVEL_DEBUG, "DlgBroker::main_thread(): "
	  "Start of recieve thread '%s'.\n", m_address.c_str());
    while(!m_stop)
      {
	zmq::pollitem_t items[] =
	  {
	    { static_cast<void*>(*m_socket), 0, ZMQ_POLLIN, 0 }
	  };
	zmq::poll(items, 1, (long)HEARTBEAT_INTERVAL/1000);

	if (items[0].revents & ZMQ_POLLIN){
	  message_ptr_t msg = std::make_shared<DlgMessage>();
	  if (!msg->Recv(m_socket)){
	    Print(DBG_LEVEL_ERROR, "DlgBroker::recieve_thread(): "
		  "Coudn't recieve message.\n");
	    continue;
	  }
	  std::lock_guard<std::mutex> lck(m_mutex);
	  m_msgs.push(msg);
	  m_condition.notify_one();
	}	
      }//end of loop
    Print(DBG_LEVEL_DEBUG, "DlgBroker::recieve_thread(): "
	  "End of recieve thread '%s'.\n", m_address.c_str());
  }//end of recieve_thread func


  void DlgBroker::send_thread()
  {
    Print(DBG_LEVEL_DEBUG, "DlgBroker::send_thread(): "
	  "Start of send thread '%s'.\n", m_address.c_str());
    while (!m_stop){
      std::unique_lock<std::mutex> lck(m_mutex);
      m_condition.wait(lck, [this](){
	  return !m_msgs.empty() || m_stop;
	});
      if (m_stop)
	break;

      message_ptr_t msg = m_msgs.front();
      m_msgs.pop();
      if (m_handlers.find(msg->GetMessageType()) ==
	  m_handlers.end()){
	Print(DBG_LEVEL_ERROR, "DlgBroker::send_thread(): "
	      "Unknown message type.\n");
	continue;
      }
      else{
	//Calling method pointer
	//Of course it would be better to use std::invoke
	//std::invoke(m_handlers[msgType], *this, params...);
	if (!(this->*m_handlers[msg->GetMessageType()])(msg)){
	  Print(DBG_LEVEL_ERROR, "DlgBroker::send_thread(): "
		"Something went wrong during msg parsing.\n");
	  continue;
	} 
      }  
    }//end of loop
    Print(DBG_LEVEL_DEBUG, "DlgBroker::send_thread(): "
	  "End of send thread '%s'.\n", m_address.c_str());
  }//end of send_thread()

  
  bool DlgBroker::join_service_callback(message_ptr_t msg)
  {
    return AddUser(msg->GetFromAddress());
  }

  bool DlgBroker::broadcast_message_callback(message_ptr_t msg)
  {
    for(auto it = m_users.begin(); it != m_users.end(); ++it){
      msg->SetToAddress((*it)->m_id);
      if (!msg->Send(m_socket))
	Print(DBG_LEVEL_ERROR, "DlgBroker::broadcast_message(): "
	      "Couldn't send message to '%s'", (*it)->m_id.c_str());
    }
    return true;
  }

  bool DlgBroker::private_message_callback(message_ptr_t msg)
  {
    std::string toAddress = msg->GetToAddress();
    auto it = std::find_if(m_users.begin(), m_users.end(),
			   [&toAddress](user_ptr_t user)
			   {
			     return user->m_id == toAddress;
			   });
    if (it == m_users.end()){
      Print(DBG_LEVEL_ERROR, "DlgBroker::private_message(): "
	    "There is no user with name '%s'", toAddress.c_str());
      return false;
    }
    return msg->Send(m_socket);
  }


  ///////////////////////////////////////////////////////////////////
  /////                      DlgService                         /////
  ///////////////////////////////////////////////////////////////////

  DlgService::DlgService(const std::string& name, const std::string& server_address) : m_name(name)
  {
    m_broker = std::make_shared<DlgBroker>(server_address);
  }

  DlgService::~DlgService()
  {
  }

  bool DlgService::HasUsers()
  {
    return m_broker->HasUsers();
  }

  bool DlgService::AddUser(message_ptr_t msg)
  {
    return m_broker->AddUser(msg->GetFromAddress());
  }
  
}//end of namespace
