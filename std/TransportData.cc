#include "TransportData.h"

#include "defs.h"

#define ZMQ_IO_THREADS_USED 2
#define ZMQ_HIGHT_WATER_MARKER 1

void CSendData::init(uint16_t port) {
	m_port = port;
	m_flag = FALSE;
	m_status = 0;
}
void CSendData::set_parameters(int32_t elem_size) {
	m_elem_size = elem_size;
	m_flag = TRUE;
}

int64_t CSendData::send(char* data, int64_t data_len) {

	std::lock_guard<std::mutex> lg(m_mutex);
	if (m_flag) {
		if (m_ring_buffer)
			RingBuffer_destroy(m_ring_buffer);
		m_ring_buffer = RingBuffer_create(m_elem_size , 1);//FIXME:
		m_flag = FALSE;
	}
	return RingBuffer_write(m_ring_buffer, data, data_len);
}


int64_t CSendData::recv(std::vector<char>& recv_buffer) {
	std::lock_guard<std::mutex> lg(m_mutex);
	if (recv_buffer.size() != m_elem_size) {
		recv_buffer.resize(m_elem_size, 0x00);
		if (m_ring_buffer)
			RingBuffer_clear(m_ring_buffer);
	}

	if (NULL == m_ring_buffer)
		m_ring_buffer = RingBuffer_create(m_elem_size ,1);//FIXME:

	if (-1 == RingBuffer_read(m_ring_buffer, recv_buffer.data(), recv_buffer.size())) {
		return -1;
	}
	return recv_buffer.size();
}
void CSendData::run() {
	timeBeginPeriod(1);

	
	while (!m_exited) {
		//if (m_flag) {
		//	m_buffer.resize(m_elem_size, 0x00);
		//	if (m_ring_buffer)
		//		RingBuffer_destroy(m_ring_buffer);
		//	m_ring_buffer = RingBuffer_create(m_elem_size * 1);//FIXME:
		//	m_flag = FALSE;
		//}

		if (m_data_transport_mode == 2) {//thrift
			sleep(1);
			continue;
		}

		if (0 == m_status) {
			int rc = 0;
			m_zmq_ctx = zmq_ctx_new();
			//zmq_ctx_set(m_zmq_ctx, ZMQ_IO_THREADS, ZMQ_IO_THREADS_USED);
			std::string addr("tcp://");

			addr.append("*:" + std::to_string(m_port));
			m_zmq_sock = zmq_socket(m_zmq_ctx, ZMQ_PUB);
			
			int high_water_mark = ZMQ_HIGHT_WATER_MARKER * 2;
			zmq_setsockopt(m_zmq_sock, ZMQ_SNDHWM, &high_water_mark, sizeof high_water_mark);

			//int64_t snd_buf_size = ONE_MB * 100;
			//rc = zmq_setsockopt(socket, ZMQ_SNDBUF, &snd_buf_size, sizeof snd_buf_size);

			zmq_bind(m_zmq_sock, addr.c_str());
			m_ring_buffer = RingBuffer_create(m_elem_size ,1);
			m_status = 1;
		}

		if (m_buffer.size() == 0 || -1 == RingBuffer_read(m_ring_buffer, m_buffer.data(), m_buffer.size())) {
			//fprintf(stdout, "%s RingBuffer_read no data\n", __FUNCTION__);
			sleep(2);
			continue;
		}

		PRINT_FRAME_INFO(m_buffer.data());
		int ret = do_send(m_buffer.data(), m_buffer.size());
		if (m_buffer.size() != ret) {
			fprintf(stdout, "%s do_send ret %d\n", ret);
		}
	}

	RingBuffer_destroy(m_ring_buffer);
	zmq_close(m_zmq_sock);
	zmq_ctx_destroy(m_zmq_ctx);
	timeEndPeriod(1);
}

int64_t CSendData::do_send(char* data, int64_t data_len) {
	int ret = zmq_send(m_zmq_sock, data, data_len,  ZMQ_DONTWAIT);
	if (ret == -1) {
		int err = zmq_errno();
		if (err == EAGAIN) {
			return -1;
		}
		fprintf(stdout, "%s zmq_send ret %d err %d\n", __FUNCTION__, ret, err);
		if (1 == m_status) {
			if (m_zmq_sock) {
				zmq_close(m_zmq_sock);
			}
			if (m_zmq_ctx) {
				zmq_ctx_destroy(m_zmq_ctx);
			}
			m_status = 0;
		}

		return -1;
	}

	int64_t timestamp;
	int64_t frame_no;
	memcpy(&timestamp, &data[FRAME_TIMESTAMP_START], sizeof int64_t);
	memcpy(&frame_no, &data[FRAME_SEQ_START], sizeof int64_t);

	m_soft_snd_counter.statistics(__FUNCTION__, FALSE);
	//printf("snd %d bytes, seq:%lld, sndno:%lld,timestamp:%lld\r", ret, frame_no, frame_no, timestamp);
	m_last_snd_seq = frame_no;
	return ret;
}
CRecvData::CRecvData():m_cb(NULL),m_cb_ctx(NULL){
}

void CRecvData::init(std::string& server_ip, uint16_t port, int64_t frame_gap) {
	m_server_ip = server_ip;
	m_port = port;
	m_status = 0;
	m_frame_gap = frame_gap;
}


void CRecvData::set_sink_data_callback(SinkDataCallback cb, void* context) {
	m_cb = cb;
	m_cb_ctx = context;
}

void CRecvData::run() {

	timeBeginPeriod(1);
	m_buffer_len = ONE_MB * 3;
	m_buffer = new char[m_buffer_len];

	while (!m_exited) {
		if (0 == m_status) {
			int rc = 0;
			m_zmq_ctx = zmq_ctx_new();
			//zmq_ctx_set(m_zmq_ctx, ZMQ_IO_THREADS, ZMQ_IO_THREADS_USED);
			std::string addr("tcp://");

			addr.append(m_server_ip + ":" + std::to_string(m_port));
			//addr.append(std::string("127.0.0.1") + ":" + std::to_string(m_port));

			m_zmq_sock = zmq_socket(m_zmq_ctx, ZMQ_SUB);
			int high_water_mark = ZMQ_HIGHT_WATER_MARKER;
			rc = zmq_setsockopt(m_zmq_sock, ZMQ_RCVHWM, &high_water_mark, sizeof high_water_mark);

			//int64_t affinity = 1;
			//rc = zmq_setsockopt(socket, ZMQ_AFFINITY, &affinity, sizeof affinity);

			//int64_t recv_buf_size = ONE_MB*100;
			//rc = zmq_setsockopt(socket, ZMQ_RCVBUF, &recv_buf_size, sizeof recv_buf_size);

			int ret = zmq_connect(m_zmq_sock, addr.c_str());
			if (ret == -1) {
				int err = zmq_errno();
				fprintf(stdout, "zmq_connect err %d\n", err);
			}
			rc = zmq_setsockopt(m_zmq_sock, ZMQ_SUBSCRIBE, "", 0);

			m_status = 1;
		}

		
		if (-1 == do_recv()) {
			sleep(2);
			continue;
		}
	}
	if (m_zmq_sock)
		zmq_close(m_zmq_sock);
	if (m_zmq_ctx)
		zmq_ctx_destroy(m_zmq_ctx);

	delete[] m_buffer;

	timeEndPeriod(1);
}
int64_t CRecvData::do_recv() {

	m_soft_recv_counter.statistics(__FUNCTION__, FALSE);
	int ret = zmq_recv(m_zmq_sock, m_buffer, m_buffer_len,  ZMQ_DONTWAIT);
	
	if (ret == -1) {
		int err = zmq_errno();
		if (err == EAGAIN) {
			return -1;
		}
		fprintf(stdout, "%s zmq_recv ret %d err %d\n", __FUNCTION__, ret, err);
		if (1 == m_status) {
			if (m_zmq_sock) {
				zmq_close(m_zmq_sock);
			}
			if (m_zmq_ctx) {
				zmq_ctx_destroy(m_zmq_ctx);
			}
			m_status = 0;
		}
		return -1;
	}

	int64_t timestamp = 0;
	int64_t recv_seq = 0;
	memcpy(&timestamp, &m_buffer[FRAME_TIMESTAMP_START], sizeof int64_t);
	memcpy(&recv_seq, &m_buffer[FRAME_SEQ_START], sizeof int64_t);

	if (std::abs(recv_seq - m_last_recv_seq) != m_frame_gap) {
		//printf("#recv %d bytes,seq:%lld,last_seq:%lld,frame_gap:%lld,timestamp:%lld\n", ret, recv_seq, m_last_recv_seq, m_frame_gap, timestamp);
	}
	else {
		//printf("recv %d bytes,seq:%lld,last_seq:%lld,timestamp:%lld\n", ret, recv_seq, m_last_recv_seq, timestamp);
	}
	m_last_recv_seq = recv_seq;

	if (m_cb)
		m_cb((unsigned char*)m_buffer, ret, m_cb_ctx);

	
	return ret;
}


///////////////////////////////////////////////////////////////////////

#include "PlaybackCtrlService.h"

#include <thrift/Thrift.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace hawkeye;

int g_transport_mode = 2;//1--zmq 2--thrift

CRecvDatabyThrift::CRecvDatabyThrift():m_cb(NULL), m_cb_ctx(NULL) {

}

void CRecvDatabyThrift::init(std::string& server_ip, uint16_t port, int64_t frame_gap) {
	m_server_ip = server_ip;
	m_port = port;
	m_status = 0;
}




void CRecvDatabyThrift::set_sink_data_callback(SinkDataCallback cb, void* context) {
	m_cb = cb;
	m_cb_ctx = context;
}

void CRecvDatabyThrift::run() {

	timeBeginPeriod(1);

	boost::shared_ptr<TTransport>	m_socket;
	boost::shared_ptr<TTransport>	m_transport;
	boost::shared_ptr<TProtocol>	m_protocol;
	PlaybackCtrlServiceClient		*m_client_data = NULL;

	std::string						image_data;

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);

	m_status = 0;
	while (!m_exited) {
		if (0 == m_status) {
			try {
				TSocket* p = new TSocket(m_server_ip, m_port);

				m_socket = boost::shared_ptr<TTransport>(p);

				p->setConnTimeout(50);
				p->setSendTimeout(30);
				p->setRecvTimeout(30);

				m_transport = boost::shared_ptr<TTransport>(new TBufferedTransport(m_socket));
				m_protocol = boost::shared_ptr<TProtocol>(new TBinaryProtocol(m_transport));

				m_client_data = new PlaybackCtrlServiceClient(m_protocol);
				m_client_data->getInputProtocol().get()->getTransport()->open();
			}
			catch (TException& e) {
				delete m_client_data;
				//m_client_data->getInputProtocol().get()->getTransport()->close();
				fprintf(stdout, "%s\n", e.what());
				sleep(100);
				continue;
			}
			m_status = 1;
		}

		try
		{
			m_client_data->get_frame_data(image_data, 0);
			if (image_data.size() > 0) {
				if (m_cb) {
					m_cb((unsigned char*)image_data.data(), image_data.size(), m_cb_ctx);
				}
				/*else {
					char name[100];
					sprintf(name, "%d.txt", m_port);
					FILE* fp = fopen(name, "wb");
					if (fp) {
						char info[100] = {0};
						sprintf(info, "%d %d %p\n", time(NULL), image_data.size(), m_cb);
						fwrite(info, 1, strlen(info), fp);
						fclose(fp);
					}
				}*/
			}
			sleep(5);
		}
		catch (TException& e)
		{
			try
			{
				m_client_data->getInputProtocol().get()->getTransport()->close();
			}
			catch (TException& e)
			{
				;// fprintf(stdout, "%s\n", e.what());
			}
			delete m_client_data;
			fprintf(stdout, "%s\n", e.what());
			sleep(100);
			m_status = 0;
		}
	}

	
	timeEndPeriod(1);
}

