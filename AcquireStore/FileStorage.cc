
#include "defs.h"
#include "utils.h"
#include "MyThread.h"
#include "FileStorage.h"
#include <mutex>

CFileStorage::CFileStorage() : m_writter_handle(NULL), m_reader_handle(NULL), m_written_bytes(0), m_has_reach_max_file_size(FALSE) {
}

CFileStorage::~CFileStorage() {
	CloseHandle(m_writter_handle);
	CloseHandle(m_reader_handle);
}

void CFileStorage::set_file_name(const std::string& file_name, const int64_t max_raw_file_size) {
	m_file_name.append(file_name);
	m_max_file_size = max_raw_file_size;
}

int64_t CFileStorage::write_file(char* buffer, int64_t buffer_len) {
	int64_t ret = -1;
	int64_t timestamp = 0;
	int64_t frame_counter = 0;
	int64_t nWillWrittenPos = -1;
	memcpy(&frame_counter, &buffer[FRAME_SEQ_START], sizeof int64_t);
	memcpy(&timestamp, &buffer[FRAME_TIMESTAMP_START], sizeof int64_t);



	if (timestamp == 0x00) {
		fprintf(stdout, "%s Insert a lost frame %d\n", __FUNCTION__, frame_counter);
		nWillWrittenPos = -1;
		ret = 0;
	}
	else {
		if (m_written_queue == NULL) {
			return -1;
		}
		ret = RingBuffer_write(m_written_queue, buffer, buffer_len);
		if (-1 == ret) {
			fprintf(stdout, "%s RingBuffer full,insert a fake frame %d\n", __FUNCTION__, frame_counter);
		}
		nWillWrittenPos = -2;
	}

	///////////////////////////////////////////////////////////TESTTEST
	{
		std::lock_guard<std::mutex> lg(m_map_lock);
	
		//FIXME:
		m_frame_offset_map.insert(std::make_pair(frame_counter, nWillWrittenPos));
		m_frame_seq_timestamp_map_for_read.insert(std::make_pair(frame_counter, timestamp));
		m_frame_timestamp_seq_map.insert(std::make_pair(timestamp, frame_counter));

		if (m_has_reach_max_file_size) {
			//fprintf(stdout, "%s delete %d\n", __FUNCTION__, m_frame_offset_map.begin()->first);
			m_frame_offset_map.erase(m_frame_offset_map.begin());
			m_frame_seq_timestamp_map_for_read.erase(m_frame_seq_timestamp_map_for_read.begin());

			m_frame_timestamp_seq_map.erase(m_frame_timestamp_seq_map.begin());
		}
	}
	return ret;
}


void CFileStorage::run() {
	m_written_queue = RingBuffer_create(GET_IMAGE_BUFFER_SIZE(g_cs.m_image_w, g_cs.m_image_h) ,CAMERA_MAX_FPS * MAX_CACHE_IMAGE_TIME);
	std::vector<char> m_swap_buffer(GET_IMAGE_BUFFER_SIZE(g_cs.m_image_w, g_cs.m_image_h), 0x00);

	DWORD dw = 0;
	if (m_writter_handle == NULL || m_writter_handle == INVALID_HANDLE_VALUE) {
		//check sector size
		m_writter_handle = CreateFile(m_file_name.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, NULL);//aligin??
		if (m_writter_handle == INVALID_HANDLE_VALUE) {
			dw = GetLastError();
			fprintf(stdout, "CreateFile failed, errno:%lu!!!\n", dw);
			exit(-1);
		}
	}

	while (!m_exited) {
		if (-1 == RingBuffer_read(m_written_queue, m_swap_buffer.data(), m_swap_buffer.size())) {
			sleep(1);
			continue;
		}
		int ret = do_write_file(m_swap_buffer.data(), m_swap_buffer.size());
		if (m_swap_buffer.size() != ret) {
			fprintf(stdout, "%s do_write_file failed! ret %d\n", __FUNCTION__, ret);
		}
	}

	RingBuffer_destroy(m_written_queue);
}

//timestamp(int64_t) frame_no(int64_t) buffer
int64_t  CFileStorage::do_write_file(char* buffer, int64_t buffer_len) {
	DWORD dw = 0;

	DWORD nBytesWritten = 0;
	int64_t timestamp = 0;
	int64_t frame_counter = 0;
	memcpy(&frame_counter, &buffer[FRAME_SEQ_START], sizeof int64_t);
	memcpy(&timestamp, &buffer[FRAME_TIMESTAMP_START], sizeof int64_t);

	BOOL ret = FALSE;
	ret = WriteFile(m_writter_handle, buffer + 16, buffer_len - 16, &nBytesWritten, NULL);
	if (FALSE == ret) {
		dw = GetLastError();
		fprintf(stdout, "WriteFile failed, errno:%lu, frame_counter:%llu!!!\n", dw, frame_counter);
		return -2;
	}

	{
		std::lock_guard<std::mutex> lg(m_map_lock);
		m_frame_offset_map[frame_counter] = m_written_bytes;
	}
	
	m_written_bytes += nBytesWritten;
	m_fps_counter.statistics(__FUNCTION__, FALSE);

	//printf("write_file %d bytes, seq:%lld\n", nBytesWritten, frame_counter);
	if (m_written_bytes >= m_max_file_size) {//FIXME: maybe has a bug
		m_has_reach_max_file_size = TRUE;
		SetFilePointer(m_writter_handle, 0, NULL, FILE_BEGIN);
		m_written_bytes = 0;
	}
	return buffer_len;
}

int64_t CFileStorage::read_file(char* buffer, int64_t buffer_len, int64_t frame_no) {
	DWORD dw = 0;
	if (INVALID_HANDLE_VALUE == m_reader_handle || NULL == m_reader_handle) {
		m_reader_handle = CreateFile(m_file_name.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, NULL);
		if (m_reader_handle == INVALID_HANDLE_VALUE) {
			dw = GetLastError();
			fprintf(stderr, "fopen failed, errno:%lu!!!\n", dw);
			return -1;
		}
	}
	
	int64_t frame_offset = 0;
	{
/////////////////////////////////////////////////////////////////////TESTTEST	
		std::lock_guard<std::mutex> lg(m_map_lock);
		std::map<int64_t, int64_t>::iterator found_it = m_frame_offset_map.find(frame_no);
		if (found_it  == m_frame_offset_map.end()) {//The frame was overwritten!!!
			fprintf(stderr, " %s Frame %lld was not found, m_frame_offset_map size(%lld)!!!\n", __FUNCTION__, frame_no, m_frame_offset_map.size());
			return -2;
		}
		else {
			frame_offset = found_it->second;
		}

	}

	if (frame_offset < 0) {//this frame is lost!!!
		fprintf(stderr, " %s Frame %lld was not found, m_frame_offset_map size(%lld),frame_offset(%lld)!!!\n", __FUNCTION__, frame_no, m_frame_offset_map.size(), frame_offset);
		return 0;
	}

	LARGE_INTEGER offset;
	offset.QuadPart = frame_offset;
	SetFilePointerEx(m_reader_handle, offset, NULL, FILE_BEGIN);

	DWORD nBytesRead = 0;
	BOOL ret = ReadFile(m_reader_handle, buffer + 16, buffer_len - 16, &nBytesRead, NULL);
	if (FALSE == ret) {
		dw = GetLastError();
		fprintf(stdout, "ReadFile failed, errno:%lu!!!\n", dw);
	}

	{
		std::lock_guard<std::mutex> lg(m_map_lock);
		memcpy(&buffer[FRAME_TIMESTAMP_START], &m_frame_seq_timestamp_map_for_read[frame_no], sizeof int64_t);
		memcpy(&buffer[FRAME_SEQ_START], &frame_no, sizeof frame_no);
	}

	return nBytesRead;
}



int64_t CFileStorage::read_file_by_timestamp(char* buffer, int64_t buffer_len, int64_t timestamp) {

	int64_t seq = 0;
	{
		std::lock_guard<std::mutex> lg(m_map_lock);
	
		std::map<int64_t, int64_t>::iterator found_it = m_frame_timestamp_seq_map.find(timestamp);
		if (found_it == m_frame_timestamp_seq_map.end()) {//The frame was overwritten!!!
			fprintf(stderr, "%s Frame %lld was not found,m_frame_timestamp_seq_map size(%lld)!!!!!!\n", __FUNCTION__, timestamp, m_frame_timestamp_seq_map.size());
			return -2;
		}
		else {
			seq = found_it->second;
		}
	}

	return read_file(buffer, buffer_len, seq);
}

