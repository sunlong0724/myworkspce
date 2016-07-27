
#include <stdint.h>
#include <string>

#include <map>
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

void printf_cameras(std::map<std::string, std::map<int, std::string>>& cs) {
	for (auto& a : cs) {
		printf("%s:%d,%s\n ", a.first.c_str(), a.second.begin()->first, a.second.begin()->second.c_str());
	}
}

int main(int argc, char** argv) {
	signal(SIGINT, sig_cb);  /*×¢²áctrl+cÐÅºÅ²¶»ñº¯Êý*/

	int display_rate = 60;
	uint16_t    port(9090);

	std::map<std::string, std::map<int ,std::string>>  cameras;

	if (argc > 1) {
		display_rate = atoi(argv[1]);
	}

	std::vector<std::string> ips;
	CAgentClient ac;
	ips = ac.scan_ip(std::string("192.168.0.10"), std::string("192.168.0.19"));

	for (auto& ip : ips) {
		
		ac.connect(ip, port);

		ac.find_cameras(cameras);
		printf_cameras(cameras);

		char cmdline[1024];
		//.\AcquireStore.exe 9070 Nano-C1280_1 0 55555
		sprintf(cmdline, ".\\AcquireStore.exe 9070 %s %d 55555", cameras.begin()->second.begin()->second.c_str(), cameras.begin()->second.begin()->first);
		printf("%s\n", cmdline);
		int32_t process_id = ac.exec_program(cmdline);



		//Sleep(5000);

		//ac.kill_program(process_id);
		//ac.close();
	}

	return 0;
}