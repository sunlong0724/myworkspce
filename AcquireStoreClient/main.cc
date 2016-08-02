
#include "PlaybackCtrlClient.h"
#include "defs.h"

#include <opencv2\opencv.hpp>

int64_t				g_recv_frame_no;
IplImage*			g_bayer_image = NULL;
IplImage*			g_rgb_image = NULL;
int64_t				g_width;
int64_t				g_height;
int recv_data_cb(unsigned char* data, int data_len, void* ctx) {
	memcpy(&g_recv_frame_no, &data[FRAME_SEQ_START], sizeof int64_t);
	//fprintf(stdout, "%s recv frame no:%lld\n", __FUNCTION__,frame_no);
	if (NULL == g_bayer_image ) {
		g_bayer_image = cvCreateImageHeader(cvSize(g_width, g_height), 8, 1);
		cvSetData(g_bayer_image, &data[FRAME_DATA_START], g_width);

		g_rgb_image = cvCreateImage(cvSize(g_width, g_height), 8, 3);
	}

	if (g_bayer_image->imageSize != data_len- FRAME_DATA_START) {
		//cvReleaseImage(&g_bayer_image);
		//cvReleaseImage(&g_rgb_image);

		g_bayer_image = cvCreateImageHeader(cvSize(g_width, g_height), 8, 1);
		cvSetData(g_bayer_image, &data[FRAME_DATA_START], g_width);

		g_rgb_image = cvCreateImage(cvSize(g_width, g_height), 8, 3);
	}

	cvCvtColor(g_bayer_image, g_rgb_image, CV_BayerRG2RGB);
	cvShowImage("mywin", g_rgb_image);
	cvWaitKey(1);
	return 0;
}

int main(int argc, char** argv) {

	CPlaybackCtrlClient client;
	std::vector<std::string> ips = client.scan_ip(std::string("192.168.0.11"), std::string("192.168.0.19"));

	for (auto& a : ips) {
		if (FALSE == client.connect(a, 9070)) {
			printf("CAcquireStoreClient connect failed!\n");
			exit(0);
		}
		int i = 0;
	again:
		int frames_playback = 3000;
		int play_frame_rate = 30;
		int full_frame_rate = client.get_frame_rate();
		int sample = (full_frame_rate + 1) / play_frame_rate;
		int64_t frame_no = 0;
		int width = g_width = client.get_image_width();
		int height = g_height = client.get_image_height();
		
		std::string name;
		client.get_user_defined_name(name);

	
		client.set_store_file(1, name);


	
		//client.set_play_frame_rate(play_frame_rate, sample);
		client.set_play_frame_resolution(g_width, g_height);
		//g_width = width / 4;
		//g_height = height / 4;

		printf("################begin start_play_live#########################3\n");
		client.start_play_live(play_frame_rate, sample);

		
		client.set_recv_image_parameters(GET_IMAGE_BUFFER_SIZE(g_width, g_height), (full_frame_rate+1)/play_frame_rate);
		client.set_recv_sink_callback(recv_data_cb, &client);
		client.play_live();

		int64_t now = time(NULL);

		while (true) {
			::Sleep(1000);
			if (time(NULL) - now > 60 * 1.5)
				break;
			printf("fps: cam(%f), soft_grab(%f), soft_snd(%f)\n", client.get_camera__grab_fps(), client.get_soft_grab_fps(), client.get_soft_snd_fps());
			continue;
		}

		//client.stop_play_live();

		//g_width = width / 4;
		//g_height = height / 4;
		//client.set_play_frame_resolution(g_width, g_height);

		//goto again;



		printf("\n################begin backward_play temp#########################\n");

		++i;
		if (i == 1) {
			sample = 12;
			play_frame_rate = 15;
			g_width = 1280;
			g_height = 720;

		}
		else if (i == 2) {
			sample = 18;
			play_frame_rate = 10;
			g_width = 1280;
			g_height = 720;
		}
		else if (i == 3) {
			sample = 18;
			play_frame_rate = 5;
			g_width = 1280;
			g_height = 720;
			i = 0;
		}

		client.set_recv_image_parameters( GET_IMAGE_BUFFER_SIZE(width, height), sample);
		client.start_backward_play_temp( play_frame_rate, sample);
		client.backward_play_temp();

		now = time(NULL);
		while (true) {
			::Sleep(1000);
			if (time(NULL) - now > 30 * 1)
				break;

			printf("fps: cam(%f), soft_grab(%f), soft_snd(%f)\n", client.get_camera__grab_fps(), client.get_soft_grab_fps(), client.get_soft_snd_fps());
			continue;
		}

		client.stop_backward_play_temp();


		client.start_forward_play_temp(play_frame_rate, sample);
		client.forward_play_temp();

		now = time(NULL);
		while (true) {
			::Sleep(1000);
			if (time(NULL) - now > 30 * 1)
				break;
			continue;
		}

		client.stop_forward_play_temp();

	

		goto again;

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

		client.start_play_live(play_frame_rate,sample);
		client.set_recv_image_parameters( GET_IMAGE_BUFFER_SIZE(width, height), (full_frame_rate + 1) / play_frame_rate);
		client.play_live();


		client.start_backward_play_temp(full_frame_rate, sample);
		client.set_recv_image_parameters(GET_IMAGE_BUFFER_SIZE(width, height), (full_frame_rate + 1) / full_frame_rate);
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

		client.close();
	}

	return 0;
}