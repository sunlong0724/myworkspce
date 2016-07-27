#include "ZmqTransportDataImpl.h"
#include <string>

#include <opencv2/opencv.hpp>


int SinkBayerDatasCallbackImpl(unsigned char* buffer, int buffer_len, void* context) {
	CPostProcessor* pProcessor = (CPostProcessor*)context;

	//timestamp(int64_t) frame_no(int64_t) buffer
	int64_t timestamp = get_current_time_in_ms();
	++pProcessor->m_frame_counter;
	memcpy(&pProcessor->m_buffer[FRAME_TIMESTAMP_START], &timestamp, sizeof timestamp);
	memcpy(&pProcessor->m_buffer[FRAME_SEQ_START], &pProcessor->m_frame_counter, sizeof int64_t);
	memcpy(&pProcessor->m_buffer[FRAME_DATA_START], buffer, buffer_len);

	//write full frame rate
	if (pProcessor->m_store_file_flag) {
		pProcessor->m_store_file.write_file(pProcessor->m_buffer.data(), pProcessor->m_buffer.size());
	}

	//check wether scale
	if (pProcessor->m_snd_frame_flag) {
		if (pProcessor->m_image_w != pProcessor->m_snd_frame_w || pProcessor->m_image_h != pProcessor->m_snd_frame_h) {
			//FIXME: scale the image bayer-->rgb--->scale--->bayer

			if (!pProcessor->m_bayer_image) {
				pProcessor->m_bayer_image = cvCreateImageHeader(cvSize(pProcessor->m_image_w, pProcessor->m_image_h), 8, 1);
			}
			if (!pProcessor->m_rgb_image) {
				pProcessor->m_rgb_image = cvCreateImage(cvSize(pProcessor->m_image_w, pProcessor->m_image_h), 8, 3);
			}

			if (!pProcessor->m_rgb_image_resized) {
				pProcessor->m_rgb_image_resized = cvCreateImage(cvSize(pProcessor->m_snd_frame_w, pProcessor->m_snd_frame_h), 8, 3);
			}

			cvSetData(pProcessor->m_bayer_image, &pProcessor->m_buffer[sizeof int64_t + sizeof int64_t], pProcessor->m_image_w);
			cvCvtColor(pProcessor->m_bayer_image, pProcessor->m_rgb_image, CV_BayerRG2RGB);

			cvResize(pProcessor->m_rgb_image, pProcessor->m_rgb_image_resized);

			cvShowImage("win", pProcessor->m_rgb_image_resized);
			cvWaitKey(1);
		}

		int ret = pProcessor->send_data((char*)pProcessor->m_buffer.data(), pProcessor->m_buffer.size());
		if (ret < 0) {
			int a = 0;
		}
		return ret;
	}
	return 0;
}

