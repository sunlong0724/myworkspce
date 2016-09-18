#ifndef __SEND_DATA_IMPL_H__
#define __SEND_DATA_IMPL_H__

#include "MyThread.h"
#include <mutex>
extern CustomStruct g_cs;

class CFileStorage : public CMyThread{
public:
	CFileStorage();
	~CFileStorage();
	static void set_file_name(const std::string& file_name, const int64_t max_raw_file_size);
	int64_t  write_file(char* buffer, int64_t buffer_len);
	int64_t read_file(char* buffer, int64_t buffer_len, int64_t frame_no);

	int64_t read_file_by_timestamp(char* buffer, int64_t buffer_len, int64_t timestamp);
protected:
	void run();
private:
	//timestamp(int64_t) frame_no(int64_t) buffer
	int64_t  do_write_file(char* buffer, int64_t buffer_len);

private:

	HANDLE						m_reader_handle;
	HANDLE						m_writter_handle;

	
	uint64_t					m_written_bytes;
	BOOL						m_has_reach_max_file_size;
	uint64_t					m_frame_counter;

	std::mutex					m_map_lock;
public:
	static std::map<int64_t, int64_t>  m_frame_timestamp_seq_map;//<timestamp, frame_no>
	static std::map<int64_t, int64_t>   m_frame_offset_map;//<frame_no,frame offset in file>

	static std::map<int64_t, int64_t>  m_frame_seq_timestamp_map_for_read;////<frame_no,timestamp>

	static std::string			m_file_name;
	static long long			m_max_file_size;

	RingBuffer					*m_written_queue;
	CFPSCounter					m_fps_counter;
};


#endif