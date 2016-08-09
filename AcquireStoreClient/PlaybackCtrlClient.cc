
#include "PlaybackCtrlClient.h"	
#include "utils.h"


#define CHECK_CONNECT_STATUS(status) do{if ((status) !=ConnectStatus_CONNECTED) return FALSE; }while(0)
#define CHECK_CONNECT_STATUS_VOID(status) do{if ((status) !=ConnectStatus_CONNECTED) return; }while(0)

CPlaybackCtrlClient::CPlaybackCtrlClient(){
	m_connect_callback = NULL;
	m_connect_ctx = NULL;
}

CPlaybackCtrlClient::~CPlaybackCtrlClient() {
}


std::vector<std::string> CPlaybackCtrlClient::scan_ip(std::string& start_ip, std::string& end_ip) {
	return scan_ip0(start_ip, end_ip);
}
void CPlaybackCtrlClient::set_connected_callback(ConnectedCallback cb, void* ctx) {
	m_connect_callback = cb;
	m_connect_ctx = ctx;
}


void    CPlaybackCtrlClient::start(const std::string& ip, const uint16_t port, int64_t frame_gap) {
	m_server_ip = ip;
	m_cmd_port = port;
	m_frame_gap = frame_gap;
	CMyThread::start();
}
void	CPlaybackCtrlClient::stop() {
	CMyThread::stop();
}


BOOL CPlaybackCtrlClient::connect(const std::string& ip, const uint16_t cmd_port) {
	try {
		WORD wVersionRequested;
		WSADATA wsaData;
		int err;
		wVersionRequested = MAKEWORD(2, 2);
		err = WSAStartup(wVersionRequested, &wsaData);

		TSocket* p = new TSocket(ip, cmd_port);

		m_socket = boost::shared_ptr<TTransport>(p);

		p->setConnTimeout(500);
		p->setSendTimeout(300);
		p->setRecvTimeout(300);

		m_transport = boost::shared_ptr<TTransport>(new TBufferedTransport(m_socket));
		m_protocol = boost::shared_ptr<TProtocol>(new TBinaryProtocol(m_transport));

		m_client = new PlaybackCtrlServiceClient(m_protocol);
		m_client->getInputProtocol().get()->getTransport()->open();

		//uint16_t data_port = 0;
		try {
			m_data_port = m_client->get_data_port();
		}
		catch (TException& e) {
			fprintf(stdout, "%s\n", e.what());
			m_status = ConnectStatus_DISCONNECT;
			return FALSE;
		}
		
		m_recv_thread.init(m_server_ip, m_data_port, m_frame_gap);
		m_recv_thread.start();
		m_status = ConnectStatus_CONNECTED;
		return TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
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
		m_status = ConnectStatus_DISCONNECT;
		return FALSE;
	}
}


int32_t	CPlaybackCtrlClient::start_grab() {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->start_grab();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return FALSE;
	}
}
int32_t CPlaybackCtrlClient::stop_grab() {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->stop_grab();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return FALSE;
	}
}

BOOL CPlaybackCtrlClient::is_connected() {
	CHECK_CONNECT_STATUS(m_status);
	try {
		return m_client->getInputProtocol().get()->getTransport()->isOpen();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return FALSE;
	}
}

void	CPlaybackCtrlClient::set_recv_sink_callback(SinkDataCallback cb, void* context) {
	m_recv_thread.set_sink_data_callback(cb, context);
}



int32_t	CPlaybackCtrlClient::set_play_frame_resolution(const int32_t width, const int32_t height) {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->set_play_frame_resolution(width, height);
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return FALSE;
	}
}
int32_t CPlaybackCtrlClient::set_play_frame_rate(const int32_t play_frame_rate,const int32_t sample_gap) {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->set_play_frame_rate(play_frame_rate, sample_gap);
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return FALSE;
	}
}
int32_t CPlaybackCtrlClient::set_store_file(const int32_t flag) {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->set_store_file(flag);
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return FALSE;
	}
}



int32_t CPlaybackCtrlClient::play_live(const int32_t play_frame_rate, const int32_t sample_gap) {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->play_live(play_frame_rate, sample_gap);
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return FALSE;
	}
}

int32_t CPlaybackCtrlClient::play_forward(const int32_t play_frame_rate, const int32_t sample_gap) {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->play_forward(play_frame_rate, sample_gap);
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return FALSE;
	}
}




int32_t CPlaybackCtrlClient::play_backward(const int32_t play_frame_rate, const int32_t sample_gap) {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->play_backward(play_frame_rate, sample_gap);
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return FALSE;
	}
}

int32_t CPlaybackCtrlClient::play_pause() {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->play_pause();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return FALSE;
	}
}

int32_t	CPlaybackCtrlClient::play_from_a2b(const int64_t a, const int64_t b) {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->play_from_a2b(a,b);
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return FALSE;
	}
}





double  CPlaybackCtrlClient::get_camera__grab_fps() {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->get_camera_grab_fps();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return FALSE;
	}
}
double  CPlaybackCtrlClient::get_soft_grab_fps() {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->get_soft_grab_fps();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return FALSE;
	}
}
double  CPlaybackCtrlClient::get_soft_snd_fps() {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->get_soft_snd_fps();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return FALSE;
	}
}

double  CPlaybackCtrlClient::get_soft_recv_fps() {
	return m_recv_thread.m_soft_recv_counter.GetFPS();
}

double  CPlaybackCtrlClient::get_write_file_fps() {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->get_file_write_fps();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return FALSE;
	}
}


int32_t CPlaybackCtrlClient::set_exposure_time(const double microseconds) {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->set_exposure_time(microseconds);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1;
	}
}
int32_t CPlaybackCtrlClient::set_gain_by_sensor_all(const double gain) {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->set_gain_by_sensor_all(gain);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1;
	}
}

int32_t CPlaybackCtrlClient::set_gain_by_sensor_analog(const double gain) {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->set_gain_by_sensor_analog(gain);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1;
	}
}
int32_t CPlaybackCtrlClient::set_gain_by_sensor_digital(const double gain) {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->set_gain_by_sensor_digital(gain);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1;
	}
}
int32_t CPlaybackCtrlClient::set_frame_rate(const double rate) {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->set_frame_rate(rate);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1;
	}
}
int32_t CPlaybackCtrlClient::enable_turbo_transfer(const int32_t enabled) {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->enable_turbo_transfer(enabled);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1;
	}
}
int32_t CPlaybackCtrlClient::set_pixel_fmt(const std::string& pixel_fmt) {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->set_pixel_fmt(pixel_fmt);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1;
	}
}
int32_t CPlaybackCtrlClient::set_offset_x(const int32_t offset_x) {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->set_offset_x(offset_x);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1;
	}
}
int32_t CPlaybackCtrlClient::set_offset_y(const int32_t offset_y) {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->set_offset_y(offset_y);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1;
	}
}

int32_t CPlaybackCtrlClient::set_image_width(const int32_t width) {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->set_image_width(width);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1;
	}
}

int32_t CPlaybackCtrlClient::set_image_height(const int32_t height) {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->set_image_height(height);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1;
	}
}

double CPlaybackCtrlClient::get_exposure_time() {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->get_exposure_time();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1.f;
	}
}

double CPlaybackCtrlClient::get_gain_by_sensor_all() {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->get_gain_by_sensor_all();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1.f;
	}
}
double CPlaybackCtrlClient::get_gain_by_sensor_analog() {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->get_gain_by_sensor_analog();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1.f;
	}
}

double CPlaybackCtrlClient::get_gain_by_sensor_digital() {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->get_gain_by_sensor_digital();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1.f;
	}
}
double CPlaybackCtrlClient::get_frame_rate() {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->get_frame_rate();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1.f;
	}
}

void CPlaybackCtrlClient::get_exposure_time_range(double* min, double* max) {
	CHECK_CONNECT_STATUS_VOID(m_status);

	try {
		MinMaxStruct r;
		m_client->get_exposure_time_range(r);
		*min = r.min;
		*max = r.max;
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return;
	}
}
void CPlaybackCtrlClient::get_gain_range_by_sensor_all(double* min, double* max) {
	CHECK_CONNECT_STATUS_VOID(m_status);

	try {
		MinMaxStruct r;
		m_client->get_gain_range_by_sensor_all(r);
		*min = r.min;
		*max = r.max;
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return;
	}
}
void CPlaybackCtrlClient::get_gain_range_by_sensor_analog(double* min, double* max) {
	CHECK_CONNECT_STATUS_VOID(m_status);

	try {
		MinMaxStruct r;
		m_client->get_gain_range_by_sensor_analog(r);
		*min = r.min;
		*max = r.max;
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return;
	}
}
void CPlaybackCtrlClient::get_gain_range_by_sensor_digital(double* min, double* max) {
	CHECK_CONNECT_STATUS_VOID(m_status);

	try {
		MinMaxStruct r;
		m_client->get_gain_range_by_sensor_digital(r);
		*min = r.min;
		*max = r.max;
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return;
	}
}
void CPlaybackCtrlClient::get_frame_rate_range(double* min, double* max) {
	CHECK_CONNECT_STATUS_VOID(m_status);

	try {
		MinMaxStruct r;
		m_client->get_frame_rate_range(r);
		*min = r.min;
		*max = r.max;
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return;
	}
}
int32_t CPlaybackCtrlClient::get_image_width() {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->get_image_width();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1;
	}
}
int32_t CPlaybackCtrlClient::get_image_height() {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->get_image_height();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1;
	}
}
int32_t CPlaybackCtrlClient::get_height_max() {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->get_height_max();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1;
	}
}
int32_t CPlaybackCtrlClient::get_width_max() {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->get_width_max();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1;
	}
}
int32_t CPlaybackCtrlClient::get_offset_x() {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->get_offset_x();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1;
	}
}
int32_t CPlaybackCtrlClient::get_offset_y() {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->get_offset_y();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1;
	}
}
int32_t CPlaybackCtrlClient::is_enabled_turbo_transfer() {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->is_enabled_turbo_transfer();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return -1;
	}
}
void CPlaybackCtrlClient::get_pixel_fmt(std::string& pixel_fmt) {
	CHECK_CONNECT_STATUS_VOID(m_status);

	try {
		return m_client->get_pixel_fmt(pixel_fmt);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
	}
}
void CPlaybackCtrlClient::get_user_defined_name(std::string& name) {
	CHECK_CONNECT_STATUS_VOID(m_status);

	try {
		return m_client->get_user_defined_name(name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
	}
}

void CPlaybackCtrlClient::get_device_serial_number(std::string& number) {
	CHECK_CONNECT_STATUS_VOID(m_status);

	try {
		return m_client->get_user_defined_name(number);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
	}
}
void CPlaybackCtrlClient::get_current_ip_address(std::string& ip) {
	CHECK_CONNECT_STATUS_VOID(m_status);

	try {
		return m_client->get_current_ip_address(ip);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
	}
}

double CPlaybackCtrlClient::get_grab_fps() {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->get_grab_fps();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return .0f;
	}
}
double CPlaybackCtrlClient::get_process_fps() {
	CHECK_CONNECT_STATUS(m_status);

	try {
		return m_client->get_process_fps();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return .0f;
	}
}
void CPlaybackCtrlClient::save_feature(std::string& content) {
	CHECK_CONNECT_STATUS_VOID(m_status);

	try {
		m_client->save_feature(content);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
	}
}
int32_t CPlaybackCtrlClient::update_feature(const std::string& content) {
	CHECK_CONNECT_STATUS(m_status);

	try {
		m_client->update_feature(content);
		return TRUE;
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		m_status = ConnectStatus_DISCONNECT;
		return FALSE;
	}
}

void CPlaybackCtrlClient::connect_pingpong() {
	try {
		WORD wVersionRequested;
		WSADATA wsaData;
		int err;
		wVersionRequested = MAKEWORD(2, 2);
		err = WSAStartup(wVersionRequested, &wsaData);

		TSocket* p = new TSocket(m_server_ip, m_cmd_port);

		m_socket_pingpong = boost::shared_ptr<TTransport>(p);

		p->setConnTimeout(500);
		p->setSendTimeout(300);
		p->setRecvTimeout(300);

		m_transport_pingpong = boost::shared_ptr<TTransport>(new TBufferedTransport(m_socket_pingpong));
		m_protocol_pingpong = boost::shared_ptr<TProtocol>(new TBinaryProtocol(m_transport_pingpong));

		m_client_pingpong = new PlaybackCtrlServiceClient(m_protocol_pingpong);
		m_client_pingpong->getInputProtocol().get()->getTransport()->open();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
	}
}

void CPlaybackCtrlClient::close_pingpong() {
	m_client_pingpong->getInputProtocol().get()->getTransport()->close();
	delete m_client_pingpong;
	m_protocol_pingpong.reset();
	m_transport_pingpong.reset();
	m_socket_pingpong.reset();
	WSACleanup();
}



void CPlaybackCtrlClient::run() {
	
	m_status = ConnectStatus_NONE;
	
	while (!m_exited) {
		if (m_status == ConnectStatus_NONE) {

			connect(m_server_ip, m_cmd_port);
			if (m_connect_callback && m_last_status != m_status) {
				m_last_status = m_status;
				m_connect_callback(m_status, m_connect_ctx);
			}
		}
		if (m_status == ConnectStatus_DISCONNECT) {
			close();
			connect(m_server_ip, m_cmd_port);

			if (m_connect_callback &&  m_last_status != m_status) {
				m_last_status = m_status;
				m_connect_callback(m_status, m_connect_ctx);
			}
		}

		try {
			connect_pingpong();
			m_client_pingpong->get_data_port();//check as PING
			close_pingpong();
		}
		catch (TException& e) {
			close_pingpong();
			fprintf(stdout, "%s\n", e.what());
			m_status = ConnectStatus_DISCONNECT;
		}
		
		sleep(500);
	}
	close();
}