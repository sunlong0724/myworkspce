
#include "PlaybackCtrlClient.h"
#include "defs.h"

int64_t g_recv_frame_no;
int recv_data_cb(unsigned char* data, int data_len, void* ctx) {
	memcpy(&g_recv_frame_no, &data[FRAME_SEQ_START], sizeof int64_t);
	//fprintf(stdout, "%s recv frame no:%lld\n", __FUNCTION__,frame_no);

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

		int frames_playback = 3000;
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

	

		printf("################begin start_play_live#########################3\n");

	again:
		uint16_t port = client.start_play_live(play_frame_rate);
		client.m_recv_thread.set_parameters(a, port, GET_IMAGE_BUFFER_SIZE(width, height), (full_frame_rate+1)/play_frame_rate);

		client.m_recv_thread.set_sink_data_callback(recv_data_cb, NULL);
		client.m_recv_thread.start();
		client.play_live();

		int64_t now = time(NULL);

		while (true) {
			::Sleep(1000);
			if (time(NULL) - now > 60 * 0.5)
				break;
			continue;
		}

		client.stop_play_live();

		//printf("\n################begin backward_play#########################\n");

		//client.m_recv_thread.set_parameters(a, port, GET_IMAGE_BUFFER_SIZE(width, height), (full_frame_rate + 1) / full_frame_rate);
		//client.start_backward_play(g_recv_frame_no-2, full_frame_rate, frames_playback);
		//int64_t expect_no = g_recv_frame_no - frames_playback;
		//client.backward_play();

		//now = time(NULL);
		//while (true) {
		//	::Sleep(1000);
		//	if (g_recv_frame_no <= expect_no)
		//		break;
		//	continue;
		//}

		//client.stop_backward_play();

		//printf("\n################begin forward_play#########################\n");

		//client.m_recv_thread.set_parameters(a, port, GET_IMAGE_BUFFER_SIZE(width, height), (full_frame_rate + 1) / full_frame_rate);
		//client.start_forward_play(g_recv_frame_no - 2, full_frame_rate, frames_playback);
		//expect_no = g_recv_frame_no + frames_playback - 500;
		//client.forward_play();
		//now = time(NULL);
		//while (true) {
		//	::Sleep(1000);
		//	if (g_recv_frame_no >= expect_no)
		//		break;
		//	continue;
		//}

		//client.stop_forward_play();



		printf("\n################begin start_play_live && backward_play_temp #########################\n");

		port = client.start_play_live(play_frame_rate);
		client.m_recv_thread.set_parameters(a, port, GET_IMAGE_BUFFER_SIZE(width, height), (full_frame_rate + 1) / play_frame_rate);
		client.play_live();


		client.start_backward_play_temp(g_recv_frame_no - 2, full_frame_rate, frames_playback);
		client.m_recv_thread.set_parameters(a, port, GET_IMAGE_BUFFER_SIZE(width, height), (full_frame_rate + 1) / full_frame_rate);
		client.backward_play_temp();
		now = time(NULL);

		while (true) {
			::Sleep(1000);
			if (time(NULL) - now > 60 * 1.5)
				break;
			continue;
		}

		client.stop_backward_play_temp();
		client.stop_play_live();

		goto again;

		client.m_recv_thread.stop();
	}

	return 0;
}