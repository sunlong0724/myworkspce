
#include <stdint.h>
#include <string>

#include <thread>
#include <signal.h>

#include "AgentClient.h"

void sleep(int32_t milliseconds) {
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

bool g_running_flag = true;
void sig_cb(int sig)
{
	if (sig == SIGINT) {
		fprintf(stdout, "%s\n", __FUNCTION__);
		g_running_flag = false;
	}
}

void printf_vector(std::vector<std::string> vec	) {
	for (auto& a : vec) {
		printf("%s\n ", a.c_str());
	}
}

int main(int argc, char** argv) {
	signal(SIGINT, sig_cb);  /*×¢²áctrl+cÐÅºÅ²¶»ñº¯Êý*/

	std::string ip("127.0.0.1");
	uint16_t    port(9090);

	if (argc > 1) {
		ip.assign(argv[1]);
	}
	if (argc > 2) {
		port = atoi(argv[2]);
	}

	char camera_name[] = "S1100466";
	int ret = 0;
	std::string str_val;
	int	   val_int = 0;
	double val = .0f;
	double min = .0f;
	double max = .0f;
	std::vector<std::string> cameras;
	AgentClient ac;

	printf("===================Camera Manager Test!!!=================================\n");


	ac.connect(ip, port);
	ac.find_cameras(cameras);
	//S1100466
	ret = ac.add_cameras(cameras);
	printf_vector(cameras);

	ret = ac.get_hold_cameras(cameras);
	printf_vector(cameras);

	ret = ac.del_cameras(cameras);
	printf_vector(cameras);

	ret = ac.get_hold_cameras(cameras);
	printf_vector(cameras);

	//find again!!!
	ac.find_cameras(cameras);
	ret = ac.add_cameras(cameras);
	printf_vector(cameras);

	ret = ac.get_hold_cameras(cameras);
	printf_vector(cameras);

	ret = ac.open_camera(camera_name);
	printf("open camera %s ret:%d\n", camera_name, ret);

	printf("===================Get property Test!!!=================================\n");

	val = ac.get_exposure_time(camera_name);
	printf("get_exposure_time %s ret:%f\n", camera_name, val);

	val = ac.get_frame_rate(camera_name);
	printf("get_frame_rate %s ret:%f\n", camera_name, val);

	//1280 no gain_by_sensor_all
	//val = ac.get_gain_by_sensor_all(camera_name);
	//printf("get_gain_by_sensor_all %s ret:%f\n", camera_name, val);

	val = ac.get_gain_by_sensor_analog(camera_name);
	printf("get_gain_by_sensor_analog %s ret:%f\n", camera_name, val);

	val = ac.get_gain_by_sensor_digital(camera_name);
	printf("get_gain_by_sensor_digital %s ret:%f\n", camera_name, val);

	ac.get_exposure_time_range(camera_name, &min, &max);
	printf("get_exposure_time_range %s min:%f max:%f\n", camera_name, min, max);

	ac.get_frame_rate_range(camera_name, &min, &max);
	printf("get_frame_rate_range %s min:%f max:%f\n", camera_name, min, max);

	//1280 no gain_by_sensor_all
	//ac.get_gain_range_by_sensor_all(camera_name, &min, &max);
	//printf("get_gain_range_by_sensor_all %s min:%f max:%f\n", camera_name, min, max);

	ac. get_gain_range_by_sensor_analog(camera_name, &min, &max);
	printf("get_gain_range_by_sensor_analog %s min:%f max:%f\n", camera_name, min, max);

	ac. get_gain_range_by_sensor_digital(camera_name, &min, &max);
	printf("get_gain_range_by_sensor_digital %s min:%f max:%f\n", camera_name, min, max);

	val_int =ac. get_height_max(camera_name);
	printf("get_height_max %s ret:%d\n", camera_name, val_int);

	val_int =ac. get_width_max(camera_name);
	printf("get_width_max %s ret:%d\n", camera_name, val_int);

	val_int = ac.get_image_width(camera_name);
	printf("get_image_width %s ret:%d\n", camera_name, val_int);

	val_int = ac.get_image_height(camera_name);
	printf("get_image_height %s ret:%d\n", camera_name, val_int);

	val_int = ac. get_offset_x(camera_name);
	printf("get_offset_x %s ret:%d\n", camera_name, val_int);

	val_int = ac. get_offset_y(camera_name);
	printf("get_offset_y %s ret:%d\n", camera_name, val_int);

	val_int = ac. is_enabled_turbo_transfer(camera_name);
	printf("is_enabled_turbo_transfer %s ret:%d\n", camera_name, val_int);

	ac.get_pixel_fmt(camera_name, str_val);
	printf("get_pixel_fmt %s ret:%s\n", camera_name, str_val.c_str());

	ac.get_device_serial_number(camera_name, str_val);
	printf("get_device_serial_number %s ret:%s\n", camera_name, str_val.c_str());

	ac.get_current_ip_address(camera_name, str_val);
	printf("get_current_ip_address %s ret:%s\n", camera_name, str_val.c_str());

	printf("===================Set property Test!!!=================================\n");
	//set
	ret = ac.set_exposure_time(camera_name, 1000.f);
	printf("set_exposure_time %s ret:%d\n", camera_name, ret);

	ret = ac.set_frame_rate(camera_name, 180.f);
	printf("set_frame_rate %s ret:%d\n", camera_name, ret);

	ret = ac.set_gain_by_sensor_analog(camera_name, 8.f);
	printf("set_gain_by_sensor_analog %s ret:%d\n", camera_name, ret);

	ret = ac.set_gain_by_sensor_digital(camera_name, 1.f);
	printf("set_gain_by_sensor_digital %s ret:%d\n", camera_name, ret);

	ret = ac.set_image_width(camera_name, 1280);
	printf("set_image_width %s ret:%d\n", camera_name, ret);

	ret = ac.set_image_height(camera_name, 720);
	printf("set_image_height %s ret:%d\n", camera_name, ret);

	ret = ac.set_offset_x(camera_name, 0);
	printf("set_offset_x %s ret:%d\n", camera_name, ret);

	ret = ac.set_offset_y(camera_name, 152);
	printf("set_offset_y %s ret:%d\n", camera_name, ret);


	printf("===================Get property Test Again!!!=================================\n");

	val = ac.get_exposure_time(camera_name);
	printf("get_exposure_time %s ret:%f\n", camera_name, val);

	val = ac.get_frame_rate(camera_name);
	printf("get_frame_rate %s ret:%f\n", camera_name, val);

	//1280 no gain_by_sensor_all
	//val = ac.get_gain_by_sensor_all(camera_name);
	//printf("get_gain_by_sensor_all %s ret:%f\n", camera_name, val);

	val = ac.get_gain_by_sensor_analog(camera_name);
	printf("get_gain_by_sensor_analog %s ret:%f\n", camera_name, val);

	val = ac.get_gain_by_sensor_digital(camera_name);
	printf("get_gain_by_sensor_digital %s ret:%f\n", camera_name, val);

	ac.get_exposure_time_range(camera_name, &min, &max);
	printf("get_exposure_time_range %s min:%f max:%f\n", camera_name, min, max);

	ac.get_frame_rate_range(camera_name, &min, &max);
	printf("get_frame_rate_range %s min:%f max:%f\n", camera_name, min, max);

	//1280 no gain_by_sensor_all
	//ac.get_gain_range_by_sensor_all(camera_name, &min, &max);
	//printf("get_gain_range_by_sensor_all %s min:%f max:%f\n", camera_name, min, max);

	ac.get_gain_range_by_sensor_analog(camera_name, &min, &max);
	printf("get_gain_range_by_sensor_analog %s min:%f max:%f\n", camera_name, min, max);

	ac.get_gain_range_by_sensor_digital(camera_name, &min, &max);
	printf("get_gain_range_by_sensor_digital %s min:%f max:%f\n", camera_name, min, max);

	val_int = ac.get_height_max(camera_name);
	printf("get_height_max %s ret:%d\n", camera_name, val_int);

	val_int = ac.get_width_max(camera_name);
	printf("get_width_max %s ret:%d\n", camera_name, val_int);

	val_int = ac.get_image_width(camera_name);
	printf("get_image_width %s ret:%d\n", camera_name, val_int);

	val_int = ac.get_image_height(camera_name);
	printf("get_image_height %s ret:%d\n", camera_name, val_int);

	val_int = ac.get_offset_x(camera_name);
	printf("get_offset_x %s ret:%d\n", camera_name, val_int);

	val_int = ac.get_offset_y(camera_name);
	printf("get_offset_y %s ret:%d\n", camera_name, val_int);

	val_int = ac.is_enabled_turbo_transfer(camera_name);
	printf("is_enabled_turbo_transfer %s ret:%d\n", camera_name, val_int);

	ac.get_pixel_fmt(camera_name, str_val);
	printf("get_pixel_fmt %s ret:%s\n", camera_name, str_val.c_str());

	ac.get_device_serial_number(camera_name, str_val);
	printf("get_device_serial_number %s ret:%s\n", camera_name, str_val.c_str());

	ac.get_current_ip_address(camera_name, str_val);
	printf("get_current_ip_address %s ret:%s\n", camera_name, str_val.c_str());


	//void get_user_defined_name(const std::string& camera_name, std::string& name);

		/*
		BOOL find_cameras(std::vector<std::string> & cameras);
		BOOL	get_hold_cameras(std::vector<std::string> & cameras);
		int32_t add_cameras(const std::vector<std::string> & l);
		int32_t del_cameras(const std::vector<std::string> & l);
		int32_t open_camera(const std::string& camera_name);
		int32_t close_camera(const std::string& camera_name);
		int32_t start_camera(const std::string& camera_name);
		int32_t stop_camera(const std::string& camera_name);
		int32_t set_exposure_time(const std::string& camera_name, const double microseconds);
		int32_t set_gain_by_sensor_all(const std::string& camera_name, const double gain);
		int32_t set_gain_by_sensor_analog(const std::string& camera_name, const double gain);
		int32_t set_gain_by_sensor_digital(const std::string& camera_name, const double gain);
		int32_t set_frame_rate(const std::string& camera_name, const double rate);
		int32_t enable_turbo_transfer(const std::string& camera_name, const int32_t enabled);
		int32_t set_pixel_fmt(const std::string& camera_name, const std::string& pixel_fmt);
		int32_t set_offset_x(const std::string& camera_name, const int32_t offset_x);
		int32_t set_offset_y(const std::string& camera_name, const int32_t offset_y);
		int32_t set_image_width(const std::string& camera_name, const int32_t width);
		int32_t set_image_height(const std::string& camera_name, const int32_t height);
		double get_exposure_time(const std::string& camera_name);
		double get_gain_by_sensor_all(const std::string& camera_name);
		double get_gain_by_sensor_analog(const std::string& camera_name);
		double get_gain_by_sensor_digital(const std::string& camera_name);
		double get_frame_rate(const std::string& camera_name);
		void get_exposure_time_range(const std::string& camera_name, double* min, double* max);
		void get_gain_range_by_sensor_all(const std::string& camera_name, double* min, double* max);
		void get_gain_range_by_sensor_analog(const std::string& camera_name, double* min, double* max);
		void get_gain_range_by_sensor_digital(const std::string& camera_name, double* min, double* max);
		void get_frame_rate_range(const std::string& camera_name, double* min, double* max);
		int32_t get_height_max(const std::string& camera_name);
		int32_t get_width_max(const std::string& camera_name);
		int32_t get_offset_x(const std::string& camera_name);
		int32_t get_offset_y(const std::string& camera_name);
		int32_t is_enabled_turbo_transfer(const std::string& camera_name);
		void get_pixel_fmt(const std::string& camera_name, std::string& pixel_fmt);
		void get_user_defined_name(const std::string& camera_name, std::string& name);
		double get_grab_fps(const std::string& camera_name);
		double get_process_fps(const std::string& camera_name);
		*/




		ac.close_camera(camera_name);
		ac.close();


	//while (g_running_flag) {
	//	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	//}
	return 0;
}