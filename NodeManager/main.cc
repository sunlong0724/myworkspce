
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


void connected_callback(ConnectStatus status, void* ctx) {
	if (status == ConnectStatus_CONNECTED) {
		CAgentClient *ac = (CAgentClient*)ctx;
		std::map<std::string, std::map<int, std::string>>  cameras;
		ac->find_cameras(cameras);
		printf_cameras(cameras);

		char cmdline[1024];
		sprintf(cmdline, ".\\AcquireStore.exe 9070 55555 %s 10 e:\\data.raw", cameras.begin()->first.c_str());
		printf("%s\n", cmdline);
		int32_t process_id = ac->exec_program(cmdline);

	}
	else if (status == ConnectStatus_DISCONNECT) {
		int a = 0;
	}
}

int main(int argc, char** argv) {
	signal(SIGINT, sig_cb);  /*ע��ctrl+c�źŲ�����*/

	int display_rate = 60;
	uint16_t    port(9090);

	std::map<std::string, std::map<int ,std::string>>  cameras;

	if (argc > 1) {
		display_rate = atoi(argv[1]);
	}

	std::vector<std::string> ips;
	CAgentClient ac;
	ips = ac.scan_ip(std::string("192.168.1.18"), std::string("192.168.1.19"));


	ac.set_connected_callback(connected_callback, &ac);
	ac.set_connect_parameters(ips[0], port);
	ac.start();

	while (1) {
		sleep(1);
	}

	ac.stop();
	return 0;
}