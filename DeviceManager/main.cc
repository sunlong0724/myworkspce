
#include <stdint.h>
#include <string>

#include <thread>
#include <signal.h>

#include "AgentClient.h"
#include "ZmqTransportDataImpl.h"
#include "ping.h"

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
	signal(SIGINT, sig_cb);  /*注册ctrl+c信号捕获函数*/

	int display_rate = 60;

	uint16_t    port(9090);

	if (argc > 1) {
		display_rate = atoi(argv[1]);
	}

	char camera_name[] = "S1100466";
	int ret = 0;
	std::string str_val;
	int	   val_int = 0;
	double val = .0f;
	double min = .0f;
	double max = .0f;
	std::vector<std::string> cameras;
	uint16_t data_port = 0;
	printf("===================Camera Manager Test!!!=================================\n");

	CPing objPing;
	PingReply reply;
	std::string DestIP("192.168");
	std::vector<std::string> ips;
	for (int i = 1; i < 2; ++i) {
		for (int j = 11; j < 50; ++j) {
			std::string tmp = DestIP;
			tmp.append("." + std::to_string(i) + "." + std::to_string(j));

			if (objPing.Ping((char*)tmp.c_str(), &reply, 10)) {
				printf("Reply from %s: bytes=%ld time=%ldms TTL=%ld\n", tmp.c_str(), reply.m_dwBytes, reply.m_dwRoundTripTime, reply.m_dwTTL);
				ips.push_back(tmp);
			}
			else {
				printf("%s 请求超时。\n", tmp.c_str());
			}
		}
	}

	for (auto& ip : ips) {
		AgentClient ac;
		if (FALSE == ac.connect(ip, port)) {
			fprintf(stderr, "%s connect server %s:%d failed!\n", __FUNCTION__, ip.c_str(), port);
			return -1;
		}
		else {
			fprintf(stderr, "%s connect server %s:%d success!\n", __FUNCTION__, ip.c_str(), port);
		}

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

		data_port = ac.open_camera(camera_name);
		printf("open camera %s ret:%d\n", camera_name, data_port);

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

		printf("===================Set property Test!!!=================================\n");
		//set
		//ret = ac.set_exposure_time(camera_name, 1000.f);
		//printf("set_exposure_time %s ret:%d\n", camera_name, ret);

		//ret = ac.set_frame_rate(camera_name, 180.f);
		//printf("set_frame_rate %s ret:%d\n", camera_name, ret);

		//ret = ac.set_gain_by_sensor_analog(camera_name, 1.f);
		//printf("set_gain_by_sensor_analog %s ret:%d\n", camera_name, ret);

		//ret = ac.set_gain_by_sensor_digital(camera_name, 1.f);
		//printf("set_gain_by_sensor_digital %s ret:%d\n", camera_name, ret);

		//ret = ac.set_image_width(camera_name, 1280);
		//printf("set_image_width %s ret:%d\n", camera_name, ret);

		//ret = ac.set_image_height(camera_name, 720);
		//printf("set_image_height %s ret:%d\n", camera_name, ret);

		//ret = ac.set_offset_x(camera_name, 0);
		//printf("set_offset_x %s ret:%d\n", camera_name, ret);

		//ret = ac.set_offset_y(camera_name, 152);
		//printf("set_offset_y %s ret:%d\n", camera_name, ret);


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

		//ac.save_feature(camera_name, str_val);
		//printf("save_feature %s ret:%s\n", camera_name, str_val.c_str());

		//ret = ac.update_feature(camera_name, str_val);
		//printf("update_feature %s ret:%d\n", camera_name, ret);

		
		CustomData* p = ac.m_custom_data;


		//ac.set_display_frame_rate(camera_name, display_rate, ac.get_frame_rate(camera_name));

		ac.start_camera(camera_name, display_rate);

		while (g_running_flag) {
			//std::this_thread::sleep_for(std::chrono::milliseconds(500));
			recv_data(p);
		}
		printf("%s 1\n", __FUNCTION__);
		ac.stop_camera(camera_name);
		printf("%s 2\n", __FUNCTION__);
		ac.close_camera(camera_name);
		printf("%s 3\n", __FUNCTION__);
		ac.close();
		printf("%s 4\n", __FUNCTION__);

	}

	return 0;
}