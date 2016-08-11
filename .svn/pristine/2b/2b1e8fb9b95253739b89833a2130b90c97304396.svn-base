#ifndef __PLAYBACK_CTRL_H__
#define __PLAYBACK_CTRL_H__
#include "MyThread.h"

#include "utils.h"

class CPlaybackCtrlThread : public CMyThread {
public:
	CPlaybackCtrlThread();
	~CPlaybackCtrlThread();

	int64_t writeImageData(char* buffer, int len);
	void calc_play_frame_no(int gap);

protected:
	void run();


public:
	std::vector<char>	m_buffer_for_camera_io;

	int64_t				m_start_play_frame_no;
	int64_t				m_start_play_frame_no_begin;

	RingBuffer			*m_ring_buffer;
	Playback_Status			m_status;
	Playback_Status			m_last_status;

	int64_t				m_from_a2b_from;
	int64_t				m_from_a2b_to;
	int64_t				m_from_a2b_index;
	BOOL				m_toward_2b;

	//IplImage*			g_bayer  ;
};



#endif // !__PLAYBACK_CTRL_H__

