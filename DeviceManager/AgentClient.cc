#include "AgentClient.h"


#define CHECK_CONNECTION()			\
	do{										\
		if (camera_name.length() == 0) {	\
		  return -1;						\
		}									\
		if (m_hold_camera_names.find(camera_name) == m_hold_camera_names.end()) {\
		 return -2;							\
		}									\
	}while(0)


AgentClient::AgentClient() {
	m_custom_data = new CustomData;
}

AgentClient::~AgentClient() {
	delete m_custom_data;
}

BOOL AgentClient::connect(const std::string& ip, const uint16_t port) {
	try {
		WORD wVersionRequested;
		WSADATA wsaData;
		int err;
		wVersionRequested = MAKEWORD(2, 2);
		err = WSAStartup(wVersionRequested, &wsaData);

		m_socket = boost::shared_ptr<TTransport>(new TSocket(ip, port));
		m_transport = boost::shared_ptr<TTransport>(new TBufferedTransport(m_socket));
		m_protocol = boost::shared_ptr<TProtocol>(new TBinaryProtocol(m_transport));

		m_client = new AgentServerServiceClient(m_protocol);
		m_client->getInputProtocol().get()->getTransport()->open();

		m_server_ip = ip;
		return TRUE;
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return FALSE;
	}
}

BOOL AgentClient::close() {
	try {
		m_client->getInputProtocol().get()->getTransport()->close();
		delete m_client;
		m_protocol.reset();
		m_transport.reset();
		m_socket.reset();
		WSACleanup();
		return TRUE;
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return FALSE;
	}
}

BOOL AgentClient::is_connected() {
	try {
		return m_client->getInputProtocol().get()->getTransport()->isOpen();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return FALSE;
	}
}

BOOL AgentClient::find_cameras(std::vector<std::string> & _return) {
	try {
		m_client->find_cameras(_return);
		return	TRUE;
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return FALSE;
	}
}

int32_t AgentClient::add_cameras(const std::vector<std::string> & l) {
	try {
		return m_client->add_cameras(l);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}

BOOL AgentClient::get_hold_cameras(std::vector<std::string> & cameras) {
	try {
		m_client->get_hold_cameras( cameras);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return FALSE;
	}
	return TRUE;
}

int32_t AgentClient::del_cameras(const std::vector<std::string> & l) {
	try {
		return m_client->del_cameras(l);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}

int32_t AgentClient::open_camera(const std::string& camera_name) {
	try {
		return m_data_port = m_client->open(camera_name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t AgentClient::close_camera(const std::string& camera_name) {
	try {
		return  m_client->close(camera_name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}

int32_t AgentClient::start_camera(const std::string& camera_name, const int32_t display_frame_rate) {
	try {

		set_display_frame_rate0(m_custom_data, display_frame_rate, m_client->get_frame_rate(camera_name));
		create_zqm_ctx(m_custom_data, get_image_width(camera_name)* get_image_height(camera_name), m_server_ip, m_data_port, 2);
		return m_client->start(camera_name, display_frame_rate);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}

int32_t AgentClient::stop_camera(const std::string& camera_name) {
	try {
		destroy_zqm_ctx(m_custom_data);
		return m_client->stop(camera_name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}

int32_t AgentClient::set_exposure_time(const std::string& camera_name, const double microseconds) {
	try {
		return m_client->set_exposure_time(camera_name, microseconds);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t AgentClient::set_gain_by_sensor_all(const std::string& camera_name, const double gain) {
	try {
		return m_client->set_gain_by_sensor_all(camera_name, gain);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}

int32_t AgentClient::set_gain_by_sensor_analog(const std::string& camera_name, const double gain) {
	try {
		return m_client->set_gain_by_sensor_analog(camera_name, gain);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t AgentClient::set_gain_by_sensor_digital(const std::string& camera_name, const double gain) {
	try {
		return m_client->set_gain_by_sensor_digital(camera_name, gain);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t AgentClient::set_frame_rate(const std::string& camera_name, const double rate) {
	try {
		return m_client->set_frame_rate(camera_name, rate);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t AgentClient::enable_turbo_transfer(const std::string& camera_name, const int32_t enabled) {
	try {
		return m_client->enable_turbo_transfer(camera_name, enabled);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t AgentClient::set_pixel_fmt(const std::string& camera_name, const std::string& pixel_fmt) {
	try {
		return m_client->set_pixel_fmt(camera_name, pixel_fmt);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t AgentClient::set_offset_x(const std::string& camera_name, const int32_t offset_x) {
	try {
		return m_client->set_offset_x(camera_name, offset_x);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t AgentClient::set_offset_y(const std::string& camera_name, const int32_t offset_y) {
	try {
		return m_client->set_offset_y(camera_name, offset_y);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}

int32_t AgentClient::set_image_width(const std::string& camera_name, const int32_t width) {
	try {
		return m_client->set_image_width(camera_name, width);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}

int32_t AgentClient::set_image_height(const std::string& camera_name, const int32_t height) {
	try {
		return m_client->set_image_height(camera_name, height);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}

double AgentClient::get_exposure_time(const std::string& camera_name) {
	try {
		return m_client->get_exposure_time(camera_name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1.f;
	}
}

double AgentClient::get_gain_by_sensor_all(const std::string& camera_name) {
	try {
		return m_client->get_gain_by_sensor_all(camera_name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1.f;
	}
}
double AgentClient::get_gain_by_sensor_analog(const std::string& camera_name) {
	try {
		return m_client->get_gain_by_sensor_analog(camera_name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1.f;
	}
}

double AgentClient::get_gain_by_sensor_digital(const std::string& camera_name) {
	try {
		return m_client->get_gain_by_sensor_digital(camera_name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1.f;
	}
}
double AgentClient::get_frame_rate(const std::string& camera_name) {
	try {
		return m_client->get_frame_rate(camera_name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1.f;
	}
}

void AgentClient::get_exposure_time_range(const std::string& camera_name, double* min, double* max) {
	try {
		MinMaxStruct r;
		m_client->get_exposure_time_range(r, camera_name);
		*min = r.min;
		*max = r.max;
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return ;
	}
}
void AgentClient::get_gain_range_by_sensor_all(const std::string& camera_name, double* min, double* max) {
	try {
		MinMaxStruct r;
		m_client->get_gain_range_by_sensor_all(r, camera_name);
		*min = r.min;
		*max = r.max;
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return;
	}
}
void AgentClient::get_gain_range_by_sensor_analog(const std::string& camera_name, double* min, double* max) {
	try {
		MinMaxStruct r;
		m_client->get_gain_range_by_sensor_analog(r, camera_name);
		*min = r.min;
		*max = r.max;
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return;
	}
}
void AgentClient::get_gain_range_by_sensor_digital(const std::string& camera_name, double* min, double* max) {
	try {
		MinMaxStruct r;
		m_client->get_gain_range_by_sensor_digital(r, camera_name);
		*min = r.min;
		*max = r.max;
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return;
	}
}
void AgentClient::get_frame_rate_range(const std::string& camera_name, double* min, double* max) {
	try {
		MinMaxStruct r;
		m_client->get_frame_rate_range(r, camera_name);
		*min = r.min;
		*max = r.max;
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return;
	}
}
int32_t AgentClient::get_image_width(const std::string& camera_name) {
	try {
		return m_client->get_image_width(camera_name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t AgentClient::get_image_height(const std::string& camera_name) {
	try {
		return m_client->get_image_height(camera_name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t AgentClient::get_height_max(const std::string& camera_name) {
	try {
		return m_client->get_height_max(camera_name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t AgentClient::get_width_max(const std::string& camera_name) {
	try {
		return m_client->get_width_max(camera_name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t AgentClient::get_offset_x(const std::string& camera_name) {
	try {
		return m_client->get_offset_x(camera_name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t AgentClient::get_offset_y(const std::string& camera_name) {
	try {
		return m_client->get_offset_y(camera_name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t AgentClient::is_enabled_turbo_transfer(const std::string& camera_name) {
	try {
		return m_client->is_enabled_turbo_transfer(camera_name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
void AgentClient::get_pixel_fmt(const std::string& camera_name, std::string& pixel_fmt) {
	try {
		return m_client->get_pixel_fmt(pixel_fmt,camera_name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
	}
}
void AgentClient::get_user_defined_name(const std::string& camera_name, std::string& name) {
	try {
		return m_client->get_user_defined_name(name, camera_name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
	}
}

void AgentClient::get_device_serial_number(const std::string& camera_name, std::string& number) {
	try {
		return m_client->get_user_defined_name(number, camera_name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
	}
}
void AgentClient::get_current_ip_address(const std::string& camera_name, std::string& ip) {
	try {
		return m_client->get_current_ip_address(ip, camera_name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
	}
}

double AgentClient::get_grab_fps(const std::string& camera_name) {
	try {
		return m_client->get_grab_fps(camera_name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return .0f;
	}
}
double AgentClient::get_process_fps(const std::string& camera_name) {
	try {
		return m_client->get_process_fps(camera_name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return .0f;
	}
}
void AgentClient::save_feature(const std::string& camera_name, std::string& content) {
	try {
		m_client->save_feature( content, camera_name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
	}
}
int32_t AgentClient::update_feature(const std::string& camera_name, const std::string& content) {
	try {
		m_client->update_feature(camera_name, content);
		return TRUE;
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return FALSE;
	}
}

int64_t	AgentClient::ping_server() {
	static int64_t counter = 0;
	try {
		counter = m_client->ping_server(counter);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
