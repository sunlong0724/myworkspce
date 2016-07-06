#ifndef __AGENT_CLIENT_H__
#define __AGENT_CLIENT_H__

#define DLL_API __declspec(dllexport)  

#include "AgentServerService.h"
#include "ZmqTransportDataImpl.h"

#include <thrift/Thrift.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace hawkeye;

class DLL_API AgentClient {
public:
	AgentClient();
	~AgentClient();

	BOOL			connect(const std::string& ip, const uint16_t port);
	BOOL			close();
	BOOL			is_connected();

	BOOL			find_cameras(std::vector<std::string> & cameras);
	BOOL			get_hold_cameras(std::vector<std::string> & cameras);
	int32_t			add_cameras(const std::vector<std::string> & l);
	int32_t			del_cameras(const std::vector<std::string> & l);
	int32_t			open_camera(const std::string& camera_name);
	int32_t			close_camera(const std::string& camera_name);

	int32_t			start_camera(const std::string& camera_name, const int32_t display_frame_rate);
	int32_t			stop_camera(const std::string& camera_name);

	int32_t			set_exposure_time(const std::string& camera_name, const double microseconds);
	int32_t			set_gain_by_sensor_all(const std::string& camera_name, const double gain);
	int32_t			set_gain_by_sensor_analog(const std::string& camera_name, const double gain);
	int32_t			set_gain_by_sensor_digital(const std::string& camera_name, const double gain);
	int32_t			set_frame_rate(const std::string& camera_name, const double rate);
	int32_t			enable_turbo_transfer(const std::string& camera_name, const int32_t enabled);
	int32_t			set_pixel_fmt(const std::string& camera_name, const std::string& pixel_fmt);
	int32_t			set_offset_x(const std::string& camera_name, const int32_t offset_x);
	int32_t			set_offset_y(const std::string& camera_name, const int32_t offset_y);
	int32_t			set_image_width(const std::string& camera_name, const int32_t width);
	int32_t			set_image_height(const std::string& camera_name, const int32_t height);

	double			get_exposure_time(const std::string& camera_name);
	double			get_gain_by_sensor_all(const std::string& camera_name);//1920 support
	double			get_gain_by_sensor_analog(const std::string& camera_name);//1280 support
	double			get_gain_by_sensor_digital(const std::string& camera_name);//1280 support
	double			get_frame_rate(const std::string& camera_name);
	void			get_exposure_time_range(const std::string& camera_name, double* min, double* max);
	void			get_gain_range_by_sensor_all(const std::string& camera_name, double* min, double* max);
	void			get_gain_range_by_sensor_analog(const std::string& camera_name, double* min, double* max);
	void			get_gain_range_by_sensor_digital(const std::string& camera_name, double* min, double* max);
	void			get_frame_rate_range(const std::string& camera_name, double* min, double* max);
	int32_t			get_image_width(const std::string& camera_name);
	int32_t			get_image_height(const std::string& camera_name);
	int32_t			get_height_max(const std::string& camera_name);
	int32_t			get_width_max(const std::string& camera_name);
	int32_t			get_offset_x(const std::string& camera_name);
	int32_t			get_offset_y(const std::string& camera_name);
	int32_t			is_enabled_turbo_transfer(const std::string& camera_name);
	void			get_pixel_fmt(const std::string& camera_name, std::string& pixel_fmt);
	void			get_user_defined_name(const std::string& camera_name, std::string& name);
	void			get_current_ip_address(const std::string& camera_name, std::string& ip);
	void			get_device_serial_number(const std::string& camera_name, std::string& number);
	double			get_grab_fps(const std::string& camera_name);
	double			get_process_fps(const std::string& camera_name);
	void			save_feature(const std::string& camera_name, std::string& content);
	int32_t			update_feature(const std::string& camera_name, const std::string& content);

	CustomData						*m_custom_data;
private:
	AgentServerServiceClient		*m_client;
	boost::shared_ptr<TTransport>	m_socket;
	boost::shared_ptr<TTransport>	m_transport;
	boost::shared_ptr<TProtocol>	m_protocol;

	uint16_t						m_data_port;
	std::string						m_server_ip;
	int64_t							ping_server();

	enum CONNECTION_STATUS
	{

	};

};


#endif // !__AGENT_CLIENT_H__

