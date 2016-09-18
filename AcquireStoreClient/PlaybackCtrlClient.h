#ifndef __ACQUIRE_STORE_CLIENT_H__
#define __ACQUIRE_STORE_CLIENT_H__
#define DLL_API __declspec(dllexport)  

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



class DLL_API CPlaybackCtrlClient :public CMyThread{
public:

	CPlaybackCtrlClient();
	~CPlaybackCtrlClient();

	std::vector<std::string> scan_ip(std::string& start_ip, std::string& end_ip);


	void	set_connected_callback(ConnectedCallback cb, void* ctx);
	void	set_recv_sink_callback(SinkDataCallback cb, void* context);
	void    start(const std::string& ip, const uint16_t port, int64_t frame_gap);//do not start recv data thread when frame_gap  is 0;
	void	stop();

	int32_t	start_grab();
	int32_t stop_grab();

	int32_t	set_play_frame_resolution(const int32_t width, const int32_t height);//FIXME:  
	int32_t set_play_frame_rate(const int32_t play_frame_rate, const int32_t sample_gap);
	int32_t set_store_file(const int32_t flag);

	int32_t play_live(const int32_t play_frame_rate, const int32_t sample_gap);;
	int32_t play_forward(const int32_t play_frame_rate, const int32_t sample_gap);
	int32_t play_backward(const int32_t play_frame_rate, const int32_t sample_gap);
	int32_t play_pause();
	int32_t	play_from_a2b(const int64_t a, const int64_t b);

	int32_t get_the_frame_data(const int8_t direct, const int8_t gap);//direct:1--forward 2--backward  gap:=0 invalid; >0 forwad; <0 backward
	int64_t sync_frame_by_timestamp_in_pause(const int64_t timestamp);

	double  get_camera__grab_fps();
	double  get_soft_grab_fps();
	double  get_soft_snd_fps();
	double  get_soft_recv_fps();
	double  get_write_file_fps();
	int32_t kill_myself();

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


protected:
	void run();

private:
	BOOL			connect(const std::string& ip, const uint16_t port);
	BOOL			close();
	BOOL			is_connected();
	void			connect_pingpong();
	void			close_pingpong();
	
private:
	PlaybackCtrlServiceClient		*m_client;

	boost::shared_ptr<TTransport>	m_socket;
	boost::shared_ptr<TTransport>	m_transport;
	boost::shared_ptr<TProtocol>	m_protocol;

	PlaybackCtrlServiceClient		*m_client_pingpong;
	boost::shared_ptr<TTransport>	m_socket_pingpong;
	boost::shared_ptr<TTransport>	m_transport_pingpong;
	boost::shared_ptr<TProtocol>	m_protocol_pingpong;

	std::string						m_server_ip;
	uint16_t						m_cmd_port;
	uint16_t						m_data_port;

	int64_t							m_frame_gap;
	CRecvData						m_recv_thread;
	CRecvDatabyThrift				m_recv_thread_by_thrift;
	const int						m_transport_mode = 2;		//1--zmq 2--thrift

	ConnectStatus					m_status;
	ConnectStatus					m_last_status;
	ConnectedCallback				m_connect_callback;
	void*							m_connect_ctx;

};
#endif // !__ACQUIRE_STORE_CLIENT_H__
