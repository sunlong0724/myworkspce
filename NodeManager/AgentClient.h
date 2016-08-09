#ifndef __AGENT_CLIENT_H__
#define __AGENT_CLIENT_H__

#define DLL_API __declspec(dllexport)  

#include "AgentServerService.h"

#include <thrift/Thrift.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "MyThread.h"
#include "defs.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace hawkeye;

class DLL_API CAgentClient :public CMyThread {
public:
	CAgentClient();
	~CAgentClient();

	static std::vector<std::string> scan_ip(std::string& start_ip, std::string& end_ip);

	void			set_connected_callback(ConnectedCallback cb, void* ctx);
	void			set_connect_parameters(const std::string& ip, const uint16_t port);

	void			start();
	void			stop();
	BOOL			find_cameras(std::map<std::string, std::map<int, std::string>>&);

	int32_t			exec_program(const std::string& cmdline);
	int32_t			kill_program(const int64_t process_id);

	BOOL		    get_disk_info(std::map<std::string, std::vector<double> >& _return);
	int32_t			get_cpu_usage();


protected:
	void run();

private:

	BOOL			connect(const std::string& ip, const uint16_t port);
	BOOL			close();
	BOOL			is_connected();
	void			connect_pingpong();
	void			close_pingpong();

private:
	AgentServerServiceClient		*m_client;
	boost::shared_ptr<TTransport>	m_socket;
	boost::shared_ptr<TTransport>	m_transport;
	boost::shared_ptr<TProtocol>	m_protocol;

	AgentServerServiceClient		*m_client_pingpong;
	boost::shared_ptr<TTransport>	m_socket_pingpong;
	boost::shared_ptr<TTransport>	m_transport_pingpong;
	boost::shared_ptr<TProtocol>	m_protocol_pingpong;


	std::string						m_server_ip;
	uint16_t						m_cmd_port;

	int32_t			m_acquire_store_process_id;

	ConnectStatus					m_status;
	ConnectStatus					m_last_status;
	ConnectedCallback				m_connect_callback;
	void*							m_connect_ctx;
};


#endif // !__AGENT_CLIENT_H__

