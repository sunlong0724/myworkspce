
#include "PlaybackCtrlClient.h"	
#include "utils.h"

CPlaybackCtrlClient::CPlaybackCtrlClient(){
}

CPlaybackCtrlClient::~CPlaybackCtrlClient() {
}


std::vector<std::string> CPlaybackCtrlClient::scan_ip(std::string& start_ip, std::string& end_ip) {
	return scan_ip0(start_ip, end_ip);
}


BOOL CPlaybackCtrlClient::connect(const std::string& ip, const uint16_t port) {
	try {
		WORD wVersionRequested;
		WSADATA wsaData;
		int err;
		wVersionRequested = MAKEWORD(2, 2);
		err = WSAStartup(wVersionRequested, &wsaData);

		m_server_ip = ip;
		TSocket* p = new TSocket(ip, port);

		m_socket = boost::shared_ptr<TTransport>(p);

		p->setConnTimeout(300);
		p->setSendTimeout(300);
		p->setRecvTimeout(300);

		m_transport = boost::shared_ptr<TTransport>(new TBufferedTransport(m_socket));
		m_protocol = boost::shared_ptr<TProtocol>(new TBinaryProtocol(m_transport));

		m_client = new PlaybackCtrlServiceClient(m_protocol);
		m_client->getInputProtocol().get()->getTransport()->open();
		m_server_ip = ip;
		uint16_t data_port = 0;
		try {
			data_port=  m_client->get_data_port();
			m_recv_thread.init(m_server_ip, data_port);
			m_recv_thread.start();
		}
		catch (TException& e) {
			fprintf(stdout, "%s\n", e.what());
			return FALSE;
		}

		return TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}

BOOL CPlaybackCtrlClient::close() {
	try {
		m_recv_thread.stop();
		m_client->getInputProtocol().get()->getTransport()->close();
		delete m_client;
		m_protocol.reset();
		m_transport.reset();
		m_socket.reset();
		WSACleanup();
		return TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}

BOOL CPlaybackCtrlClient::is_connected() {
	try {
		return m_client->getInputProtocol().get()->getTransport()->isOpen();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}

void	CPlaybackCtrlClient::set_recv_sink_callback(SinkDataCallback cb, void* context) {
	m_recv_thread.set_sink_data_callback(cb, context);
}
void	CPlaybackCtrlClient::set_recv_image_parameters(int32_t elem_size, int64_t frame_gap) {
	m_recv_thread.set_parameters(elem_size, frame_gap);
}

int32_t	CPlaybackCtrlClient::get_frames_data(int32_t frame_seq, int32_t how_many_frames) {
	return 0;
}



int32_t CPlaybackCtrlClient::start_play_live(const int32_t play_frame_rate, const int32_t sample_gap) {
	try {
		return m_client->start_play_live(play_frame_rate,sample_gap);
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}
int32_t CPlaybackCtrlClient::stop_play_live() {
	try {
		return  m_client->stop_play_live();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}
int32_t CPlaybackCtrlClient::play_live() {
	try {
		return  m_client->play_live();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}
int32_t	CPlaybackCtrlClient::set_play_frame_resolution(const int32_t width, const int32_t height) {
	try {
		return m_client->set_play_frame_resolution(width, height);
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}
int32_t CPlaybackCtrlClient::set_play_frame_rate(const int32_t play_frame_rate,const int32_t sample_gap) {
	try {
		return m_client->set_play_frame_rate(play_frame_rate, sample_gap);
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}
int32_t CPlaybackCtrlClient::set_store_file(const int32_t flag, const std::string& file_name) {
	try {
		return m_client->set_store_file(flag, file_name);
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}

int32_t CPlaybackCtrlClient::start_forward_play(const int32_t play_frame_rate, const int32_t sample_gap) {
	try {
		return m_client->start_forward_play(play_frame_rate, sample_gap);
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}

int32_t CPlaybackCtrlClient::stop_forward_play() {
	try {
		return m_client->stop_forward_play();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}

int32_t CPlaybackCtrlClient::forward_play() {
	try {
		return m_client->forward_play();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}

int32_t CPlaybackCtrlClient::start_backward_play(const int32_t play_frame_rate, const int32_t sample_gap) {
	try {
		return m_client->start_backward_play(play_frame_rate, sample_gap);
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}


int32_t CPlaybackCtrlClient::stop_backward_play() {
	try {
		return m_client->stop_backward_play();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}

int32_t CPlaybackCtrlClient::backward_play() {
	try {
		return m_client->backward_play();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}

int32_t CPlaybackCtrlClient::start_forward_play_temp(const int32_t play_frame_rate, const int32_t sample_gap) {
	try {
		return m_client->start_forward_play_temp(play_frame_rate, sample_gap);
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}

int32_t CPlaybackCtrlClient::stop_forward_play_temp() {
	try {
		return m_client->stop_forward_play_temp();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}

int32_t CPlaybackCtrlClient::forward_play_temp() {
	try {
		return m_client->forward_play_temp();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}

int32_t CPlaybackCtrlClient::start_backward_play_temp(const int32_t play_frame_rate, const int32_t sample_gap) {
	try {
		return m_client->start_backward_play_temp(play_frame_rate, sample_gap);
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}

int32_t CPlaybackCtrlClient::stop_backward_play_temp() {
	try {
		return m_client->stop_backward_play_temp();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}

int32_t CPlaybackCtrlClient::backward_play_temp() {
	try {
		return m_client->backward_play_temp();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}

double  CPlaybackCtrlClient::get_camera__grab_fps() {
	try {
		return m_client->get_camera_grab_fps();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}
double  CPlaybackCtrlClient::get_soft_grab_fps() {
	try {
		return m_client->get_soft_grab_fps();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}
double  CPlaybackCtrlClient::get_soft_snd_fps() {
	try {
		return m_client->get_soft_snd_fps();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}


int32_t CPlaybackCtrlClient::set_exposure_time(const double microseconds) {
	try {
		return m_client->set_exposure_time(microseconds);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CPlaybackCtrlClient::set_gain_by_sensor_all(const double gain) {
	try {
		return m_client->set_gain_by_sensor_all(gain);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}

int32_t CPlaybackCtrlClient::set_gain_by_sensor_analog(const double gain) {
	try {
		return m_client->set_gain_by_sensor_analog(gain);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CPlaybackCtrlClient::set_gain_by_sensor_digital(const double gain) {
	try {
		return m_client->set_gain_by_sensor_digital(gain);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CPlaybackCtrlClient::set_frame_rate(const double rate) {
	try {
		return m_client->set_frame_rate(rate);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CPlaybackCtrlClient::enable_turbo_transfer(const int32_t enabled) {
	try {
		return m_client->enable_turbo_transfer(enabled);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CPlaybackCtrlClient::set_pixel_fmt(const std::string& pixel_fmt) {
	try {
		return m_client->set_pixel_fmt(pixel_fmt);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CPlaybackCtrlClient::set_offset_x(const int32_t offset_x) {
	try {
		return m_client->set_offset_x(offset_x);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CPlaybackCtrlClient::set_offset_y(const int32_t offset_y) {
	try {
		return m_client->set_offset_y(offset_y);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}

int32_t CPlaybackCtrlClient::set_image_width(const int32_t width) {
	try {
		return m_client->set_image_width(width);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}

int32_t CPlaybackCtrlClient::set_image_height(const int32_t height) {
	try {
		return m_client->set_image_height(height);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}

double CPlaybackCtrlClient::get_exposure_time() {
	try {
		return m_client->get_exposure_time();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1.f;
	}
}

double CPlaybackCtrlClient::get_gain_by_sensor_all() {
	try {
		return m_client->get_gain_by_sensor_all();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1.f;
	}
}
double CPlaybackCtrlClient::get_gain_by_sensor_analog() {
	try {
		return m_client->get_gain_by_sensor_analog();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1.f;
	}
}

double CPlaybackCtrlClient::get_gain_by_sensor_digital() {
	try {
		return m_client->get_gain_by_sensor_digital();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1.f;
	}
}
double CPlaybackCtrlClient::get_frame_rate() {
	try {
		return m_client->get_frame_rate();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1.f;
	}
}

void CPlaybackCtrlClient::get_exposure_time_range(double* min, double* max) {
	try {
		MinMaxStruct r;
		m_client->get_exposure_time_range(r);
		*min = r.min;
		*max = r.max;
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return;
	}
}
void CPlaybackCtrlClient::get_gain_range_by_sensor_all(double* min, double* max) {
	try {
		MinMaxStruct r;
		m_client->get_gain_range_by_sensor_all(r);
		*min = r.min;
		*max = r.max;
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return;
	}
}
void CPlaybackCtrlClient::get_gain_range_by_sensor_analog(double* min, double* max) {
	try {
		MinMaxStruct r;
		m_client->get_gain_range_by_sensor_analog(r);
		*min = r.min;
		*max = r.max;
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return;
	}
}
void CPlaybackCtrlClient::get_gain_range_by_sensor_digital(double* min, double* max) {
	try {
		MinMaxStruct r;
		m_client->get_gain_range_by_sensor_digital(r);
		*min = r.min;
		*max = r.max;
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return;
	}
}
void CPlaybackCtrlClient::get_frame_rate_range(double* min, double* max) {
	try {
		MinMaxStruct r;
		m_client->get_frame_rate_range(r);
		*min = r.min;
		*max = r.max;
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return;
	}
}
int32_t CPlaybackCtrlClient::get_image_width() {
	try {
		return m_client->get_image_width();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CPlaybackCtrlClient::get_image_height() {
	try {
		return m_client->get_image_height();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CPlaybackCtrlClient::get_height_max() {
	try {
		return m_client->get_height_max();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CPlaybackCtrlClient::get_width_max() {
	try {
		return m_client->get_width_max();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CPlaybackCtrlClient::get_offset_x() {
	try {
		return m_client->get_offset_x();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CPlaybackCtrlClient::get_offset_y() {
	try {
		return m_client->get_offset_y();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CPlaybackCtrlClient::is_enabled_turbo_transfer() {
	try {
		return m_client->is_enabled_turbo_transfer();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
void CPlaybackCtrlClient::get_pixel_fmt(std::string& pixel_fmt) {
	try {
		return m_client->get_pixel_fmt(pixel_fmt);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
	}
}
void CPlaybackCtrlClient::get_user_defined_name(std::string& name) {
	try {
		return m_client->get_user_defined_name(name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
	}
}

void CPlaybackCtrlClient::get_device_serial_number(std::string& number) {
	try {
		return m_client->get_user_defined_name(number);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
	}
}
void CPlaybackCtrlClient::get_current_ip_address(std::string& ip) {
	try {
		return m_client->get_current_ip_address(ip);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
	}
}

double CPlaybackCtrlClient::get_grab_fps() {
	try {
		return m_client->get_grab_fps();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return .0f;
	}
}
double CPlaybackCtrlClient::get_process_fps() {
	try {
		return m_client->get_process_fps();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return .0f;
	}
}
void CPlaybackCtrlClient::save_feature(std::string& content) {
	try {
		m_client->save_feature(content);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
	}
}
int32_t CPlaybackCtrlClient::update_feature(const std::string& content) {
	try {
		m_client->update_feature(content);
		return TRUE;
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return FALSE;
	}
}
