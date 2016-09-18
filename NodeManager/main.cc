
#include <stdint.h>
#include <string>

#include <map>
#include <thread>
#include <signal.h>
#include "AgentClient.h"

#include "utils.h"

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

int64_t g_process_id = 0;
void connected_callback(ConnectStatus status, void* ctx) {
	if (status == ConnectStatus_CONNECTED) {
		CAgentClient *ac = (CAgentClient*)ctx;
		std::map<std::string, std::map<int, std::string>>  cameras;
		ac->find_cameras(cameras);
		printf_cameras(cameras);

		char cmdline[1024];
		sprintf(cmdline, "AcquireStore 9098 55598 192.168.0.98 10 d:\\data.raw");
		printf("%s\n", cmdline);
		g_process_id = ac->exec_program(cmdline);

	}
	else if (status == ConnectStatus_DISCONNECT) {
		int a = 0;
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

	CAgentClient ac;

	ac.set_connected_callback(connected_callback, &ac);
	ac.set_connect_parameters("192.168.1.99", port);
	ac.start();

	sleep(1000*5);
	ac.kill_program(g_process_id);

	while (1) {
		sleep(1);
	}

	ac.stop();
	return 0;
}