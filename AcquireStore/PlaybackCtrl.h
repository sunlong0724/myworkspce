#ifndef __PLAYBACK_CTRL_H__
#define __PLAYBACK_CTRL_H__

#include "utils.h"
#include "FileStorage.h"
#include "TransportData.h"
#include "PostProcessor.h"
#include "MyThread.h"
#include "Camera.h"
#include "SinkBayerDatasCallbackImpl.h"

extern CustomStruct g_cs;
extern int processor_sink_data_cb(unsigned char* data, int data_len, void* ctx);

class CPlaybackCtrlThread : public CMyThread {
public:
	CPlaybackCtrlThread() : m_start_play_frame_no(-1),  m_status(Playback_NONE){

	}
	~CPlaybackCtrlThread() {}

	int64_t writeImageData(char* buffer, int len) {
		return RingBuffer_write(m_ring_buffer, buffer, len);
	}

	void run() {
		int64_t frame_no = 0;
		int k = 0;
		std::vector<char> buffer_for_file_io(GET_IMAGE_BUFFER_SIZE(g_cs.m_image_w, g_cs.m_image_h), 0x00);
		while (true) {
			int64_t	now = get_current_time_in_ms();
			if (m_exited)
				break;

			if (Playback_NONE == m_status|| Playback_PAUSE == m_status) {
				sleep(2);
				continue;
			}else if (Playback_PLAYING_FILE == m_status || Playback_PLAYING_FILE_TEMP == m_status) {
				if (m_playback_how_many_frames > 0) {
					if (m_has_playback_how_many_frames >= m_playback_how_many_frames) {
						sleep(2);
						continue;
					}
				}
				else if (m_playback_how_many_frames == 0) {//unlimitied

				}

				if (g_cs.m_file_storage_object_for_read->read_file(buffer_for_file_io.data(), buffer_for_file_io.size(), m_start_play_frame_no) <= 0) {
					m_status = Playback_NONE;
					continue;
				}

				if (g_cs.m_processor_data_flag) {
					g_cs.m_post_processor_thread->writeImageData(buffer_for_file_io.data(), buffer_for_file_io.size());
				}
				else {
					processor_sink_data_cb((unsigned char*)buffer_for_file_io.data(), buffer_for_file_io.size(), &g_cs);
				}

				++m_has_playback_how_many_frames;

				if (Playback_PLAYING_FILE_TEMP == m_status) {
					if (m_is_forward) {
						m_start_play_frame_no += g_cs.m_play_frame_gap_temp;
					}
					else {
						m_start_play_frame_no -= g_cs.m_play_frame_gap_temp;
					}
				}
				else {
					if (m_is_forward) {
						m_start_play_frame_no += g_cs.m_play_frame_gap;
					}
					else {
						m_start_play_frame_no -= g_cs.m_play_frame_gap;
					}
				}
			}
			else if (Playback_PLAYING_CAMERAS == m_status) {
				if (-1 == RingBuffer_read(m_ring_buffer, m_buffer_for_porcessor.data(), m_buffer_for_porcessor.size())) {
					sleep(2);
					continue;
				}

				if (g_cs.m_processor_data_flag) {
					g_cs.m_post_processor_thread->writeImageData(m_buffer_for_porcessor.data(), m_buffer_for_porcessor.size());
				}
				else {
					processor_sink_data_cb((unsigned char*)m_buffer_for_porcessor.data(), m_buffer_for_porcessor.size(), &g_cs);
				}
			}
			int64_t now2 = get_current_time_in_ms();
			int64_t	time_gap = 1000 / g_cs.m_play_frame_rate;
			int64_t t_gap = now2 - now;

			if (time_gap > t_gap) {
				//fprintf(stdout, "%s sleep %lld\n", __FUNCTION__, time_gap - t_gap);
				sleep(time_gap - t_gap);
			}
		}
	}


public:
	std::vector<char>	m_buffer_for_porcessor;
	std::vector<char>	m_buffer_for_camera_io;
	
	BOOL				m_is_forward;
	int64_t				m_start_play_frame_no;
	int64_t				m_playback_how_many_frames;
	int64_t				m_has_playback_how_many_frames;

	RingBuffer			*m_ring_buffer;
	CtrlStatus			m_status;
};


inline int processor_sink_data_cb(unsigned char* data, int data_len, void* ctx) {
	CustomStruct* p = (CustomStruct*)ctx;
	return p->m_snd_data_thread->send((char*)data, data_len);
}

#endif // !__PLAYBACK_CTRL_H__

