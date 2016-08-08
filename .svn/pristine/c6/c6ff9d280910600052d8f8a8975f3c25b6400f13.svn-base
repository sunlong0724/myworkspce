
#include "PlaybackCtrlClient.h"
#include "defs.h"
#include "utils.h"

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
		g_rgb_image = cvCreateImage(cvSize(g_width, g_height), 8, 3);
	}

	if (g_bayer_image->imageSize != data_len- FRAME_DATA_START) {
		//cvReleaseImage(&g_bayer_image);
		//cvReleaseImage(&g_rgb_image);

		g_bayer_image = cvCreateImageHeader(cvSize(g_width, g_height), 8, 1);
		g_rgb_image = cvCreateImage(cvSize(g_width, g_height), 8, 3);
	}
	cvSetData(g_bayer_image, &data[FRAME_DATA_START], g_width);
	cvCvtColor(g_bayer_image, g_rgb_image, CV_BayerRG2RGB);
	cvShowImage("mywin0", g_rgb_image);
	cvWaitKey(1);
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

void connect_cb(ConnectStatus status, void* ctx) {
	if (status == ConnectStatus_CONNECTED) {
		g_connect_flag = TRUE;
	}
	else if (status == ConnectStatus_DISCONNECT) {
		g_connect_flag = FALSE;

		cvReleaseImageHeader(&g_bayer_image);
		cvReleaseImage(&g_rgb_image);

		g_bayer_image = g_rgb_image = NULL;
	}
}

#include <iostream>
#include <exception>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

using namespace boost::filesystem;
using namespace boost::system;

BOOL check_avaiable_driver(std::string driver) {
	path p(driver);
	if (is_directory(p))
	{
		std::cout << p << " is a directory containing:\n";
		for (auto&& x : directory_iterator(p)) {
			if (strcmp("raw", x.path().extension().string().c_str()) == 0) {
				return FALSE;
			}
		}
	}
	return TRUE;
}

int main(int argc, char** argv) {
	signal(SIGINT, sig_cb);  /*×¢²áctrl+cÐÅºÅ²¶»ñº¯Êý*/
	std::string server_ip("192.168.1.51");
	uint16_t		 port(9070);
	CPlaybackCtrlClient client;

	if (argc > 1)
		server_ip.assign(argv[1]);
	if (argc > 2)
		port = atoi(argv[2]);

	client.set_connected_callback(connect_cb, NULL);
	client.set_connect_parameters(server_ip, port);
	client.start();

again:

	while (1) {
		if (g_connect_flag)
			break;
		sleep(200);
	}


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

	client.set_play_frame_resolution(g_width, g_height);
	client.start_play_live(play_frame_rate, sample);
		
	client.set_recv_image_parameters(GET_IMAGE_BUFFER_SIZE(g_width, g_height), (full_frame_rate+1)/play_frame_rate);
	client.set_recv_sink_callback(recv_data_cb, &client);
	client.play_live();

	int64_t now = time(NULL);

	while (1) {
		if (g_connect_flag == FALSE)
			break;
		if (g_running_flag == false) {
			goto end;
		}
		printf("fps: cam(%.2f), soft_grab(%.2f), soft_snd(%.2f)\n", client.get_camera__grab_fps(), client.get_soft_grab_fps(), client.get_soft_snd_fps());
		sleep(1000);
	}


	goto again;
	
end:
	client.stop();
	return 0;
}