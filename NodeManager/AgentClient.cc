#include "AgentClient.h"
#include "ping.h"

AgentClient::AgentClient() {
}

AgentClient::~AgentClient() {
}

//can scan 192.168.0.1~192.168.1.255 eg.
std::vector<std::string> AgentClient::scan_ip(std::string& start_ip, std::string& end_ip) {
	return scan_ip(start_ip, end_ip);
}

BOOL AgentClient::connect(const std::string& ip, const uint16_t port) {
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

		m_server_ip = ip;
		return TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}

BOOL AgentClient::close() {
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

BOOL AgentClient::is_connected() {
	try {
		return m_client->getInputProtocol().get()->getTransport()->isOpen();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}

BOOL AgentClient::find_cameras(std::map<std::string, std::map<int, std::string>>& _return){
	try {
		m_client->find_cameras(_return);
		return	TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}

int32_t AgentClient::add_cameras(const std::vector<std::string> & l) {
	try {
		return m_client->add_cameras(l);
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return -1;
	}
}

BOOL AgentClient::get_hold_cameras(std::vector<std::string> & cameras) {
	try {
		m_client->get_hold_cameras( cameras);
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
	return TRUE;
}

int32_t AgentClient::del_cameras(const std::vector<std::string> & l) {
	try {
		return m_client->del_cameras(l);
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return -1;
	}
}

int32_t AgentClient::exec_program(const std::string& cmdline) {
	// Your implementation goes here
	try {
		return m_client->exec_program(cmdline);
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return -1;
	}

}

int32_t AgentClient::kill_program(const int64_t process_id) {
	// Your implementation goes here
	try {
		return m_client->kill_program(process_id);
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return -1;
	}
}


