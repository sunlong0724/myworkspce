#ifndef __AGENT_CLIENT_H__
#define __AGENT_CLIENT_H__

#define DLL_API __declspec(dllexport)  

#include "AgentServerService.h"
#include "ZmqTransportDataImpl.h"

#include <thrift/Thrift.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace hawkeye;

class DLL_API AgentClient {
public:
	AgentClient();
	~AgentClient();

	BOOL			connect(const std::string& ip, const uint16_t port);
	BOOL			close();
	BOOL			is_connected();

	BOOL			find_cameras(std::map<std::string, std::map<int, std::string>>&);
	BOOL			get_hold_cameras(std::vector<std::string> & cameras);
	int32_t			add_cameras(const std::vector<std::string> & l);
	int32_t			del_cameras(const std::vector<std::string> & l);

	int32_t exec_acquire_store(const std::string& cmdline);
	int32_t kill_acquire_store(const int64_t process_id);
	
	CustomData						*m_custom_data;
private:
	AgentServerServiceClient		*m_client;
	boost::shared_ptr<TTransport>	m_socket;
	boost::shared_ptr<TTransport>	m_transport;
	boost::shared_ptr<TProtocol>	m_protocol;

	uint16_t						m_data_port;
	std::string						m_server_ip;

};


#endif // !__AGENT_CLIENT_H__

