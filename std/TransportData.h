#ifndef __SEND_DATA_H__
#define __SEND_DATA_H__

#include "defs.h"
#include "MyThread.h"
#include "utils.h"

class CSendData : public CMyThread{
public:
	void init(int port, int32_t elem_size) {
		m_port = port;
		m_elem_size = elem_size;

	}

	int64_t send(char* data, int64_t data_len) {
		return RingBuffer_write(m_ring_buffer, data, data_len);
	}
protected:
	void run() {
		m_zmq_ctx = zmq_ctx_new();
		zmq_ctx_set(m_zmq_ctx, ZMQ_IO_THREADS, 2);
		std::string addr("tcp://");
		int high_water_mark = CAMERA_MAX_FPS * 9;

		addr.append("*:" + std::to_string(m_port));
		m_zmq_sock = zmq_socket(m_zmq_ctx, ZMQ_PUB);
		zmq_setsockopt(m_zmq_sock, ZMQ_SNDHWM, &high_water_mark, sizeof high_water_mark);
		zmq_bind(m_zmq_sock, addr.c_str());

		m_buffer.resize(m_elem_size, 0x00);
		m_ring_buffer = RingBuffer_create(m_elem_size * 10);

		//m_status = TRANSPORT_STATUS_NONE;
		while (!m_exited) {
			if (-1 == RingBuffer_read(m_ring_buffer, m_buffer.data(), m_buffer.size())) {
				sleep(2);
				continue;
			}

			int ret = do_send(m_buffer.data(), m_buffer.size());
			if (m_buffer.size() != ret) {
				fprintf(stdout, "%s do_send ret %d\n", ret);
			}
		}

		RingBuffer_destroy(m_ring_buffer);
		zmq_close(m_zmq_sock);
		zmq_ctx_destroy(m_zmq_ctx);
	}

private:
	int64_t do_send(char* data, int64_t data_len) {
		int ret = zmq_send(m_zmq_sock, data, data_len, 0);// ZMQ_DONTWAIT);
		if (ret < 0) {
			fprintf(stdout, "%s zmq_send ret %d\n", __FUNCTION__, ret);
			return -1;
		}

		int64_t timestamp;
		int64_t frame_no;
		memcpy(&timestamp, &data[FRAME_TIMESTAMP_START], sizeof int64_t);
		memcpy(&frame_no, &data[FRAME_SEQ_START], sizeof int64_t);

		printf("snd %d bytes, seq:%lld, sndno:%lld,timestamp:%lld\r", ret, frame_no, frame_no, timestamp);
		m_last_snd_seq = frame_no;
		return ret;
	}

private:
	void				*m_zmq_ctx;
	void				*m_zmq_sock;
	std::vector<char>	m_buffer;

	int64_t				m_frame_counter;
	int64_t				m_last_snd_seq;
	RingBuffer			*m_ring_buffer;

	uint16_t			m_port;
	int32_t				m_elem_size;


};

class CRecvData : public CMyThread {
public:
	void set_parameters(std::string& server_ip, uint16_t port, int32_t elem_size, int64_t frame_gap = 1) {
		m_server_ip = server_ip;
		m_port = port;
		m_elem_size = elem_size;
		m_frame_gap = frame_gap;
	}

	void set_sink_data_callback(SinkDataCallback cb, void* context) {
		m_cb = cb;
		m_cb_ctx = context;
	}

protected:
	void run() {
		m_zmq_ctx = zmq_ctx_new();
		zmq_ctx_set(m_zmq_ctx, ZMQ_IO_THREADS, 2);
		std::string addr("tcp://");

		addr.append(m_server_ip + ":" + std::to_string(m_port));
		m_zmq_sock = zmq_socket(m_zmq_ctx, ZMQ_SUB);
		zmq_connect(m_zmq_sock, addr.c_str());
		zmq_setsockopt(m_zmq_sock, ZMQ_SUBSCRIBE, "", 0);

		m_buffer.resize(m_elem_size, 0x00);
		while (!m_exited) {
			if (-1 == do_recv()) {
				sleep(2);
				continue;
			}
			sleep(5);//FIXME:
		}

		zmq_close(m_zmq_sock);
		zmq_ctx_destroy(m_zmq_ctx);
	}

private:
	int64_t do_recv() {
		int ret = zmq_recv(m_zmq_sock, (void*)m_buffer.data(), m_buffer.size(), 0);// ZMQ_DONTWAIT);
		if (ret < 0) {
			fprintf(stdout, "%s zmq_recv ret %d\n", __FUNCTION__, ret);
			return -1;
		}

		int64_t timestamp = 0;
		int64_t recv_seq = 0;
		memcpy(&timestamp, &m_buffer[FRAME_TIMESTAMP_START], sizeof int64_t);
		memcpy(&recv_seq, &m_buffer[FRAME_SEQ_START], sizeof int64_t);

		if (std::abs(recv_seq - m_last_recv_seq) != m_frame_gap) {
			printf("#recv %d bytes,seq:%lld,last_seq:%lld,frame_gap:%lld,timestamp:%lld\n", ret, recv_seq, m_last_recv_seq, m_frame_gap, timestamp);
		}
		else {
			printf("recv %d bytes,seq:%lld,last_seq:%lld,timestamp:%lld\n", ret, recv_seq, m_last_recv_seq, timestamp);
		}
		m_last_recv_seq = recv_seq;

		if (m_cb)
			m_cb((unsigned char*)m_buffer.data(), m_buffer.size(), m_cb_ctx);

		return m_buffer.size();
	}

private:
	void				*m_zmq_ctx;
	void				*m_zmq_sock;
	std::vector<char>	m_buffer;

	int64_t				m_frame_counter;
	int64_t				m_last_recv_seq;
	int64_t				m_frame_gap;

	std::string			m_server_ip;
	uint16_t			m_port;

	int32_t				m_elem_size;

	SinkDataCallback	m_cb;
	void*				m_cb_ctx;

};

#endif // !__SEND_DATA_H__
