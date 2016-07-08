#ifndef __SEND_DATA_IMPL_H__
#define __SEND_DATA_IMPL_H__

#include <string>
#include <vector>

#include "zmq.h"

int SinkBayerDatasCallbackImpl(unsigned char* buffer, int buffer_len, void* ctx);

class CStoreFile {

public:
	CStoreFile(int64_t max_file_size = 0) :m_max_file_size(max_file_size), m_hFile(NULL) {}

	~CStoreFile() {
		CloseHandle(m_hFile);
	}

	static void set_file_name(const std::string& file_name) {
		m_file_name.append(file_name);
	}

	int64_t  write_file(char* buffer, int64_t buffer_len) {
		if (!m_hFile) {
			m_hFile = CreateFile(m_file_name.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, NULL);
			if (!m_hFile) {
				fprintf(stdout, "fopen failed!!!\n");
				return -1;
			}
		}

		DWORD nBytesWritten = 0;
		BOOL ret = WriteFile(m_hFile, buffer, buffer_len, &nBytesWritten, NULL);
		if (FALSE == ret) {
			fprintf(stdout, "WriteFile failed!!!\n");
		}

		LARGE_INTEGER FileSize;
		BOOL r = GetFileSizeEx(m_hFile, &FileSize);
		if (!r) {
			printf("GetFileSizeEx failed\n");
			return FALSE;
		}

		if (FileSize.QuadPart >= m_max_file_size) {
			SetFilePointer(m_hFile, 0, NULL, FILE_BEGIN);
		}

		return nBytesWritten;
	}

	int64_t read_file(char* buffer, int64_t buffer_len) {
		if (!m_hFile) {
			m_hFile = CreateFile(m_file_name.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, NULL);
			if (!m_hFile) {
				fprintf(stdout, "fopen failed!!!\n");
				return -1;
			}
		}

		DWORD nBytesRead = 0;
		BOOL ret = ReadFile(m_hFile, buffer, buffer_len, &nBytesRead, NULL);
		if (FALSE == ret) {
			fprintf(stdout, "ReadFile failed!!!\n");
		}

		//SetFilePointer(m_hFile, 0, NULL, FILE_CURRENT);

		return nBytesRead;
	}

private:
	int64_t						m_max_file_size;
	HANDLE						m_hFile;

	std::map<int64_t, int64_t>   m_frame_record;//<frame_no,frame offset in file>
public:
	static std::string			m_file_name;
};


class CPostProcessor {
	typedef int(*SinkDataCallback)(unsigned char*, int, void*);
public:
	CPostProcessor():m_sink_data_cb(NULL), m_sink_data_cb_ctx(NULL){}
	~CPostProcessor(){}

	void set_sink_data_callback(SinkDataCallback _cb, void* context) {
		m_sink_data_cb = _cb;
		m_sink_data_cb_ctx = context;
	}

	BOOL create_zmq_context(int type, uint16_t port, const std::string& dst_ip, int32_t data_len) {
		//  Connect to task ventilator
		m_zmq_ctx= zmq_ctx_new();
		std::string addr("tcp://");

		if (ZMQ_PUB == type) {
			addr.append("*:" + std::to_string(port));
			m_zmq_sock = zmq_socket(m_zmq_ctx, ZMQ_PUB);
			int sndhwm = 2000;
			zmq_setsockopt(m_zmq_sock, ZMQ_SNDHWM, &sndhwm, sizeof sndhwm);
			zmq_bind(m_zmq_sock, addr.c_str());
		}
		else {//ZMQ_SUB
			addr.append(dst_ip + ":" + std::to_string(port));
			m_zmq_sock = zmq_socket(m_zmq_ctx, ZMQ_SUB);
			zmq_connect(m_zmq_sock, addr.c_str());
			zmq_setsockopt(m_zmq_sock, ZMQ_SUBSCRIBE, "", 0);
			//int recvdhwm = 5000;
			//zmq_setsockopt(data->socket, ZMQ_RCVHWM, &recvdhwm, sizeof recvdhwm);
		}

		m_snd_frame_rate = m_last_snd_no = m_recv_seq = m_last_recv_seq = m_frame_counter = 0L;
		std::vector<char> tmp(sizeof int64_t + sizeof int64_t + data_len, 0x00);
		m_buffer = tmp;

		return TRUE;
	}

	BOOL destroy_zmq_contex() {
		zmq_close(m_zmq_sock);
		zmq_ctx_destroy(m_zmq_ctx);
	}

	int32_t set_send_frame_rate(int32_t snd_frame_rate, int32_t full_frame_rate) {
		m_frame_gap = (full_frame_rate + 1) / snd_frame_rate;
		return m_snd_frame_rate = snd_frame_rate;
	}

	int32_t send_data(char* data, int data_len) {
		int ret = zmq_send(m_zmq_sock, m_buffer.data(), m_buffer.size(), 0);
		if (ret < 0) {
			int a = 0;
		}
		return ret;
	}

	int32_t recv_data() {
		int ret = zmq_recv(m_zmq_sock, (void*)m_buffer.data(), m_buffer.size(), 0);

		int64_t timestamp = 0;
		memcpy(&timestamp, m_buffer.data(), sizeof int64_t);
		memcpy(&m_recv_seq, &m_buffer[sizeof int64_t], sizeof int64_t);

		//printf("recv %d bytes, seq:%lld, timestamp:%lld\n", ret, ctx->seq, ctx->last_seq, timestamp);
		//if (!image) {
		//	image = cvCreateImageHeader(cvSize(1280, 720), 8, 1);
		//	pRGB = cvCreateImage(cvSize(1280, 720), 8, 3);
		//}
		//cvSetData(image, &ctx->buffer[sizeof int64_t + sizeof int64_t], 1280);
		//cvCvtColor(image, pRGB, CV_BayerRG2RGB);
		//cvShowImage("win", pRGB);
		//cvWaitKey(1);

		if (m_recv_seq != m_last_recv_seq + m_frame_gap) {
			printf("##########recv %d bytes, seq:%lld,last_seq:%lld,frame_gap:%lld, timestamp:%ll\n", ret, m_recv_seq, m_last_recv_seq, m_frame_gap, timestamp);
		}else {
			printf("recv %d bytes, seq:%lld,last_seq:%lld, timestamp:%lld\n", ret, m_recv_seq, m_last_recv_seq, timestamp);
		}
		m_last_recv_seq =m_recv_seq;

		if (m_sink_data_cb)
			m_sink_data_cb((unsigned char*)m_buffer.data(), m_buffer.size(), m_sink_data_cb_ctx);



		return m_buffer.size();
	}
	

public:
	void				*m_zmq_ctx;
	void				*m_zmq_sock;
	std::vector<char>	m_buffer;

	int64_t				m_frame_counter;

	int64_t				m_frame_gap;
	int64_t				m_snd_frame_rate;
	int64_t				m_last_snd_no;

	int64_t				m_recv_seq;
	int64_t				m_last_recv_seq;

	
	SinkDataCallback    m_sink_data_cb;
	void*			    m_sink_data_cb_ctx;

	//file operator
	BOOL				m_store_file_flag;
	CStoreFile			m_store_file;
};
#endif