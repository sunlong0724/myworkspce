#include "AgentClient.h"
#include "utils.h"

CAgentClient::CAgentClient() {
}

CAgentClient::~CAgentClient() {
}

//can scan 192.168.0.1~192.168.1.255 eg.
std::vector<std::string> CAgentClient::scan_ip(std::string& start_ip, std::string& end_ip) {
	return scan_ip0(start_ip, end_ip);
}

BOOL CAgentClient::connect(const std::string& ip, const uint16_t port) {
	try {
		WORD wVersionRequested;
		WSADATA wsaData;
		int err;
		wVersionRequested = MAKEWORD(2, 2);
		err = WSAStartup(wVersionRequested, &wsaData);

		TSocket* p = new TSocket(ip, port);

		m_socket = boost::shared_ptr<TTransport>(p);

		p->setConnTimeout(300);
		p->setSendTimeout(300);
		p->setRecvTimeout(300);

		m_transport = boost::shared_ptr<TTransport>(new TBufferedTransport(m_socket));
		m_protocol = boost::shared_ptr<TProtocol>(new TBinaryProtocol(m_transport));

		m_client = new AgentServerServiceClient(m_protocol);
		m_client->getInputProtocol().get()->getTransport()->open();

		m_status = ConnectStatus_CONNECTED;
		return TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return FALSE;
	}
}

BOOL CAgentClient::close() {
	try {
		m_client->getInputProtocol().get()->getTransport()->close();
		delete m_client;
		m_protocol.reset();
		m_transport.reset();
		m_socket.reset();
		WSACleanup();
		return TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}

BOOL CAgentClient::is_connected() {
	try {
		return m_client->getInputProtocol().get()->getTransport()->isOpen();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return FALSE;
	}
}


void CAgentClient::set_connected_callback(ConnectedCallback cb, void* ctx) {
	m_connect_callback = cb;
	m_connect_ctx = ctx;
}

void	CAgentClient::set_connect_parameters(const std::string& ip, const uint16_t port) {
	m_server_ip = ip;
	m_cmd_port = port;
}


void CAgentClient::run() {

	m_status = ConnectStatus_NONE;

	while (!m_exited) {
		if (m_status == ConnectStatus_NONE) {

			connect(m_server_ip, m_cmd_port);
			if (m_connect_callback && m_last_status != m_status) {
				m_last_status = m_status;
				m_connect_callback(m_status, m_connect_ctx);
			}
		}else 	if (m_status == ConnectStatus_DISCONNECT) {
			close();
			connect(m_server_ip, m_cmd_port);

			if (m_connect_callback &&  m_last_status != m_status) {
				m_last_status = m_status;
				m_connect_callback(m_status, m_connect_ctx);
			}
		}
		else {
			try {
				connect_pingpong();
				//m_client_pingpong->get_cpu_usage();//check as PING
				close_pingpong();
			}
			catch (TException& e) {
				close_pingpong();
				fprintf(stdout, "%s\n", e.what());
				m_status = ConnectStatus_DISCONNECT;
			}
		}

		sleep(500);
	}
	close();
}

void    CAgentClient::start() {
	CMyThread::start();
}
void	CAgentClient::stop() {
	CMyThread::stop();
}


void CAgentClient::connect_pingpong() {
	try {
		WORD wVersionRequested;
		WSADATA wsaData;
		int err;
		wVersionRequested = MAKEWORD(2, 2);
		err = WSAStartup(wVersionRequested, &wsaData);

		TSocket* p = new TSocket(m_server_ip, m_cmd_port);

		m_socket_pingpong = boost::shared_ptr<TTransport>(p);

		p->setConnTimeout(500);
		p->setSendTimeout(300);
		p->setRecvTimeout(300);

		m_transport_pingpong = boost::shared_ptr<TTransport>(new TBufferedTransport(m_socket_pingpong));
		m_protocol_pingpong = boost::shared_ptr<TProtocol>(new TBinaryProtocol(m_transport_pingpong));

		m_client_pingpong = new AgentServerServiceClient(m_protocol_pingpong);
		m_client_pingpong->getInputProtocol().get()->getTransport()->open();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
	}
}

void CAgentClient::close_pingpong() {
	m_client_pingpong->getInputProtocol().get()->getTransport()->close();
	delete m_client_pingpong;
	m_protocol_pingpong.reset();
	m_transport_pingpong.reset();
	m_socket_pingpong.reset();
	WSACleanup();
}


BOOL CAgentClient::find_cameras(std::map<std::string, std::map<int, std::string>>& _return){
	try {
		m_client->find_cameras(_return);
		return	TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return FALSE;
	}
}


int32_t CAgentClient::exec_program(const std::string& cmdline) {
	// Your implementation goes here
	try {
		return m_client->exec_program(cmdline);
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1;
	}
}

int32_t CAgentClient::kill_program(const int64_t process_id) {
	// Your implementation goes here
	try {
		return m_client->kill_program(process_id);
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1;
	}
}

BOOL CAgentClient::get_disk_info(std::map<std::string, std::vector<double> >& _return) {
	try {
		m_client->get_disk_info(_return);
		return TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1;
	}
}

int32_t CAgentClient::get_cpu_usage() {
	try {
		return m_client->get_cpu_usage();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1;
	}
}


