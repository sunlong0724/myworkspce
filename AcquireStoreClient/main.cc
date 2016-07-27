
#include "AcquireStoreClient.h"
int main(int argc, char** argv) {


	CAcquireStoreClient client;
	std::vector<std::string> ips = client.scan_ip(std::string("192.168.0.18"), std::string("192.168.0.19"));

	for (auto& a : ips) {
		if (FALSE == client.connect(a, 9070)) {
			printf("CAcquireStoreClient connect failed!\n");
			exit(0);
		}

		int snd_frame_rate = 25;
		int64_t frame_no = 0;
		
		client.init();
		std::string name;
		client.get_user_defined_name(name);
		client.set_store_file(1, name);
		client.set_snd_frame_resolution(320, 240);

		std::vector<char> buffer(client.get_image_width() * client.get_image_height() + sizeof int64_t + sizeof int64_t, 0x00);
		int recv_count = 0;

		client.start(snd_frame_rate);
		while (true) {
			client.m_pProcessor->recv_data(buffer.data(), buffer.size());
			memcpy(&frame_no, &buffer[FRAME_SEQ_START], sizeof int64_t);
			//fprintf(stdout, "1 recv frame no:%lld\n", frame_no);

			::Sleep(10);
			continue;

			++recv_count;
			if (recv_count > snd_frame_rate * 15) {
				//snd_frame_rate = client.get_frame_rate();
				client.do_pause(1);
				client.backward_play(frame_no, snd_frame_rate);
				recv_count = 0;
				while (true) {
					memset(buffer.data(), 0x00, buffer.size());
					client.m_pProcessor->recv_data(buffer.data(), buffer.size());
					memcpy(&frame_no, &buffer[FRAME_SEQ_START], sizeof int64_t);
					//fprintf(stdout, "2 recv frame no:%lld\n", frame_no);
					
					if (frame_no <= 20) {
						client.do_pause(1);
						client.forward_play(frame_no, snd_frame_rate);
						recv_count = 0;
						while (true) {
							memset(buffer.data(), 0x00, buffer.size());
							client.m_pProcessor->recv_data(buffer.data(), buffer.size());
							memcpy(&frame_no, &buffer[FRAME_SEQ_START], sizeof int64_t);

							if (++recv_count > snd_frame_rate * 15) {
								client.do_pause(0);//FIXME:
								break;
							}
						}
					}
				}
			}
		}

		client.stop();
	}

	return 0;
}