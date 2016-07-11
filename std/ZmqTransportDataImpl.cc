#include "ZmqTransportDataImpl.h"
#include <string>

#include <opencv2/opencv.hpp>
#include "utils.h"



int SinkBayerDatasCallbackImpl(unsigned char* buffer, int buffer_len, void* context) {
	CPostProcessor* pProcessor = (CPostProcessor*)context;

	//get a frame per 5
	if (pProcessor->m_frame_counter != 0 && pProcessor->m_frame_gap != 0 && pProcessor->m_last_snd_no + pProcessor->m_frame_gap != pProcessor->m_frame_counter) {
		++pProcessor->m_frame_counter;
		return 0;
	}

	int64_t timestamp = get_current_time_in_ms();
	memcpy(&pProcessor->m_buffer[0], &timestamp, sizeof timestamp);
	memcpy(&pProcessor->m_buffer[sizeof int64_t], &pProcessor->m_frame_counter, sizeof int64_t);
	memcpy(&pProcessor->m_buffer[sizeof int64_t + sizeof int64_t], buffer, buffer_len);

	////store
	//if (ctx->store_file_flag) {
	//	DWORD nBytesWritten = 0;
	//	BOOL ret = WriteFile(ctx->hFile, ctx->buffer.data(), ctx->buffer.size(), &nBytesWritten, NULL);

	//	LARGE_INTEGER FileSize;
	//	BOOL r = GetFileSizeEx(ctx->hFile, &FileSize);
	//	if (!r) {
	//		printf("GetFileSizeEx failed\n");
	//		return FALSE;
	//	}

	//	const static int64_t MAX_RAW_VIDEO_FILE_SIZE = 1024 * 1024 * 1024 * 10;//10GB
	//	if (FileSize.QuadPart >= MAX_RAW_VIDEO_FILE_SIZE) {
	//		SetFilePointer(ctx->hFile, 0, NULL, FILE_BEGIN);
	//	}
	//}

	int ret = pProcessor->send_data((char*)buffer, buffer_len);
	if (ret < 0) {
		int a = 0;
	}


	if (pProcessor->m_store_file_flag) {

		pProcessor->m_store_file.write_file(pProcessor->m_buffer.data(), pProcessor->m_buffer.size());
	}

	return ret;
}

