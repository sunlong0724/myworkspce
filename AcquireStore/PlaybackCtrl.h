#ifndef __PLAYBACK_CTRL_H__
#define __PLAYBACK_CTRL_H__
#include "MyThread.h"

#include "utils.h"

class CPlaybackCtrlThread : public CMyThread {
public:
	CPlaybackCtrlThread();
	~CPlaybackCtrlThread();

	int64_t writeImageData(char* buffer, int len);
	void calc_play_frame_no_temp(int gap);

protected:
	void run();


public:
	std::vector<char>	m_buffer_for_camera_io;

	BOOL				m_is_forward;
	int64_t				m_start_play_frame_no;
	int64_t				m_start_play_frame_no_begin;

	RingBuffer			*m_ring_buffer;
	CtrlStatus			m_status;

	//IplImage*			g_bayer  ;
};



#endif // !__PLAYBACK_CTRL_H__

