#ifndef __ACQUIRE_STORE_CLIENT_H__
#define __ACQUIRE_STORE_CLIENT_H__


#include "PlaybackCtrlService.h"

#include <thrift/Thrift.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include <TransportData.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace hawkeye;

typedef int(*SinkDataCallback)(unsigned char*, int, void*);


class CPlaybackCtrlClient {
public:

	CPlaybackCtrlClient();
	~CPlaybackCtrlClient();

	std::vector<std::string> scan_ip(std::string& start_ip, std::string& end_ip);

	BOOL			connect(const std::string& ip, const uint16_t port);
	BOOL			close();
	BOOL			is_connected();

	//PLAYBACK

	void	set_recv_sink_callback(SinkDataCallback cb, void* context);
	void	set_recv_image_parameters(int32_t elem_size, int64_t frame_gap = 1);

	int32_t	set_play_frame_resolution(const int32_t width, const int32_t height);//FIXME:  
	int32_t set_play_frame_rate(const int32_t play_frame_rate, const int32_t sample_gap);
	int32_t set_store_file(const int32_t flag, const std::string& file_name);

	int32_t	get_frames_data(int32_t frame_seq, int32_t how_many_frames);

	int32_t start_play_live(const int32_t play_frame_rate,const int32_t sample_gap);
	int32_t stop_play_live();
	int32_t play_live();

	int32_t start_forward_play(const int32_t play_frame_rate, const int32_t sample_gap);
	int32_t stop_forward_play();
	int32_t forward_play();

	int32_t start_backward_play(const int32_t play_frame_rate, const int32_t sample_gap);
	int32_t stop_backward_play();
	int32_t backward_play();

	int32_t start_forward_play_temp(const int32_t play_frame_rate, const int32_t sample_gap);
	int32_t stop_forward_play_temp();
	int32_t forward_play_temp();

	int32_t start_backward_play_temp(const int32_t play_frame_rate, const int32_t sample_gap);
	int32_t stop_backward_play_temp();
	int32_t backward_play_temp();

	double  get_camera__grab_fps();
	double  get_soft_grab_fps();
	double  get_soft_snd_fps();

	//CAMERA
	int32_t set_exposure_time(const double microseconds) ;
	int32_t set_gain_by_sensor_all(const double gain) ;
	int32_t set_gain_by_sensor_analog(const double gain) ;
	int32_t set_gain_by_sensor_digital(const double gain) ;

	int32_t set_frame_rate(const double rate) ;
	int32_t enable_turbo_transfer(const int32_t enabled) ;

	int32_t set_pixel_fmt(const std::string& pixel_fmt) ;
	int32_t set_offset_x(const int32_t offset_x) ;
	int32_t set_offset_y(const int32_t offset_y) ;
	int32_t set_image_width(const int32_t width) ;
	int32_t set_image_height(const int32_t height) ;
	double get_exposure_time() ;
	double get_gain_by_sensor_all() ;
	double get_gain_by_sensor_analog() ;
	double get_gain_by_sensor_digital() ;
	double get_frame_rate() ;
	void get_exposure_time_range(double* min, double* max) ;
	void get_gain_range_by_sensor_all(double* min, double* max) ;
	void get_gain_range_by_sensor_analog(double* min, double* max) ;
	void get_gain_range_by_sensor_digital(double* min, double* max) ;
	void get_frame_rate_range(double* min, double* max) ;
	int32_t get_image_width() ;
	int32_t get_image_height() ;
	int32_t get_height_max() ;
	int32_t get_width_max() ;
	int32_t get_offset_x() ;
	int32_t get_offset_y() ;
	int32_t is_enabled_turbo_transfer() ;
	void get_pixel_fmt(std::string& _return) ;
	void get_user_defined_name(std::string& _return) ;
	void get_device_serial_number(std::string& _return) ;
	void get_current_ip_address(std::string& _return) ;
	double get_grab_fps() ;
	double get_process_fps() ;
	void save_feature(std::string& _return) ;
	int32_t update_feature(const std::string& content) ;

	//WATCH
	//int32_t	query_camrea_status();
	
private:
	PlaybackCtrlServiceClient		*m_client;

	boost::shared_ptr<TTransport>	m_socket;
	boost::shared_ptr<TTransport>	m_transport;
	boost::shared_ptr<TProtocol>	m_protocol;

	std::string						m_server_ip;
	uint16_t						m_data_port;

	CRecvData						m_recv_thread;
};
#endif // !__ACQUIRE_STORE_CLIENT_H__
