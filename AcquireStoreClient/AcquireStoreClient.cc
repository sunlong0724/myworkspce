
#include "AcquireStoreClient.h"	
#include "utils.h"

CAcquireStoreClient::CAcquireStoreClient():m_pProcessor(new CPostProcessor()){
}

CAcquireStoreClient::~CAcquireStoreClient() {
	delete m_pProcessor;
}


std::vector<std::string> CAcquireStoreClient::scan_ip(std::string& start_ip, std::string& end_ip) {
	return scan_ip0(start_ip, end_ip);
}


BOOL CAcquireStoreClient::connect(const std::string& ip, const uint16_t port) {
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

		m_client = new AcquireStoreServiceClient(m_protocol);
		m_client->getInputProtocol().get()->getTransport()->open();

		m_server_ip = ip;
		return TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}

BOOL CAcquireStoreClient::close() {
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
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}

BOOL CAcquireStoreClient::is_connected() {
	try {
		return m_client->getInputProtocol().get()->getTransport()->isOpen();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}

int32_t	CAcquireStoreClient::init() {
	try {
		return m_client->init();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}
int32_t CAcquireStoreClient::uninit() {
	try {
		return m_client->uninit();
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}


int32_t CAcquireStoreClient::start(const int32_t snd_frame_rate) {
	try {
		int ret = m_client->start(snd_frame_rate);
		if (FALSE != ret ) {
			m_pProcessor->create_zmq_context(2, ret, m_server_ip, m_client->get_image_width() , m_client->get_image_height());
			m_pProcessor->set_send_frame_rate(snd_frame_rate, m_client->get_frame_rate());
			return	ret;
		}
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}
int32_t CAcquireStoreClient::stop() {
	try {
		m_client->stop();
		m_pProcessor->destroy_zmq_contex();
		return	TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}
int32_t	CAcquireStoreClient::set_snd_frame_resolution(const int32_t width, const int32_t height) {
	try {
		m_client->set_snd_frame_resolution(width, height);
		return	TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}
int32_t CAcquireStoreClient::set_snd_frame_rate(const int32_t snd_frame_rate) {
	try {
		m_client->set_snd_frame_rate(snd_frame_rate);
		return	TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}
int32_t CAcquireStoreClient::set_store_file(const int32_t flag, const std::string& file_name) {
	try {
		m_client->set_store_file(flag, file_name);
		return	TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}
int32_t CAcquireStoreClient::do_pause(const int32_t flag) {
	try {
		m_client->do_pause(flag);
		return	TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}
int32_t CAcquireStoreClient::forward_play(const int64_t frame_seq, const int32_t snd_frame_rate) {
	try {
		m_pProcessor->set_send_frame_rate(snd_frame_rate, m_client->get_frame_rate());
		m_pProcessor->m_recv_seq = m_pProcessor->m_last_recv_seq = 0;

		m_client->set_snd_frame_rate(snd_frame_rate);
		m_client->forward_play(frame_seq, snd_frame_rate);
		return	TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}
int32_t CAcquireStoreClient::backward_play(const int64_t frame_seq, const int32_t snd_frame_rate) {
	try {
		m_pProcessor->set_send_frame_rate(snd_frame_rate, m_client->get_frame_rate());
		m_client->set_snd_frame_rate(snd_frame_rate);
		m_client->backward_play(frame_seq, snd_frame_rate);
		return	TRUE;
	}
	catch (TException& e) {
		fprintf(stdout, "%s\n", e.what());
		return FALSE;
	}
}


int32_t CAcquireStoreClient::set_exposure_time(const double microseconds) {
	try {
		return m_client->set_exposure_time(microseconds);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CAcquireStoreClient::set_gain_by_sensor_all(const double gain) {
	try {
		return m_client->set_gain_by_sensor_all(gain);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}

int32_t CAcquireStoreClient::set_gain_by_sensor_analog(const double gain) {
	try {
		return m_client->set_gain_by_sensor_analog(gain);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CAcquireStoreClient::set_gain_by_sensor_digital(const double gain) {
	try {
		return m_client->set_gain_by_sensor_digital(gain);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CAcquireStoreClient::set_frame_rate(const double rate) {
	try {
		return m_client->set_frame_rate(rate);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CAcquireStoreClient::enable_turbo_transfer(const int32_t enabled) {
	try {
		return m_client->enable_turbo_transfer(enabled);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CAcquireStoreClient::set_pixel_fmt(const std::string& pixel_fmt) {
	try {
		return m_client->set_pixel_fmt(pixel_fmt);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CAcquireStoreClient::set_offset_x(const int32_t offset_x) {
	try {
		return m_client->set_offset_x(offset_x);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CAcquireStoreClient::set_offset_y(const int32_t offset_y) {
	try {
		return m_client->set_offset_y(offset_y);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}

int32_t CAcquireStoreClient::set_image_width(const int32_t width) {
	try {
		return m_client->set_image_width(width);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}

int32_t CAcquireStoreClient::set_image_height(const int32_t height) {
	try {
		return m_client->set_image_height(height);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}

double CAcquireStoreClient::get_exposure_time() {
	try {
		return m_client->get_exposure_time();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1.f;
	}
}

double CAcquireStoreClient::get_gain_by_sensor_all() {
	try {
		return m_client->get_gain_by_sensor_all();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1.f;
	}
}
double CAcquireStoreClient::get_gain_by_sensor_analog() {
	try {
		return m_client->get_gain_by_sensor_analog();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1.f;
	}
}

double CAcquireStoreClient::get_gain_by_sensor_digital() {
	try {
		return m_client->get_gain_by_sensor_digital();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1.f;
	}
}
double CAcquireStoreClient::get_frame_rate() {
	try {
		return m_client->get_frame_rate();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1.f;
	}
}

void CAcquireStoreClient::get_exposure_time_range(double* min, double* max) {
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
void CAcquireStoreClient::get_gain_range_by_sensor_all(double* min, double* max) {
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
void CAcquireStoreClient::get_gain_range_by_sensor_analog(double* min, double* max) {
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
void CAcquireStoreClient::get_gain_range_by_sensor_digital(double* min, double* max) {
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
void CAcquireStoreClient::get_frame_rate_range(double* min, double* max) {
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
int32_t CAcquireStoreClient::get_image_width() {
	try {
		return m_client->get_image_width();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CAcquireStoreClient::get_image_height() {
	try {
		return m_client->get_image_height();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CAcquireStoreClient::get_height_max() {
	try {
		return m_client->get_height_max();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CAcquireStoreClient::get_width_max() {
	try {
		return m_client->get_width_max();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CAcquireStoreClient::get_offset_x() {
	try {
		return m_client->get_offset_x();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CAcquireStoreClient::get_offset_y() {
	try {
		return m_client->get_offset_y();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
int32_t CAcquireStoreClient::is_enabled_turbo_transfer() {
	try {
		return m_client->is_enabled_turbo_transfer();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
}
void CAcquireStoreClient::get_pixel_fmt(std::string& pixel_fmt) {
	try {
		return m_client->get_pixel_fmt(pixel_fmt);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
	}
}
void CAcquireStoreClient::get_user_defined_name(std::string& name) {
	try {
		return m_client->get_user_defined_name(name);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
	}
}

void CAcquireStoreClient::get_device_serial_number(std::string& number) {
	try {
		return m_client->get_user_defined_name(number);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
	}
}
void CAcquireStoreClient::get_current_ip_address(std::string& ip) {
	try {
		return m_client->get_current_ip_address(ip);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
	}
}

double CAcquireStoreClient::get_grab_fps() {
	try {
		return m_client->get_grab_fps();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return .0f;
	}
}
double CAcquireStoreClient::get_process_fps() {
	try {
		return m_client->get_process_fps();
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return .0f;
	}
}
void CAcquireStoreClient::save_feature(std::string& content) {
	try {
		m_client->save_feature(content);
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
	}
}
int32_t CAcquireStoreClient::update_feature(const std::string& content) {
	try {
		m_client->update_feature(content);
		return TRUE;
	}
	catch (TException& e) {
		fprintf(stderr, "%s\n", e.what());
		return FALSE;
	}
}
