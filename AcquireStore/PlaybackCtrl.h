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
		PRINT_FRAME_INFO(buffer);
		return RingBuffer_write(m_ring_buffer, buffer, len);
	}

	void calc_play_frame_no(int gap) {
		if (Playback_PLAYING_FILE_TEMP == m_status) {
			if (m_is_forward) {
				m_start_play_frame_no += gap;
			}
			else {
				m_start_play_frame_no -= gap;
			}
		}
		else {
			if (m_is_forward) {
				m_start_play_frame_no += gap;
			}
			else {
				m_start_play_frame_no -= gap;
			}
		}
	}

	void run() {
		timeBeginPeriod(1);
		int64_t frame_no = 0;
		int k = 0;
		std::vector<char> buffer_io(GET_IMAGE_BUFFER_SIZE(g_cs.m_image_w, g_cs.m_image_h), 0x00);

		g_bayer= NULL;
		while (true) {
			int64_t	now = get_current_time_in_ms();
			if (m_exited)
				break;

			if (Playback_NONE == m_status|| Playback_PAUSE == m_status) {
				sleep(1);
				continue;
			}else if (Playback_PLAYING_FILE_TEMP == m_status) {
				if (g_cs.m_how_many_frames_flag) {
					if (g_cs.m_how_many_frames  > 0) {
						g_cs.m_how_many_frames--;
					}
					else {
						g_cs.m_how_many_frames_flag = FALSE;
						m_status = Playback_NONE;
						continue;
					}
				}

				int nByteRead = g_cs.m_file_storage_object_for_read->read_file(buffer_io.data(), buffer_io.size(), m_start_play_frame_no);
				if (nByteRead < 0) {
					if (Playback_PLAYING_FILE_TEMP == m_status) {
						if (m_is_forward) {
							m_start_play_frame_no += g_cs.m_play_frame_gap_temp;
						}
						else {
							m_status = Playback_NONE;
						}
					}
					continue;
				}
				else if (nByteRead == 0) {
					//this frame is lost!!!
					calc_play_frame_no(1);
					continue;
				}
				else {
					if (g_cs.m_processor_data_flag) {
						g_cs.m_post_processor_thread->writeImageData(buffer_io.data(), buffer_io.size());
					}
					else {
						processor_sink_data_cb((unsigned char*)buffer_io.data(), buffer_io.size(), &g_cs);
					}
					calc_play_frame_no(g_cs.m_play_frame_gap_temp);
				}

				int64_t	now2 = get_current_time_in_ms();
				double t = 1000.0f / ((double)g_cs.m_play_frame_rate+2.0f);
				if (now2 - now < t) {
					//fprintf(stdout, " sleep:%f", t - (double)(now2 - now));
					sleep(t - (double)(now2 - now));
				}
			}
			else if (Playback_PLAYING_CAMERAS == m_status) {
			
				if (-1 == RingBuffer_read(m_ring_buffer, buffer_io.data(), buffer_io.size())) {
					sleep(1);
					continue;
				}
#if 0
				if (!g_bayer) {
					g_bayer = cvCreateImageHeader(cvSize(g_cs.m_image_w, g_cs.m_image_h), 8, 1);
					cvSetData(g_bayer, &buffer_io[FRAME_DATA_START], g_cs.m_image_w);
				}
				cvShowImage("win_playback_ctrl", g_bayer);
#endif

				if (g_cs.m_processor_data_flag) {
					PRINT_FRAME_INFO(buffer_io.data());
					g_cs.m_post_processor_thread->writeImageData(buffer_io.data(), buffer_io.size());
				}
				else {
					processor_sink_data_cb((unsigned char*)buffer_io.data(), buffer_io.size(), &g_cs);
				}
			}
		}
		timeEndPeriod(1);
	}


public:
	std::vector<char>	m_buffer_for_camera_io;

	BOOL				m_is_forward;
	int64_t				m_start_play_frame_no;
	//int64_t				m_playback_how_many_frames;
	//int64_t				m_has_playback_how_many_frames;

	RingBuffer			*m_ring_buffer;
	CtrlStatus			m_status;

	IplImage*			g_bayer  ;
};


inline int processor_sink_data_cb(unsigned char* data, int data_len, void* ctx) {
	CustomStruct* p = (CustomStruct*)ctx;
	//fprintf(stdout, "\n%s\n", __FUNCTION__);
	return p->m_snd_data_thread->send((char*)data, data_len);
}

#endif // !__PLAYBACK_CTRL_H__

