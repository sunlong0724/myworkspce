
#include "PlaybackCtrlClient.h"
#include "defs.h"


int recv_data_cb(unsigned char* data, int data_len, void* ctx) {
	int64_t frame_no;
	memcpy(&frame_no, &data[FRAME_SEQ_START], sizeof int64_t);
	fprintf(stdout, "%s recv frame no:%lld\n", __FUNCTION__,frame_no);

	return 0;
}

int main(int argc, char** argv) {

	CPlaybackCtrlClient client;
	std::vector<std::string> ips = client.scan_ip(std::string("192.168.0.18"), std::string("192.168.0.19"));

	for (auto& a : ips) {
		if (FALSE == client.connect(a, 9070)) {
			printf("CAcquireStoreClient connect failed!\n");
			exit(0);
		}

		int play_frame_rate = 30;
		int full_frame_rate = client.get_frame_rate();
		int64_t frame_no = 0;
		int64_t width = client.get_image_width();
		int64_t height = client.get_image_height();
		
		std::string name;
		client.get_user_defined_name(name);

		client.set_store_file(1, name);
		client.set_play_frame_resolution(width, height);
		client.set_play_frame_rate(play_frame_rate);

		uint16_t port = client.start_play_live();

		client.m_recv_thread.init(a, port, GET_IMAGE_BUFFER_SIZE(width, height), (full_frame_rate+1)/play_frame_rate);
		client.m_recv_thread.set_sink_data_callback(recv_data_cb, NULL);
		client.m_recv_thread.start();

		while (true) {
			::Sleep(10);
			continue;
		}
		client.m_recv_thread.stop();
		client.stop_play_live();
	}

	return 0;
}