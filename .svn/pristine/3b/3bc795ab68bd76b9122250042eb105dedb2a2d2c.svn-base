#include "SinkBayerDatasCallbackImpl.h"

#include "defs.h"
#include "utils.h"
#include "FileStorage.h"
#include "PlaybackCtrl.h"
#include <opencv2\opencv.hpp>

IplImage* g_bayer_image = NULL;


int SinkBayerDatasCallbackImpl(unsigned char* buffer, int buffer_len, int lost,  void* context) {
	CustomStruct* p = (CustomStruct*)context;

	//timestamp(int64_t) frame_no(int64_t) buffer
	int64_t timestamp = get_current_time_in_ms();
	++p->m_frame_counter;

	//write full frame rate
	if (p->m_store_file_flag) {
		int written = 0;
		memcpy(&p->m_playback_thread->m_buffer_for_camera_io[FRAME_SEQ_START], &p->m_frame_counter, sizeof int64_t);

		if (buffer != NULL) {
			memcpy(&p->m_playback_thread->m_buffer_for_camera_io[FRAME_TIMESTAMP_START], &timestamp, sizeof timestamp);
			memcpy(&p->m_playback_thread->m_buffer_for_camera_io[FRAME_DATA_START], buffer, buffer_len);

			written = p->m_file_storage_object_for_write_thread->write_file(p->m_playback_thread->m_buffer_for_camera_io.data(), GET_IMAGE_BUFFER_SIZE(g_cs.m_image_w, g_cs.m_image_h));
			//fprintf(stdout, "write_file success 1 , frame_counter:%lld, timestamp:%lld!!!\n", p->m_frame_counter,timestamp);
		}
		else {
			timestamp = 0x00;//0x00 This frame is dirty!!!
			memcpy(&p->m_playback_thread->m_buffer_for_camera_io[FRAME_TIMESTAMP_START], &timestamp, sizeof timestamp);

			written = p->m_file_storage_object_for_write_thread->write_file(p->m_playback_thread->m_buffer_for_camera_io.data(), GET_IMAGE_BUFFER_SIZE(g_cs.m_image_w, g_cs.m_image_h));
			//fprintf(stdout, "write_file success 2, frame_counter:%lld,timestamp:%lld!!!\n", p->m_frame_counter, timestamp);
		}

		if (written < 0) {
			fprintf(stdout, "p->m_file_storage_object_for_write_thread->write_file failed written %d\n", written);
		}

	}

	if (buffer == NULL) {
		p->m_last_live_play_seq++;
		return 0;
	}

	//check the data whether need!!!
	if (p->m_play_frame_gap != 0 && p->m_last_live_play_seq + p->m_play_frame_gap != p->m_frame_counter) {
		return 0;
	}
	//fprintf(stdout, "####m_last_live_play_seq %d, m_frame_counter %d, m_play_frame_gap %d\n", p->m_last_live_play_seq, p->m_frame_counter, p->m_play_frame_gap);
	g_cs.m_soft_grab_counter.statistics(__FUNCTION__, FALSE);
	if (p->m_snd_live_frame_flag) {
		int ret =  p->m_playback_thread->writeImageData(p->m_playback_thread->m_buffer_for_camera_io.data(), GET_IMAGE_BUFFER_SIZE(g_cs.m_image_w, g_cs.m_image_h));
		g_cs.m_last_live_play_seq += g_cs.m_play_frame_gap;
		return ret;
	}

	
	return 0;
}

