
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

void printf_cameras(std::map<std::string, std::map<int, std::string>>& cs) {
	for (auto& a : cs) {
		printf("%s:%d,%s\n ", a.first.c_str(), a.second.begin()->first, a.second.begin()->second.c_str());
	}
}

//can scan 192.168.0.1~192.168.1.255 eg.
std::vector<std::string> scan_ip(std::string& start_ip, std::string& end_ip) {

	std::vector<std::string> result;

	CPing objPing;
	PingReply reply;

	size_t third_dot_pos = start_ip.rfind(".");
	size_t second_dot_pos = start_ip.rfind(".", third_dot_pos-1);

	int8_t fourth_start = ::atoi(start_ip.substr(third_dot_pos+1).c_str());
	int8_t third_start = ::atoi(start_ip.substr(second_dot_pos+1, third_dot_pos).c_str());

	third_dot_pos = end_ip.rfind(".");
	second_dot_pos = end_ip.rfind(".", third_dot_pos-1);

	int8_t fourth_end = ::atoi(end_ip.substr(third_dot_pos+1).c_str());
	int8_t third_end = ::atoi(end_ip.substr(second_dot_pos+1, third_dot_pos).c_str());

	std::string ip_sub( start_ip.substr(0, second_dot_pos));
	for (int i = third_start; i <= third_end; ++i) {
		for (int j = fourth_start; j <= fourth_end; ++j) {
			std::string tmp = ip_sub;
			tmp.append("." + std::to_string(i) + "." + std::to_string(j));

			if (objPing.Ping((char*)tmp.c_str(), &reply, 15)) {//5ms
				printf("Reply from %s: bytes=%ld time=%ldms TTL=%ld\n", tmp.c_str(), reply.m_dwBytes, reply.m_dwRoundTripTime, reply.m_dwTTL);
				result.push_back(tmp);
			}
			else {
				printf("%s 请求超时。\n", tmp.c_str());
			}
		}
	}
	return result;
}

int main(int argc, char** argv) {
	signal(SIGINT, sig_cb);  /*注册ctrl+c信号捕获函数*/

	int display_rate = 60;
	uint16_t    port(9090);

	std::map<std::string, std::map<int, std::string>>  cameras;

	if (argc > 1) {
		display_rate = atoi(argv[1]);
	}

	std::vector<std::string> ips;
	ips = scan_ip(std::string("192.168.1.10"), std::string("192.168.1.19"));

	for (auto& ip : ips) {
		AgentClient ac;
		ac.connect(ip, port);

		ac.find_cameras(cameras);
		printf_cameras(cameras);

		char cmdline[1024];
		sprintf(cmdline, ".\\AcquireStore.exe 9070 %s %d 55555", cameras.begin()->second.begin()->second.c_str(), cameras.begin()->second.begin()->first);
		int32_t process_id = ac.exec_acquire_store(cmdline);

		Sleep(5000);

		ac.kill_acquire_store(process_id);
		ac.close();
	}

	return 0;
}