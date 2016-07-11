
#include "AcquireStoreClient.h"
int main(int argc, char** argv) {


	CAcquireStoreClient client;
	std::vector<std::string> ips = client.scan_ip(std::string("192.168.1.10"), std::string("192.168.1.19"));

	for (auto& a : ips) {
		client.connect(a, 9090);
		client.start(30);

		while (true) {
			client.m_pProcessor->recv_data();
		}

		client.stop();
	}

	return 0;
}