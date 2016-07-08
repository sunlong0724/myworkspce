#ifndef __MY_UTILS_H__
#define __MY_UTILS_H__

#include <vector>
#include <string>
#include <chrono>
#include "ping/Ping.h"

inline int64_t get_current_time_in_ms() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

std::vector<std::string> scan_ip(std::string& start_ip, std::string& end_ip) {

	std::vector<std::string> result;

	CPing objPing;
	PingReply reply;

	size_t third_dot_pos = start_ip.rfind(".");
	size_t second_dot_pos = start_ip.rfind(".", third_dot_pos - 1);

	int8_t fourth_start = ::atoi(start_ip.substr(third_dot_pos + 1).c_str());
	int8_t third_start = ::atoi(start_ip.substr(second_dot_pos + 1, third_dot_pos).c_str());

	third_dot_pos = end_ip.rfind(".");
	second_dot_pos = end_ip.rfind(".", third_dot_pos - 1);

	int8_t fourth_end = ::atoi(end_ip.substr(third_dot_pos + 1).c_str());
	int8_t third_end = ::atoi(end_ip.substr(second_dot_pos + 1, third_dot_pos).c_str());

	std::string ip_sub(start_ip.substr(0, second_dot_pos));
	for (int i = third_start; i <= third_end; ++i) {
		for (int j = fourth_start; j <= fourth_end; ++j) {
			std::string tmp = ip_sub;
			tmp.append("." + std::to_string(i) + "." + std::to_string(j));

			if (objPing.Ping((char*)tmp.c_str(), &reply, 15)) {//15ms
				printf("Reply from %s: bytes=%ld time=%ldms TTL=%ld\n", tmp.c_str(), reply.m_dwBytes, reply.m_dwRoundTripTime, reply.m_dwTTL);
				result.push_back(tmp);
			}
			else {
				printf("%s ÇëÇó³¬Ê±¡£\n", tmp.c_str());
			}
		}
	}
	return result;
}


#endif // !__MY_UTILS_H__
