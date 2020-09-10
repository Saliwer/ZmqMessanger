#include "DlgServer.h"

namespace ZmqMessanger
{
  DlgServer::DlgServer(const std::string& address) : m_address(address)
  {
    m_socket = nullptr;
    m_thread = nullptr;
    m_handlers[JOIN_SERVICE] = &DlgServer::join_service_callback;
    m_handlers[CREATE_SERVICE] = &DlgServer::create_service_callback;
    m_handlers[DELETE_SERVICE] = &DlgServer::delete_service_callback;
    
    m_stop = true;
    Print(DBG_LEVEL_DEBUG, "DlgServer::DlgServer(): "
	  "Server was created.\n");
  }

  DlgServer::DlgServer(const char* address) : DlgServer(std::string(address))
  {
  }

  DlgServer::~DlgServer()
  {
    m_stop = true;
    if (m_thread && m_thread->joinable())
      m_thread->join();
  }

  void DlgServer::Start()
  {
    if (!m_socket){
      try{
	m_socket = ZMQ::Instance()->CreateSocket(ZMQ_ROUTER);
	std::string endpoint = TRANSPORT_PROTOCOL + m_address;
	m_socket->bind(endpoint.data());
      }
      catch(zmq::error_t& e){
	Print(DBG_LEVEL_ERROR, "DlgServer::Start(): "
	      "zmq::exception %s\n", e.what());
	throw Exception("DlgServer::Start(): fatal error.");
      }
      catch(std::exception& e){
	Print(DBG_LEVEL_ERROR, "DlgServer::Start(): "
	      "std::exception %s\n", e.what());
	throw Exception("DlgServer::Start():  fatal error.");
      }
      catch(...){
	Print(DBG_LEVEL_ERROR, "DlgServer::Start(): unknown exeption.\n");
	throw Exception("DlgServer::Start(): fatal error.");
      }
      
      Print(DBG_LEVEL_DEBUG, "DlgServer::Start(): "
	    "Server is bound to '%s' address.\n", m_address.c_str());
    }
    
    m_stop = false;
    m_thread =
      std::make_shared<std::thread>(&DlgServer::main_thread, this);
  }

  void DlgServer::Stop()
  {
    m_stop = true;
    if (m_thread->joinable())
      m_thread->join();
  }

  void DlgServer::main_thread()
  {
    Print(DBG_LEVEL_DEBUG, "DlgServer::main_thread(): "
	  "Start of main thread.\n");
    while(!m_stop){
      zmq::pollitem_t items[] =
	{
	  { static_cast<void*>(*m_socket), 0, ZMQ_POLLIN, 0 }
	};
      zmq::poll(items, 1, 0);
      if (items[0].revents & ZMQ_POLLIN){
	message_ptr_t msg = std::make_shared<DlgMessage>();
	if (!msg->Recv(m_socket)){
	  Print(DBG_LEVEL_ERROR, "DlgServer::main_thread(): "
		"Coudn't recieve message.\n");
	  continue;
	}

	auto it = m_handlers.find(msg->GetMessageType());
	if (it == m_handlers.end()){
	  Print(DBG_LEVEL_ERROR, "DlgServer::main_thread(): "
	      "Unknown message type.\n");
	  continue;
	}
	else{
	  //Calling method pointer
	  //Of course it would be better to use std::invoke
	  //std::invoke(m_handlers[msgType], *this, params...);
	  if (!(this->*it->second)(msg))
	    {
	      Print(DBG_LEVEL_ERROR, "DlgServer::send_thread(): "
		    "Something went wrong during msg parsing.\n");
	      continue;
	    } 
	}	
      }     
    }//end of loop
    Print(DBG_LEVEL_DEBUG, "DlgServer::main_thread(): "
	  "End of main thread.\n");
  }//end of main_thread()

  bool DlgServer::join_service_callback(message_ptr_t msg)
  {
    //...
    return true;
  }

  bool DlgServer::create_service_callback(message_ptr_t msg)
  {
    std::string name = msg->GetMessageStrBody();
    auto it = std::find_if(m_services.begin(), m_services.end(),
			   [&name](service_ptr_t service){
			     return name == service->GetName();
			   });
    if (it != m_services.end()){
      Print(DBG_LEVEL_ERROR, "DlgServer::create_service_callback():"
	    " Service with name '%s' is already exists.\n",
	    name.c_str());
      return false;
    }
    Print(DBG_LEVEL_DEBUG, "DlgServer::create_service_callback(): "
	  " Creating service with name '%s'.\n",
	  name.c_str());
    service_ptr_t newService =
      std::make_shared<DlgService>(name, m_address);
    m_services.push_back(newService);
    return true;
  }

  bool DlgServer::delete_service_callback(message_ptr_t msg)
  {
    std::string name = msg->GetMessageStrBody();
    auto it = std::find_if(m_services.begin(), m_services.end(),
			   [&name](service_ptr_t service){
			     return name == service->GetName();
			   });
    if (it == m_services.end()){
      Print(DBG_LEVEL_ERROR, "DlgServer::delete_service_callback():"
	    " There is no service with name '%s'.\n", name.c_str());
      return false;
    }
    else{
      Print(DBG_LEVEL_DEBUG, "DlgServer::delete_service_callback(): "
	    " Deleting service with name '%s'.\n",
	    (*it)->GetName().c_str());
      m_services.erase(it);
    }
      return true;
  }
  
}//end of namespace

