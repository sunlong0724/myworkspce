#ifndef __SEND_DATA_H__
#define __SEND_DATA_H__

#include "MyThread.h"
#include "utils.h"

class CSendData : public CMyThread{
public:
	void init(uint16_t port);
	void set_parameters(int32_t elem_size);

	int64_t send(char* data, int64_t data_len);
protected:
	void run();
private:
	int64_t do_send(char* data, int64_t data_len);

public:
	CFPSCounter			m_soft_snd_counter;
	int64_t				m_last_snd_seq;
private:
	void				*m_zmq_ctx;
	void				*m_zmq_sock;
	std::vector<char>	m_buffer;

	int64_t				m_frame_counter;
	RingBuffer			*m_ring_buffer;

	uint16_t			m_port;
	int32_t				m_elem_size;
	BOOL				m_flag;
	int					m_status;
	
};

class CRecvData : public CMyThread {
public:
	void init(std::string& server_ip, uint16_t port);
	void set_parameters(int32_t elem_size, int64_t frame_gap = 1);

	void set_sink_data_callback(SinkDataCallback cb, void* context);
protected:
	void run();

private:
	int64_t do_recv();

public:
	CFPSCounter			m_soft_recv_counter;
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
