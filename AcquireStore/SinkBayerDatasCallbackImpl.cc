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


	if (buffer == NULL) {
		g_cs.m_soft_grab_counter.statistics(__FUNCTION__, FALSE, FALSE);
		timestamp = 0x00;
	}
	else {
		g_cs.m_soft_grab_counter.statistics(__FUNCTION__, FALSE);
		memcpy(&p->m_playback_thread->m_buffer_for_camera_io[FRAME_DATA_START], buffer, buffer_len);
	}
	memcpy(&p->m_playback_thread->m_buffer_for_camera_io[FRAME_SEQ_START], &p->m_frame_counter, sizeof int64_t);
	memcpy(&p->m_playback_thread->m_buffer_for_camera_io[FRAME_TIMESTAMP_START], &timestamp, sizeof timestamp);

	//write full frame rate
	if (p->m_store_file_flag) {
		int	written = p->m_file_storage_object_for_write_thread->write_file(p->m_playback_thread->m_buffer_for_camera_io.data(), GET_IMAGE_BUFFER_SIZE(g_cs.m_image_w, g_cs.m_image_h));
		if (written < 0) {
			fprintf(stdout, "p->m_file_storage_object_for_write_thread->write_file failed written %d\n", written);
		}

	}

	if (buffer == NULL) {
		p->m_last_live_play_seq++;
		return 0;
	}

	//fprintf(stdout, "1####m_last_live_play_seq %d, m_frame_counter %d, m_play_frame_gap %d m_snd_live_frame_flag %d\n", p->m_last_live_play_seq, p->m_frame_counter, p->m_play_frame_gap, p->m_snd_live_frame_flag);
	//check the data whether need!!!
	if (p->m_play_frame_gap != 0 && 
		(p->m_last_live_play_seq + p->m_play_frame_gap > p->m_frame_counter && p->m_last_live_play_seq != 0)) {
		return 0;
	}
	//fprintf(stdout, "2####m_last_live_play_seq %d, m_frame_counter %d, m_play_frame_gap %d m_snd_live_frame_flag %d\n", p->m_last_live_play_seq, p->m_frame_counter, p->m_play_frame_gap, p->m_snd_live_frame_flag);

	if (p->m_snd_live_frame_flag) {
		int ret =  p->m_playback_thread->writeImageData(p->m_playback_thread->m_buffer_for_camera_io.data(), GET_IMAGE_BUFFER_SIZE(g_cs.m_image_w, g_cs.m_image_h));
		g_cs.m_last_live_play_seq = p->m_frame_counter;
		return ret;
	}

	
	return 0;
}

