#ifndef __SEND_DATA_IMPL_H__
#define __SEND_DATA_IMPL_H__

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <thread>
#include "zmq.h"
#include <opencv2\opencv.hpp>
extern IplImage* image;

#define ONE_GB 1024*1024*1024

#define FRAME_TIMESTAMP_START  0
#define FRAME_SEQ_START   sizeof int64_t
#define FRAME_DATA_START sizeof int64_t+sizeof int64_t


#define CAMERA_MAX_FPS 180


int SinkBayerDatasCallbackImpl(unsigned char* buffer, int buffer_len, void* ctx);

inline int64_t get_current_time_in_ms() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

inline void sleep(int32_t milliseconds) {
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

class CStoreFile {

public:
	CStoreFile() : m_writter_handle(NULL),m_reader_handle(NULL), m_written_bytes(0), m_has_reach_max_file_size(FALSE){
	}

	~CStoreFile() {
		CloseHandle(m_writter_handle);
		CloseHandle(m_reader_handle);
	}

	static void set_file_name(const std::string& file_name, const int64_t max_raw_file_size) {
		m_file_name.append(file_name);
		m_max_file_size = max_raw_file_size;
	}

	//timestamp(int64_t) frame_no(int64_t) buffer
	int64_t  write_file(char* buffer, int64_t buffer_len) {
		DWORD dw = 0;
		if (m_writter_handle == NULL|| m_writter_handle == INVALID_HANDLE_VALUE) {
			//check sector size
			//FILE_FLAG_NO_BUFFERING check aligin
			m_writter_handle = CreateFile(m_file_name.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE|FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL/* | FILE_FLAG_NO_BUFFERING*/, NULL);//aligin??
			if (m_writter_handle == INVALID_HANDLE_VALUE) {
				dw = GetLastError();
				fprintf(stdout, "CreateFile failed, errno:%lu!!!\n", dw);
				return -1;
			}
			else {
				//FIXME:
				//STORAGE_PROPERTY_QUERY q = { StorageDeviceProperty ,PropertyStandardQuery };
				//DWORD BytesReturned = 0;
				//BOOL ret =  DeviceIoControl(m_hFile,				// handle to a partition
				//		IOCTL_STORAGE_QUERY_PROPERTY,				// dwIoControlCode
				//		&q,											// input buffer - STORAGE_PROPERTY_QUERY structure
				//		sizeof q,									// size of input buffer
				//		NULL,										// output buffer - see Remarks
				//		0,											// size of output buffer
				//		&BytesReturned,								// number of bytes returned
				//		NULL);										// OVERLAPPED structure
				//if (ret == FALSE) {
				//	dw = GetLastError();
				//	fprintf(stdout, "DeviceIoControl failed, errno:%lu!!!\n", dw);
				//}
			}
		}

		DWORD nBytesWritten = 0;

		//FIXEM:
		//BOOL ret = WriteFile(m_writter_handle, &buffer[sizeof int64_t + sizeof int64_t], buffer_len - sizeof int64_t - sizeof int64_t, &nBytesWritten, NULL);
		BOOL ret = WriteFile(m_writter_handle, buffer, buffer_len, &nBytesWritten, NULL);
		if (FALSE == ret) {
			dw = GetLastError();
			fprintf(stdout, "WriteFile failed, errno:%lu!!!\n", dw);
		}

		
		int64_t frame_counter = 0;
		memcpy(&frame_counter, &buffer[FRAME_SEQ_START], sizeof int64_t);
		m_frame_offset_map.insert(std::make_pair(frame_counter, m_written_bytes));
		m_written_bytes += nBytesWritten;

		//printf("write_file %d bytes, seq:%lld\n", nBytesWritten, frame_counter);

		if (m_written_bytes >= m_max_file_size) {//FIXME: maybe has a bug
			m_has_reach_max_file_size = TRUE;
			SetFilePointer(m_writter_handle, 0, NULL, FILE_BEGIN);
			m_written_bytes = 0;
		}

		if (m_has_reach_max_file_size) {
			m_frame_offset_map.erase(m_frame_offset_map.begin());
		}

		return nBytesWritten;
	}

	int64_t read_file(char* buffer, int64_t buffer_len, int64_t frame_no) {
		DWORD dw = 0;
		if (INVALID_HANDLE_VALUE == m_reader_handle || NULL == m_reader_handle) {
			m_reader_handle = CreateFile(m_file_name.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE|FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL /*| FILE_FLAG_NO_BUFFERING*/, NULL);
			if (m_reader_handle == INVALID_HANDLE_VALUE) {
				dw = GetLastError();
				fprintf(stderr, "fopen failed, errno:%lu!!!\n", dw);
				return -1;
			}
		}

		if (m_frame_offset_map.find(frame_no) == m_frame_offset_map.end()) {
			fprintf(stderr, "Frame %lld was not found!!!\r", frame_no);
			return -2;
		}
		else {
			int64_t	frame_offset = m_frame_offset_map[frame_no];
			//fprintf(stdout, "%s frame_no:%lld, frame_offset:%lld\n", __FUNCTION__,frame_no, frame_offset);
			LARGE_INTEGER offset;
			offset.QuadPart = frame_offset;
			SetFilePointerEx(m_reader_handle, offset, NULL, FILE_BEGIN);
		}
		DWORD nBytesRead = 0;
		//BOOL ret = ReadFile(m_reader_handle, &buffer[sizeof int64_t + sizeof int64_t], buffer_len - sizeof int64_t - sizeof int64_t, &nBytesRead, NULL);
		BOOL ret = ReadFile(m_reader_handle, buffer, buffer_len, &nBytesRead, NULL);
		if (FALSE == ret) {
			dw = GetLastError();
			fprintf(stdout, "ReadFile failed, errno:%lu!!!\n", dw);
		}


		//if (!image) {
		//	image = cvCreateImageHeader(cvSize(1280, 720), 8, 1);
		//	//pRGB = cvCreateImage(cvSize(1280, 720), 8, 3);
		//}
		//cvSetData(image, &buffer[sizeof int64_t + sizeof int64_t], 1280);
		////cvCvtColor(image, pRGB, CV_BayerRG2RGB);
		//cvShowImage("win", image);
		//cvWaitKey(1);

		return nBytesRead;
	}

private:

	HANDLE						m_reader_handle;
	HANDLE						m_writter_handle;

	std::map<int64_t, int64_t>   m_frame_offset_map;//<frame_no,frame offset in file>
	uint64_t					m_written_bytes;
	BOOL						m_has_reach_max_file_size;
	uint64_t					m_frame_counter;
public:
	static std::string			m_file_name;
	static long long			m_max_file_size;
};

class CPostProcessor {
	typedef int(*SinkDataCallback)(unsigned char*, int, void*);
public:
	CPostProcessor():m_play_extied(TRUE), m_snd_frame_w(0), m_snd_frame_h(0), m_start_play_frame_no(-1), m_snd_frame_flag(FALSE), m_bayer_image(NULL), m_rgb_image(NULL), m_rgb_image_resized(NULL){
	
	}
	~CPostProcessor(){}

	BOOL create_zmq_context(int type, uint16_t port, const std::string& dst_ip, int32_t image_w, int32_t image_h) {
		//  Connect to task ventilator
		m_zmq_ctx= zmq_ctx_new();
		std::string addr("tcp://");
		int high_water_mark = CAMERA_MAX_FPS * 10;

		if (ZMQ_PUB == type) {
			addr.append("*:" + std::to_string(port));
			m_zmq_sock = zmq_socket(m_zmq_ctx, ZMQ_PUB);
			zmq_setsockopt(m_zmq_sock, ZMQ_SNDHWM, &high_water_mark, sizeof high_water_mark);
			zmq_bind(m_zmq_sock, addr.c_str());
		}
		else {//ZMQ_SUB
			addr.append(dst_ip + ":" + std::to_string(port));
			m_zmq_sock = zmq_socket(m_zmq_ctx, ZMQ_SUB);
			zmq_connect(m_zmq_sock, addr.c_str());
			zmq_setsockopt(m_zmq_sock, ZMQ_SUBSCRIBE, "", 0);
			//zmq_setsockopt(m_zmq_sock, ZMQ_RCVHWM, &high_water_mark, sizeof high_water_mark);
		}

		m_image_w = image_w;
		m_image_h = image_h;

		m_snd_frame_rate = m_last_snd_no = m_recv_seq = m_last_recv_seq = m_frame_counter = 0L;
		std::vector<char> tmp(sizeof int64_t + sizeof int64_t + image_w * image_h, 0x00);
		m_buffer = tmp;

		return TRUE;
	}

	BOOL destroy_zmq_contex() {
		zmq_close(m_zmq_sock);
		zmq_ctx_destroy(m_zmq_ctx);
		return FALSE;
	}

	int32_t set_snd_frame_resolution(int32_t w, int32_t h) {
		m_snd_frame_w = w;
		m_snd_frame_h = h;
		return TRUE;
	}

	int32_t set_send_frame_rate(int32_t snd_frame_rate, int32_t full_frame_rate) {
		m_frame_gap = (full_frame_rate + 1) / snd_frame_rate;
		return m_snd_frame_rate = snd_frame_rate;
	}

	int32_t set_play_parameters(const int64_t frame_seq, BOOL is_forward) {
		m_start_play_frame_no = frame_seq;
		m_is_forward = is_forward;
		return 0;
	}

	int32_t send_data(char* data, int data_len) {
		int ret = zmq_send(m_zmq_sock, data,data_len,0);// ZMQ_DONTWAIT);
		if (ret < 0) {
			int a = 0;
		}

		int64_t timestamp;
		int64_t frame_no;
		memcpy(&timestamp, &data[FRAME_TIMESTAMP_START], sizeof int64_t);
		memcpy(&frame_no, &data[FRAME_SEQ_START], sizeof int64_t);

		//FIXME: #################################################################
		printf("snd %d bytes, seq:%lld, sndno:%lld,timestamp:%lld\r", ret, frame_no, frame_no, timestamp);

		m_last_snd_no = frame_no;
		return ret;
	}

	int32_t recv_data(char* buffer, int32_t buffer_size) {
		int ret = zmq_recv(m_zmq_sock, (void*)m_buffer.data(), m_buffer.size(), 0);// ZMQ_DONTWAIT);
		if (ret < 0) {
			int a = 0;
		}

		if (buffer != NULL && buffer_size >= m_buffer.size()) {
			memcpy(buffer, m_buffer.data(), m_buffer.size());
		}

		int64_t timestamp = 0;
		memcpy(&timestamp, &buffer[FRAME_TIMESTAMP_START], sizeof int64_t);
		memcpy(&m_recv_seq, &buffer[FRAME_SEQ_START], sizeof int64_t);

		//printf("recv %d bytes, seq:%lld, timestamp:%lld\n", ret, ctx->seq, ctx->last_seq, timestamp);
		//if (!image) {
		//	image = cvCreateImageHeader(cvSize(1280, 720), 8, 1);
		//	//pRGB = cvCreateImage(cvSize(1280, 720), 8, 3);
		//}
		//cvSetData(image, &m_buffer[sizeof int64_t + sizeof int64_t], 1280);
		////cvCvtColor(image, pRGB, CV_BayerRG2RGB);
		//cvShowImage("win", image);
		//cvWaitKey(1);



		if ( std::abs(m_recv_seq - m_last_recv_seq) != m_frame_gap) {
			printf("#recv %d bytes,seq:%lld,last_seq:%lld,frame_gap:%lld,timestamp:%lld\n", ret, m_recv_seq, m_last_recv_seq, m_frame_gap, timestamp);
		}else {
			printf("recv %d bytes,seq:%lld,last_seq:%lld,timestamp:%lld\r", ret, m_recv_seq, m_last_recv_seq, timestamp);
		}
		m_last_recv_seq =m_recv_seq;
		return m_buffer.size();
	}

	void play_run() {
		int64_t frame_no = 0;
		int k = 0;

		std::vector<char> buffer(sizeof int64_t + sizeof int64_t + m_image_h*m_image_w, 0x00);
		while (true) {
			int64_t	now = get_current_time_in_ms();
			if (m_play_extied)
				break;

			if (m_start_play_frame_no == -1) {
				Sleep(2);//FIXME:
				continue;
			}

			//fprintf(stdout, "%s frame no:%lld\n", __FUNCTION__, m_start_play_frame_no);
			if (m_store_file.read_file(buffer.data(), buffer.size(), m_start_play_frame_no) <= 0) {
				Sleep(2);
				continue;
			}

			if (m_image_h != m_snd_frame_h || m_image_w != m_snd_frame_w) {
				//check wether scale;
			}
		
			send_data(buffer.data(), buffer.size());
			if (m_is_forward) {
				m_start_play_frame_no += m_frame_gap;
			}
			else {
				m_start_play_frame_no -= m_frame_gap;
			}

			int64_t now2 = get_current_time_in_ms();
			int64_t	time_gap = 1000 / m_snd_frame_rate;
			int64_t t_gap = now2 - now;

			if (time_gap > t_gap) {
				//fprintf(stdout, "%s sleep %lld\n", __FUNCTION__, time_gap - t_gap);
				sleep( time_gap - t_gap);
			}
		}
	}


	

public:
	void				*m_zmq_ctx;
	void				*m_zmq_sock;
	std::vector<char>	m_buffer;

	int64_t				m_frame_counter;

	int32_t				m_image_w;
	int32_t				m_image_h;

	int64_t				m_frame_gap;
	int64_t				m_snd_frame_rate;
	int64_t				m_last_snd_no;

	int32_t				m_snd_frame_w;
	int32_t				m_snd_frame_h;

	int64_t				m_recv_seq;
	int64_t				m_last_recv_seq;

	BOOL				m_is_forward;
	int64_t				m_start_play_frame_no;

	std::thread			m_play_thread;
	BOOL				m_play_extied;
	
	//file operator
	BOOL				m_store_file_flag;
	CStoreFile			m_store_file;

	BOOL				m_snd_frame_flag;

	IplImage*			m_bayer_image;
	IplImage*			m_rgb_image;
	IplImage*			m_rgb_image_resized;

};
#endif