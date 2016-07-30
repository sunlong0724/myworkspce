#include "SinkBayerDatasCallbackImpl.h"

#include "defs.h"
#include "utils.h"
#include "FileStorage.h"
#include "PlaybackCtrl.h"

int SinkBayerDatasCallbackImpl(unsigned char* buffer, int buffer_len, void* context) {
	CustomStruct* p = (CustomStruct*)context;

	//timestamp(int64_t) frame_no(int64_t) buffer
	int64_t timestamp = get_current_time_in_ms();
	++p->m_frame_counter;

	memcpy(&p->m_playback_thread->m_buffer_for_camera_io[FRAME_TIMESTAMP_START], &timestamp, sizeof timestamp);
	memcpy(&p->m_playback_thread->m_buffer_for_camera_io[FRAME_SEQ_START], &p->m_frame_counter, sizeof int64_t);
	memcpy(&p->m_playback_thread->m_buffer_for_camera_io[FRAME_DATA_START], buffer, buffer_len);

	//write full frame rate
	if (p->m_store_file_flag) {
		p->m_file_storage_object_for_write->write_file(p->m_playback_thread->m_buffer_for_camera_io.data(), p->m_playback_thread->m_buffer_for_camera_io.size());
	}

	//check the data whether need!!!
	if (p->m_play_frame_gap != 0 && p->m_last_live_play_seq + p->m_play_frame_gap != p->m_frame_counter) {
		return 0;
	}

	if (p->m_snd_live_frame_flag) {
		int ret =  p->m_playback_thread->writeImageData(p->m_playback_thread->m_buffer_for_camera_io.data(), p->m_playback_thread->m_buffer_for_camera_io.size());
		g_cs.m_last_live_play_seq += g_cs.m_play_frame_gap;
		return ret;
	}
	return 0;
}

