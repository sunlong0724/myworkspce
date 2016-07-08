#ifndef __ACQUIRE_STORE_CLIENT_H__
#define __ACQUIRE_STORE_CLIENT_H__


#include "AcquireStoreService.h"

#include <thrift/Thrift.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace hawkeye;

class CAcquireStoreClient {
public:

	CAcquireStoreClient();
	~CAcquireStoreClient();

	BOOL			connect(const std::string& ip, const uint16_t port);
	BOOL			close();
	BOOL			is_connected();

	int32_t start(const int32_t snd_frame_rate);
	int32_t stop();
	int32_t set_snd_frame_rate(const int32_t snd_frame_rate, const int32_t full_frame_rate);
	int32_t set_store_file(const int32_t flag, const std::string& file_name);
	int32_t do_pause(const int32_t flag);
	int32_t forward_play(const int64_t frame_seq, const int32_t snd_frame_rate);
	int32_t backward_play(const int64_t frame_seq, const int32_t snd_frame_rate);
	int32_t forward_play_temporary(const int64_t frame_seq, const int32_t snd_frame_rate);
	int32_t backward_play_temporary(const int64_t frame_seq, const int32_t snd_frame_rate) ;

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
	void get_exposure_time_range(MinMaxStruct& _return) ;
	void get_gain_range_by_sensor_all(MinMaxStruct& _return) ;
	void get_gain_range_by_sensor_analog(MinMaxStruct& _return) ;
	void get_gain_range_by_sensor_digital(MinMaxStruct& _return) ;
	void get_frame_rate_range(MinMaxStruct& _return) ;
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

private:
	AcquireStoreServiceClient*		m_client;

	boost::shared_ptr<TTransport>	m_socket;
	boost::shared_ptr<TTransport>	m_transport;
	boost::shared_ptr<TProtocol>	m_protocol;

	uint16_t						m_data_port;
	std::string						m_server_ip;

};
#endif // !__ACQUIRE_STORE_CLIENT_H__
