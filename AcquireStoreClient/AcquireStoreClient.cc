
#include "AcquireStoreClient.h"	

CAcquireStoreClient::CAcquireStoreClient():m_pProcessor(new CPostProcessor()){
}

CAcquireStoreClient::~CAcquireStoreClient() {
	delete m_pProcessor;
}


std::vector<std::string> CAcquireStoreClient::scan_ip(std::string& start_ip, std::string& end_ip) {
	return scan_ip(start_ip, end_ip);
}


BOOL CAcquireStoreClient::connect(const std::string& ip, const uint16_t port) {
	try {
		WORD wVersionRequested;
		WSADATA wsaData;
		int err;
		wVersionRequested = MAKEWORD(2, 2);
		err = WSAStartup(wVersionRequested, &wsaData);

		m_server_ip = ip;
		TSocket* p = new TSocket(ip, port);

		m_socket = boost::shared_ptr<TTransport>(p);

		p->setConnTimeout(300);
		p->setSendTimeout(300);
		p->setRecvTimeout(300);

		m_transport = boost::shared_ptr<TTransport>(new TBufferedTransport(m_socket));
		m_protocol = boost::shared_ptr<TProtocol>(new TBinaryProtocol(m_transport));

		m_client = new AcquireStoreServiceClient(m_protocol);
		m_client->getInputProtocol().get()->getTransport()->open();

		m_server_ip = ip;
		return TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}

BOOL CAcquireStoreClient::close() {
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

BOOL CAcquireStoreClient::is_connected() {
	try {
		return m_client->getInputProtocol().get()->getTransport()->isOpen();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}


int32_t CAcquireStoreClient::start(const int32_t snd_frame_rate) {
	try {
		int ret = m_client->start(snd_frame_rate);
		if (FALSE != ret ) {
			m_pProcessor->create_zmq_context(2, ret, m_server_ip, m_client->get_image_width() * m_client->get_image_height());
			return	ret;
		}
		
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}
int32_t CAcquireStoreClient::stop() {
	try {
		m_client->stop();
		m_pProcessor->destroy_zmq_contex();
		return	TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}
int32_t CAcquireStoreClient::set_snd_frame_rate(const int32_t snd_frame_rate, const int32_t full_frame_rate) {
	try {
		m_client->set_snd_frame_rate(snd_frame_rate, full_frame_rate);
		return	TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}
int32_t CAcquireStoreClient::set_store_file(const int32_t flag, const std::string& file_name) {
	try {
		m_client->set_store_file(flag, file_name);
		return	TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}
int32_t CAcquireStoreClient::do_pause(const int32_t flag) {
	try {
		m_client->do_pause(flag);
		return	TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}
int32_t CAcquireStoreClient::forward_play(const int64_t frame_seq, const int32_t snd_frame_rate) {
	try {
		m_client->forward_play(frame_seq, snd_frame_rate);
		return	TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}
int32_t CAcquireStoreClient::backward_play(const int64_t frame_seq, const int32_t snd_frame_rate) {
	try {
		m_client->backward_play(frame_seq, snd_frame_rate);
		return	TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}
int32_t CAcquireStoreClient::forward_play_temporary(const int64_t frame_seq, const int32_t snd_frame_rate) {
	try {
		m_client->forward_play_temporary(frame_seq, snd_frame_rate);
		return	TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}
int32_t CAcquireStoreClient::backward_play_temporary(const int64_t frame_seq, const int32_t snd_frame_rate) {
	try {
		m_client->backward_play_temporary(frame_seq, snd_frame_rate);
		return	TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}
