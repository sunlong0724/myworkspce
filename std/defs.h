#ifndef __DEFS__H__
#define __DEFS__H__

#define ONE_GB 1024*1024*1024

#define FRAME_TIMESTAMP_START  0
#define FRAME_SEQ_START   sizeof int64_t
#define FRAME_DATA_START sizeof int64_t+sizeof int64_t

#define CAMERA_MAX_FPS 180
#define GET_IMAGE_BUFFER_SIZE(w,h)  (sizeof int64_t + sizeof int64_t + (w) * (h))

class CPlaybackCtrlThread;
class CPostProcessor;
class CPostProcessor;
class CSendData;
class CFileStorage;
class CCamera;

#include <vector>
#include <winSock2.h>
#include <Windows.h>

enum CtrlStatus {
	Playback_NONE					= 0x001,
	Playback_PAUSE					= 0x002,

	Playback_START_PLAY_CAMERAS		= 0x004,
	Playback_STOP_PLAY_CAMERAS		= 0x008,
	Playback_PLAYING_CAMERAS		= 0x010,

	Playback_START_PLAY_FILE		= 0x020,
	Playback_STOP_PLAY_FILE			= 0x040,
	Playback_PLAYING_FILE			= 0x080,

	Playback_START_PLAY_FILE_TEMP	= 0x100,
	Playback_STOP_PLAY_FILE_TEMP	= 0x200,
	Playback_PLAYING_FILE_TEMP		= 0x400,
};

enum TRANSPORT_STATUS {
	TRANSPORT_STATUS_NONE,
	TRANSPORT_STATUS_SENDING,
	TRANSPORT_STATUS_RECVING
};

typedef struct _CustomStruct {
	_CustomStruct():m_playback_thread(NULL), m_post_processor_thread(NULL), m_snd_data_thread(NULL), m_file_storage_object_for_write(NULL), m_file_storage_object_for_read(NULL), m_camera(NULL),
		m_frame_counter(0),	m_store_file_flag(0), m_snd_live_frame_flag(0), m_processor_data_flag(0), m_play_frame_gap(0), m_last_live_play_seq(0), m_data_port(0), m_play_frame_rate(1){}


	
	CPlaybackCtrlThread			*m_playback_thread;
	CPostProcessor				*m_post_processor_thread;
	CSendData					*m_snd_data_thread;
	CFileStorage				*m_file_storage_object_for_write;
	CFileStorage				*m_file_storage_object_for_read;
	CCamera						*m_camera;

	//std::vector<char>			m_buffer;

	BOOL						m_store_file_flag;
	BOOL						m_snd_live_frame_flag;
	BOOL						m_processor_data_flag;

	uint16_t					m_data_port;

	int32_t						m_image_w;
	int32_t						m_image_h;

	int64_t						m_frame_counter;//global counter
	int64_t						m_frame_rate;

	int64_t						m_play_frame_gap;
	int64_t						m_play_frame_rate;

	int64_t						m_play_frame_gap_temp;
	int64_t						m_play_frame_rate_temp;


	int64_t						m_last_live_play_seq;

	int32_t						m_play_frame_w;
	int32_t						m_play_frame_h;
}CustomStruct;


#endif // !__DEFS__H_
