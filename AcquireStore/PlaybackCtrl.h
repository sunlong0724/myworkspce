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
		std::vector<char> buffer(GET_IMAGE_BUFFER_SIZE(g_cs.m_image_w, g_cs.m_image_h), 0x00);
		while (true) {
			int64_t	now = get_current_time_in_ms();
			if (m_exited)
				break;

			if (Playback_NONE == m_status|| Playback_PAUSE == m_status) {
				sleep(2);
				continue;
			}else if (Playback_START_PLAY_FILE == m_status || Playback_START_PLAY_FILE_TEMP == m_status) {
				if (m_start_play_frame_no == -1) {
					sleep(2);//FIXME:
					continue;
				}
				m_status = Playback_PLAYING_FILE;
			}else if (Playback_PLAYING_FILE == m_status || Playback_PLAYING_FILE_TEMP == m_status) {

				if (g_cs.m_file_storage_object->read_file(buffer.data(), buffer.size(), m_start_play_frame_no) <= 0) {
					sleep(2);
					continue;
				}

				if (g_cs.m_processor_data_flag) {
					g_cs.m_post_processor_thread->writeImageData(buffer.data(), buffer.size());
				}
				else {
					processor_sink_data_cb((unsigned char*)buffer.data(), buffer.size(), &g_cs);
				}

				if (m_is_forward) {
					m_start_play_frame_no += g_cs.m_frame_gap;
				}
				else {
					m_start_play_frame_no -= g_cs.m_frame_gap;
				}
			}else if (Playback_START_PLAY_CAMERAS == m_status) {
				m_buffer.resize(GET_IMAGE_BUFFER_SIZE(g_cs.m_image_w, g_cs.m_image_h), 0x00);
				m_buffer_for_porcessor.resize(GET_IMAGE_BUFFER_SIZE(g_cs.m_image_w, g_cs.m_image_h), 0x00);
				m_ring_buffer = RingBuffer_create(GET_IMAGE_BUFFER_SIZE(g_cs.m_image_w, g_cs.m_image_h) * 10);

				g_cs.m_processor_data_flag = (g_cs.m_image_w != g_cs.m_play_frame_w || g_cs.m_image_h != g_cs.m_play_frame_h);
				if (g_cs.m_processor_data_flag) {
					g_cs.m_post_processor_thread->set_parameters(g_cs.m_image_w, g_cs.m_image_h, g_cs.m_play_frame_w, g_cs.m_play_frame_h);
					g_cs.m_post_processor_thread->set_sink_callback(processor_sink_data_cb, &g_cs);
					g_cs.m_post_processor_thread->start();
				}
				g_cs.m_camera->SetSinkBayerDataCallback(SinkBayerDatasCallbackImpl, &g_cs);
				if (!g_cs.m_camera->CreateOtherObjects()) {
					fprintf(stdout, "%s CreateOtherObjects failed!\n", __FUNCTION__);
				}

				g_cs.m_snd_frame_flag = TRUE;

				g_cs.m_camera->Start();
				m_status = Playback_PLAYING_CAMERAS;
			}else if (Playback_PLAYING_CAMERAS == m_status) {
				if (-1 == RingBuffer_read(m_ring_buffer, m_buffer_for_porcessor.data(), m_buffer_for_porcessor.size()))	{
					sleep(2);
					continue;
				}

				if (g_cs.m_processor_data_flag) {
					g_cs.m_post_processor_thread->writeImageData(m_buffer_for_porcessor.data(), m_buffer_for_porcessor.size());
				}
				else {
					processor_sink_data_cb((unsigned char*)m_buffer_for_porcessor.data(), m_buffer_for_porcessor.size(), &g_cs);
				}
			}else if (Playback_STOP_PLAY_CAMERAS == m_status) {
				g_cs.m_camera->Stop();
				g_cs.m_camera->DestroyOtherObjects();
				m_buffer.clear();
				m_buffer_for_porcessor.clear();
				RingBuffer_clear(m_ring_buffer);
				g_cs.m_post_processor_thread->stop();

				RingBuffer_destroy(m_ring_buffer);
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
	std::vector<char>	m_buffer;
	
	BOOL				m_is_forward;
	int64_t				m_start_play_frame_no;
	int64_t				m_play_how_many_frames;

	RingBuffer			*m_ring_buffer;
	CtrlStatus			m_status;
};


inline int processor_sink_data_cb(unsigned char* data, int data_len, void* ctx) {
	CustomStruct* p = (CustomStruct*)ctx;
	return p->m_snd_data_thread->send((char*)data, data_len);
}

#endif // !__PLAYBACK_CTRL_H__

