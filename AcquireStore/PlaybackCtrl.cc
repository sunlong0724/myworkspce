#include "utils.h"
#include "FileStorage.h"
#include "TransportData.h"
#include "PostProcessor.h"
#include "MyThread.h"
#include "Camera.h"
#include "SinkBayerDatasCallbackImpl.h"

#include "PlaybackCtrl.h"

extern CustomStruct g_cs;
extern int processor_sink_data_cb(unsigned char* data, int data_len, void* ctx);

CPlaybackCtrlThread::CPlaybackCtrlThread() : m_start_play_frame_no(-1), m_status(Pb_STATUS_NONE){

}
CPlaybackCtrlThread::~CPlaybackCtrlThread() {}

int64_t CPlaybackCtrlThread::writeImageData(char* buffer, int len) {
	PRINT_FRAME_INFO(buffer);
	return  RingBuffer_write(m_ring_buffer, buffer, len);
}

void CPlaybackCtrlThread::calc_play_frame_no(int gap) {
	if (Pb_STATUS_PLAY_FORWARD ==  m_status) {
		m_start_play_frame_no += gap;
	}
	else {
		m_start_play_frame_no -= gap;
	}
}

void CPlaybackCtrlThread::run() {
	timeBeginPeriod(1);
	int64_t frame_no = 0;
	int k = 0;
	std::vector<char> buffer_io(GET_IMAGE_BUFFER_SIZE(g_cs.m_image_w, g_cs.m_image_h), 0x00);
	//m_playback_thread init
	m_buffer_for_camera_io.resize(GET_IMAGE_BUFFER_SIZE(g_cs.m_image_w, g_cs.m_image_h), 0x00);
	m_ring_buffer = RingBuffer_create(GET_IMAGE_BUFFER_SIZE(g_cs.m_image_w, g_cs.m_image_h) * 1);

	while (true) {
		int64_t	now = get_current_time_in_ms();
		if (m_exited)
			break;

		if (Pb_STATUS_NONE == m_status || Pb_STATUS_PLAY_PAUSE == m_status) {
			sleep(1);
			continue;
		}
		else if (Pb_STATUS_PLAY_FORWARD == m_status || Pb_STATUS_PLAY_BACKWARD == m_status ) {
			int nByteRead = g_cs.m_file_storage_object_for_read->read_file(buffer_io.data(), buffer_io.size(), m_start_play_frame_no);
			if (nByteRead < 0) {
				//this frame was overwrittern!!!
				if (Pb_STATUS_PLAY_FORWARD == m_status) {
					if (m_start_play_frame_no + g_cs.m_play_frame_gap < m_start_play_frame_no_begin) {
						m_start_play_frame_no += g_cs.m_play_frame_gap;
					}
					else {
						m_status = Pb_STATUS_PLAY_PAUSE;
						fprintf(stdout, "%s 1 m_status %d  m_start_play_frame_no %d\n", __FUNCTION__, m_status, m_start_play_frame_no);
					}
				}
				else {
					m_status = Pb_STATUS_PLAY_PAUSE;
					fprintf(stdout, "%s 2 m_status %d  m_start_play_frame_no %d\n", __FUNCTION__, m_status, m_start_play_frame_no);
				}
				continue;
			}
			else if (nByteRead == 0) {
				//this frame is lost!!!
				calc_play_frame_no(1);
				fprintf(stdout, "%s read_file nByteRead=0 m_start_play_frame_no %d\n", __FUNCTION__, m_start_play_frame_no);
				continue;
			}
			else {
				if (g_cs.m_processor_data_flag) {
					g_cs.m_post_processor_thread->writeImageData(buffer_io.data(), buffer_io.size());
				}
				else {
					processor_sink_data_cb((unsigned char*)buffer_io.data(), buffer_io.size(), &g_cs);
				}
				calc_play_frame_no(g_cs.m_play_frame_gap);
				fprintf(stdout, "%s 3 m_status %d  m_start_play_frame_no %d\r", __FUNCTION__, m_status, m_start_play_frame_no);
			}

			int64_t	now2 = get_current_time_in_ms();
			double t = 1000.0f / ((double)g_cs.m_play_frame_rate + 2.0f);
			if (now2 - now < t) {
				//fprintf(stdout, " sleep:%f", t - (double)(now2 - now));
				sleep(t - (double)(now2 - now));
			}
		}
		else if (Pb_STATUS_PLAY_CAMERAS == m_status) {

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
		else if (Pb_STATUS_PLAY_FROM_A2B_LOOP == m_status) {
			//fprintf(stdout, "%s 0 m_status %d  m_from_a2b_index %d\n", __FUNCTION__, m_status, m_from_a2b_index);
			int nByteRead = g_cs.m_file_storage_object_for_read->read_file(buffer_io.data(), buffer_io.size(), m_from_a2b_index);
			if (nByteRead < 0) {//maybe frame A is overwrittern!!! //should not be here!!!
				if (m_toward_2b) {
					m_from_a2b_index += g_cs.m_play_frame_gap;
				}
				else {
					m_status = Pb_STATUS_PLAY_PAUSE;
					fprintf(stdout, "%s 1 m_status %d  m_from_a2b_index %d\n", __FUNCTION__, m_status, m_from_a2b_index);
				}

				if (m_from_a2b_index >= m_from_a2b_to) {
					m_from_a2b_index -= (g_cs.m_play_frame_gap * 2);
					m_toward_2b = !m_toward_2b;
				}
				else if (m_from_a2b_from <= m_from_a2b_from) {
					m_from_a2b_index += (g_cs.m_play_frame_gap * 2);
					m_toward_2b = !m_toward_2b;
				}
				continue;
			}
			else if (nByteRead == 0) {
				if (m_toward_2b) {
					m_from_a2b_index += 1;
				}
				else {
					m_from_a2b_index -= 1;
				}

				if (m_from_a2b_index >= m_from_a2b_to) {
					m_from_a2b_index -= (1 * 2);
					m_toward_2b = !m_toward_2b;
				}
				else if (m_from_a2b_from <= m_from_a2b_from) {
					m_from_a2b_index += (1 * 2);
					m_toward_2b = !m_toward_2b;
				}
				fprintf(stdout, "%s 2 m_status %d  m_from_a2b_index %d m_toward_2b %d\n", __FUNCTION__, m_status, m_from_a2b_index, m_toward_2b);
				continue;
			}
			else {
				if (g_cs.m_processor_data_flag) {
					g_cs.m_post_processor_thread->writeImageData(buffer_io.data(), buffer_io.size());
				}
				else {
					processor_sink_data_cb((unsigned char*)buffer_io.data(), buffer_io.size(), &g_cs);
				}

				if (m_toward_2b) {
					m_from_a2b_index += g_cs.m_play_frame_gap;
				}
				else {
					m_from_a2b_index -= g_cs.m_play_frame_gap;
				}

				if (m_from_a2b_index >= m_from_a2b_to) {
					m_from_a2b_index -= (g_cs.m_play_frame_gap*2);
					m_toward_2b = !m_toward_2b;
				}
				else if (m_from_a2b_index <= m_from_a2b_from) {
					m_from_a2b_index += (g_cs.m_play_frame_gap*2);
					m_toward_2b = !m_toward_2b;
				}

				fprintf(stdout, "%s 3 m_status %d  m_from_a2b_index %d m_toward_2b %d\r", __FUNCTION__, m_status, m_from_a2b_index, m_toward_2b);
			}

			int64_t	now2 = get_current_time_in_ms();
			double t = 1000.0f / ((double)g_cs.m_play_frame_rate + 2.0f);
			if (now2 - now < t) {
				//fprintf(stdout, " sleep:%f", t - (double)(now2 - now));
				sleep(t - (double)(now2 - now));
			}
		}
	}

	RingBuffer_destroy(m_ring_buffer);
	m_ring_buffer = NULL;
	timeEndPeriod(1);
}


inline int processor_sink_data_cb(unsigned char* data, int data_len, void* ctx) {
	CustomStruct* p = (CustomStruct*)ctx;
	//fprintf(stdout, "\n%s\n", __FUNCTION__);
	return p->m_snd_data_thread->send((char*)data, data_len);
}

