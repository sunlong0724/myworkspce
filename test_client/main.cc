
#include "PlaybackCtrlClient.h"
#include "defs.h"
#include "utils.h"

#include <opencv2\opencv.hpp>

int64_t				g_frame_timestamp;
int64_t				g_recv_frame_no;
IplImage*			g_bayer_image = NULL;
IplImage*			g_rgb_image = NULL;
int64_t				g_width;
int64_t				g_height;

int64_t				g_frame_a = 0;
int64_t				g_frame_b = 0;
BOOL				g_mode_ab_for_a_pressed = FALSE;
BOOL				g_mode_ab_for_b_pressed = FALSE;
time_t				g_now_time;
CFPSCounter			g_recv_fps_counter2;

int					g_play_frame_rate = 25;

bool				g_show = true;

int recv_data_cb(unsigned char* data, int data_len, void* ctx) {
	memcpy(&g_recv_frame_no, &data[FRAME_SEQ_START], sizeof int64_t);
	memcpy(&g_frame_timestamp, &data[FRAME_TIMESTAMP_START], sizeof int64_t);

	//fprintf(stdout, "%s recv frame no:%lld\n", __FUNCTION__,frame_no);
	int width = g_width;
	int height = g_height;

	g_recv_fps_counter2.statistics(__FUNCTION__, FALSE);
	
	g_bayer_image = cvCreateImageHeader(cvSize(width, height), 8, 1);
	g_rgb_image = cvCreateImage(cvSize(width,height), 8, 3);

	cvSetData(g_bayer_image, &data[FRAME_DATA_START], width);
	cvCvtColor(g_bayer_image, g_rgb_image, CV_BayerBG2RGB);
	if (g_show) {
		cvShowImage("mywin0", g_rgb_image);
		cvWaitKey(1);
	}
	cvReleaseImageHeader(&g_bayer_image);
	cvReleaseImage(&g_rgb_image);

	//g_now_time = time(NULL);
	
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
BOOL	g_store_file = TRUE;
void connect_cb(ConnectStatus status, void* ctx) {
	CPlaybackCtrlClient* client = (CPlaybackCtrlClient*)(ctx);
	if (status == ConnectStatus_CONNECTED) {
		g_connect_flag = TRUE;
		client->start_grab();

		double full_frame_rate = client->get_frame_rate();
		int sample = (full_frame_rate + 1.0) / g_play_frame_rate;
		int64_t frame_no = 0;
		int width = g_width = client->get_image_width();
		int height = g_height = client->get_image_height();

		std::string name;
		client->get_user_defined_name(name);

		client->set_store_file(g_store_file);

		client->set_play_frame_resolution(g_width, g_height);
		client->set_recv_sink_callback(recv_data_cb, client);

		client->play_live(g_play_frame_rate, sample);
	}
	else if (status == ConnectStatus_DISCONNECT) {
		g_connect_flag = FALSE;

		cvReleaseImageHeader(&g_bayer_image);
		cvReleaseImage(&g_rgb_image);

		g_bayer_image = g_rgb_image = NULL;

		client->stop_grab();
	}
}

void connect_cb2(ConnectStatus status, void* ctx) {

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
			client.play_forward(g_play_frame_rate, (client.get_frame_rate() + 0.5)/ g_play_frame_rate);
			break;
		case 'b'://backplay
			client.play_backward(g_play_frame_rate, (client.get_frame_rate() + 0.5) / g_play_frame_rate);
			break;
		case 'l'://live
			g_frame_a = g_frame_b = 0;
			 g_width = client.get_image_width();
			 g_height =client.get_image_height();
			 client.set_play_frame_resolution(g_width, g_height);
			client.play_live(g_play_frame_rate, (client.get_frame_rate() + 0.5) / g_play_frame_rate);
			break;
		case 'k'://live
			g_frame_a = g_frame_b = 0;
			g_width = client.get_image_width()/4;
			g_height = client.get_image_height() / 4;
			client.set_play_frame_resolution(g_width, g_height);
			client.play_live(g_play_frame_rate, (client.get_frame_rate() + 0.5) / g_play_frame_rate);
			break;
		case 's'://live
			g_width = client.get_image_width();
			g_height = client.get_image_height();
			client.set_play_frame_resolution(g_width, g_height);
			break;
		case 'd'://live
			g_width = client.get_image_width() / 4;
			g_height = client.get_image_height() / 4;
			client.set_play_frame_resolution(g_width, g_height);
			break;
		case 't':
			g_store_file = !g_store_file;
			client.set_store_file(g_store_file);
			break;
		case 'n':
			client.get_the_frame_data(1, 1);
			break;
		case 'm':
			client.get_the_frame_data(0, 1);
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

		case 'e':
			client.kill_myself();
			break;
		case 'u':
			client.sync_frame_by_timestamp_in_pause(g_frame_timestamp);
			break;
		case 'h'://hide
			g_show = !g_show;
			break;
		default:
			break;
		}
	}
}

#if 0 
CPlaybackCtrlClient g_client;
int main(int argc, char** argv) {

	//CPlaybackCtrlClient c1;
	//CPlaybackCtrlClient* cl;
	//cl = new CPlaybackCtrlClient;
	signal(SIGINT, sig_cb);  /*×¢²áctrl+cÐÅºÅ²¶»ñº¯Êý*/
	std::string server_ip("192.168.1.19");
	uint16_t		 port(9070);

	if (argc > 1)
		server_ip.assign(argv[1]);
	if (argc > 2)
		port = atoi(argv[2]);

	client.set_connected_callback(connect_cb, &client);
	client.start(server_ip, port,1);

	g_client.set_connected_callback(connect_cb2, &g_client);
	g_client.start(server_ip, port, 0);
	
	std::thread t(run);
	CFPSCounter tmp;

	//cvNamedWindow("mywin0");
	while (1) {
		if (g_running_flag == false) {
			goto end;
		}

		tmp.statistics(__FUNCTION__, FALSE);
		//g_now_time = time(NULL);
		printf("%lld fps: cam(%.2f), soft_grab(%.2f), soft_snd(%.2f)\n",g_now_time, g_client.get_camera__grab_fps(), \
			g_client.get_soft_grab_fps(), g_client.get_soft_snd_fps());
		sleep(1000);
		
	}

end:
	g_client.stop();
	client.stop_grab();
	client.stop();
	exit(0);
	return 0;
}
#endif


int main1111(int argc, char** argv) {

	int data_len = 1280 * 720;
	char* buf = new char[data_len];

	FILE* fp = fopen("d:\\data.raw", "rb");

	g_bayer_image = cvCreateImageHeader(cvSize(1280, 720), 8, 1);
	while (fread(buf, 1, data_len, fp) != 0) {
	
		cvSetData(g_bayer_image, buf, 1280);
		cvShowImage("mywin0", g_bayer_image);
		cvWaitKey(30);

	
	}
	cvReleaseImageHeader(&g_bayer_image);
	return 0;
}

// condition_variable example
#include <iostream>           // std::cout
#include <thread>             // std::thread
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable

std::mutex mtx;
std::condition_variable cvar;
bool ready = false;

void print_id(int id) {
	std::unique_lock<std::mutex> lck(mtx);
	std::cout << "1 thread " << id << '\n';

	while (!ready) cvar.wait(lck);
	// ...
	std::cout << "2 thread " << id << '\n';
}

void go() {
	std::unique_lock<std::mutex> lck(mtx);
	ready = true;
	std::cout << "1 g0 thread "  << '\n';
	cvar.notify_all();
	std::cout << "2 go thread "  << '\n';
}

int main()
{
	std::thread threads[10];
	// spawn 10 threads:
	for (int i = 0; i<10; ++i)
		threads[i] = std::thread(print_id, i);

	std::cout << "10 threads ready to race...\n";
	go();                       // go!

	for (auto& th : threads) th.join();

	return 0;
}