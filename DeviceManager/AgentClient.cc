#include "AgentClient.h"


AgentClient::AgentClient() {
	m_custom_data = new CustomData;
}

AgentClient::~AgentClient() {
	delete m_custom_data;
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

		p->setConnTimeout(30000);
		p->setSendTimeout(30000);
		p->setRecvTimeout(30000);

		m_transport = boost::shared_ptr<TTransport>(new TBufferedTransport(m_socket));
		m_protocol = boost::shared_ptr<TProtocol>(new TBinaryProtocol(m_transport));

		m_client = new AgentServerServiceClient(m_protocol);
		m_client->getInputProtocol().get()->getTransport()->open();

		m_server_ip = ip;
		return TRUE;
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
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
		fprintf(stderr, "%s\n", e.what());
		return FALSE;
	}
}

BOOL AgentClient::is_connected() {
	try {
		return m_client->getInputProtocol().get()->getTransport()->isOpen();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return FALSE;
	}
}

BOOL AgentClient::find_cameras(std::map<std::string, std::map<int, std::string>>& _return){
	try {
		m_client->find_cameras(_return);
		return	TRUE;
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return FALSE;
	}
}

int32_t AgentClient::add_cameras(const std::vector<std::string> & l) {
	try {
		return m_client->add_cameras(l);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}

BOOL AgentClient::get_hold_cameras(std::vector<std::string> & cameras) {
	try {
		m_client->get_hold_cameras( cameras);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return FALSE;
	}
	return TRUE;
}

int32_t AgentClient::del_cameras(const std::vector<std::string> & l) {
	try {
		return m_client->del_cameras(l);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}

int32_t AgentClient::exec_acquire_store(const std::string& cmdline) {
	try {
		return m_client->exec_acquire_store(cmdline);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t AgentClient::kill_acquire_store(const int64_t process_id) {
	try {
		return m_client->kill_acquire_store(process_id);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}


