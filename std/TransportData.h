#ifndef __SEND_DATA_H__
#define __SEND_DATA_H__

#include "defs.h"
#include "MyThread.h"
#include "utils.h"

class CSendData : public CMyThread{
public:
	void init(uint16_t port) {
		m_port = port;
		m_flag = FALSE;
		m_status = 0;
	}
	void set_parameters( int32_t elem_size) {
		m_elem_size = elem_size;
		m_flag = TRUE;
	}

	int64_t send(char* data, int64_t data_len) {
		return RingBuffer_write(m_ring_buffer, data, data_len);
	}
protected:
	void run() {
		timeBeginPeriod(1);
	

		while (!m_exited) {
			if (m_flag) {
				m_buffer.resize(m_elem_size, 0x00);
				m_flag = FALSE;
			}

			if (0 == m_status) {
				m_zmq_ctx = zmq_ctx_new();
				//zmq_ctx_set(m_zmq_ctx, ZMQ_IO_THREADS, 2);
				std::string addr("tcp://");
				int high_water_mark = 9;

				addr.append("*:" + std::to_string(m_port));
				m_zmq_sock = zmq_socket(m_zmq_ctx, ZMQ_PUB);
				zmq_setsockopt(m_zmq_sock, ZMQ_SNDHWM, &high_water_mark, sizeof high_water_mark);
				zmq_bind(m_zmq_sock, addr.c_str());

				m_ring_buffer = RingBuffer_create(ONE_MB * MAX_CACHE_IMAGE_COUNT);
				m_status = 1;
			}

			if ( m_buffer.size()  == 0|| -1 == RingBuffer_read(m_ring_buffer, m_buffer.data(), m_buffer.size())) {
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

private:
	int64_t do_send(char* data, int64_t data_len) {
		int ret = zmq_send(m_zmq_sock, data, data_len, ZMQ_DONTWAIT);
		if (ret == -1) {
			int err = zmq_errno();
			if (err == EAGAIN) {
				return -1;
			}
			fprintf(stdout, "%s zmq_recv ret %d\n", __FUNCTION__, ret);
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

public:
	CFPSCounter			m_soft_snd_counter;
private:
	void				*m_zmq_ctx;
	void				*m_zmq_sock;
	std::vector<char>	m_buffer;

	int64_t				m_frame_counter;
	int64_t				m_last_snd_seq;
	RingBuffer			*m_ring_buffer;

	uint16_t			m_port;
	int32_t				m_elem_size;
	BOOL				m_flag;
	int					m_status;
	
};

class CRecvData : public CMyThread {
public:
	void init(std::string& server_ip, uint16_t port) {
		m_server_ip = server_ip;
		m_port = port;
		m_flag = FALSE;
		m_status = 0;
	}
	void set_parameters( int32_t elem_size, int64_t frame_gap = 1) {
		m_elem_size = elem_size;
		m_frame_gap = frame_gap;
		m_flag = TRUE;
	}

	void set_sink_data_callback(SinkDataCallback cb, void* context) {
		m_cb = cb;
		m_cb_ctx = context;
	}

protected:
	void run() {


		while (!m_exited) {
			if (m_flag) {
				m_flag = FALSE;
				m_buffer.resize(m_elem_size, 0x00);
			}

			if (0 == m_status) {
				m_zmq_ctx = zmq_ctx_new();
				//zmq_ctx_set(m_zmq_ctx, ZMQ_IO_THREADS, 2);
				std::string addr("tcp://");

				addr.append(m_server_ip + ":" + std::to_string(m_port));
				m_zmq_sock = zmq_socket(m_zmq_ctx, ZMQ_SUB);
				int ret = zmq_connect(m_zmq_sock, addr.c_str());
				if (ret == -1) {
					int err = zmq_errno();
					fprintf(stdout, "zmq_connect err %d\n", err);
				}
				zmq_setsockopt(m_zmq_sock, ZMQ_SUBSCRIBE, "", 0);

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
	}

private:
	int64_t do_recv() {
		int ret = zmq_recv(m_zmq_sock, (void*)m_buffer.data(), m_buffer.size(), ZMQ_DONTWAIT);
		if (ret == -1) {
			int err = zmq_errno();
			if (err == EAGAIN) {
				return -1;
			}
			fprintf(stdout, "%s zmq_recv ret %d\n", __FUNCTION__, ret);
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
	BOOL				m_flag;

	SinkDataCallback	m_cb;
	void*				m_cb_ctx;
	int					m_status;
};

#endif // !__SEND_DATA_H__
