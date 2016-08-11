
#include "PlaybackCtrlClient.h"
#include "defs.h"
#include "utils.h"

#include <opencv2\opencv.hpp>

int64_t				g_recv_frame_no;
IplImage*			g_bayer_image = NULL;
IplImage*			g_rgb_image = NULL;
int64_t				g_width;
int64_t				g_height;

int64_t				g_frame_a = 0;
int64_t				g_frame_b = 0;
BOOL				g_mode_ab_for_a_pressed = FALSE;
BOOL				g_mode_ab_for_b_pressed = FALSE;


int recv_data_cb(unsigned char* data, int data_len, void* ctx) {
	memcpy(&g_recv_frame_no, &data[FRAME_SEQ_START], sizeof int64_t);
	//fprintf(stdout, "%s recv frame no:%lld\n", __FUNCTION__,frame_no);
	int width = g_width;
	int height = g_height;

	
	g_bayer_image = cvCreateImageHeader(cvSize(width, height), 8, 1);
	g_rgb_image = cvCreateImage(cvSize(width,height), 8, 3);

	cvSetData(g_bayer_image, &data[FRAME_DATA_START], width);
	cvCvtColor(g_bayer_image, g_rgb_image, CV_BayerRG2RGB);
	cvShowImage("mywin0", g_rgb_image);
	cvWaitKey(1);

	cvReleaseImageHeader(&g_bayer_image);
	cvReleaseImage(&g_rgb_image);
	return 0;
}

#include "signal.h"
bool g_running_flag = true;
void sig_cb(int sig)
{
	if (sig == SIGINT) {
		fprintf(stdout, "%s\n", __FUNCTION__);
		g_running_flag = false;
	}
}

BOOL g_connect_flag = FALSE;
CPlaybackCtrlClient client;

void connect_cb(ConnectStatus status, void* ctx) {
	CPlaybackCtrlClient* client = (CPlaybackCtrlClient*)(ctx);
	if (status == ConnectStatus_CONNECTED) {
		g_connect_flag = TRUE;
		client->start_grab();

		int frames_playback = 3000;
		int play_frame_rate = 30;
		int full_frame_rate = client->get_frame_rate();
		int sample = (full_frame_rate + 1) / play_frame_rate;
		int64_t frame_no = 0;
		int width = g_width = client->get_image_width();
		int height = g_height = client->get_image_height();

		std::string name;
		client->get_user_defined_name(name);

		client->set_store_file(1);

		client->set_play_frame_resolution(g_width, g_height);
		client->set_recv_sink_callback(recv_data_cb, client);

		client->play_live(play_frame_rate, sample);
	}
	else if (status == ConnectStatus_DISCONNECT) {
		g_connect_flag = FALSE;

		cvReleaseImageHeader(&g_bayer_image);
		cvReleaseImage(&g_rgb_image);

		g_bayer_image = g_rgb_image = NULL;

		client->stop_grab();
	}
}

void run() {
	while (1) {
		
		char ci = getchar();
		fprintf(stdout, "####%c was pressed!\n", ci);
		switch (ci)
		{
		case 'p'://pause
			printf("pause frame no %d\n", g_recv_frame_no);
			client.play_pause();
			break;
		case 'f'://forward
			client.play_forward(30, 6);
			break;
		case 'b'://backplay
			client.play_backward(30, 6);
			break;
		case 'l'://live
			g_frame_a = g_frame_b = 0;
			 g_width = 1280;
			 g_height =720;
			 client.set_play_frame_resolution(g_width, g_height);
			client.play_live(30, 6);
			break;
		case 'k'://live
			g_frame_a = g_frame_b = 0;
			g_width = 320;
			g_height = 180;
			client.set_play_frame_resolution(g_width, g_height);
			client.play_live(30, 6);
			break;
		case 's'://live
			g_width = 1280;
			g_height = 720;
			client.set_play_frame_resolution(g_width, g_height);
			break;
		case 'd'://live
			g_width = 320;
			g_height = 180;
			client.set_play_frame_resolution(g_width, g_height);
			break;
		case ' '://abmode
			if (0 == g_frame_a) {
				g_frame_a = g_recv_frame_no;
				break;
			}

			if (0 == g_frame_b) {
				g_frame_b = g_recv_frame_no;
				client.play_from_a2b(g_frame_a, g_frame_b);
				break;
			}
		default:
			break;
		}
	}
}

int main(int argc, char** argv) {
	signal(SIGINT, sig_cb);  /*×¢²áctrl+cÐÅºÅ²¶»ñº¯Êý*/
	std::string server_ip("192.168.1.19");
	uint16_t		 port(9070);


	if (argc > 1)
		server_ip.assign(argv[1]);
	if (argc > 2)
		port = atoi(argv[2]);

	client.set_connected_callback(connect_cb, &client);
	client.start(server_ip, port,1);

	std::thread t(run);
	while (1) {
		if (g_running_flag == false) {
			goto end;
		}
		printf("fps: cam(%.2f), soft_grab(%.2f), soft_snd(%.2f)\n", client.get_camera__grab_fps(), client.get_soft_grab_fps(), client.get_soft_snd_fps());
		sleep(1000);
	}

end:
	client.stop_grab();
	client.stop();
	exit(0);
	return 0;
}