#ifndef __SEND_DATA_IMPL_H__
#define __SEND_DATA_IMPL_H__

#include "utils.h"

class CFileStorage {

public:
	CFileStorage() : m_writter_handle(NULL),m_reader_handle(NULL), m_written_bytes(0), m_has_reach_max_file_size(FALSE){
	}

	~CFileStorage() {
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


#endif