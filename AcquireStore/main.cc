#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/StdThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include <iostream>
#include <stdexcept>
#include <sstream>
		  
#include "PlaybackCtrlService_server.cpp"

#include <opencv2\opencv.hpp>

#include "defs.h"
#include "utils.h"
#include "FileStorage.h"
#include "PlaybackCtrl.h"
#include "MyThread.h"
#include "SinkBayerDatasCallbackImpl.h"

#include <iostream>
#include <exception>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

#define _DALSA
#ifdef _DALSA
//  ..\DALSACamera
#if _DEBUG
#pragma comment(lib, "DALSACamerad.lib")
#else 
#pragma comment(lib, "DALSACamera.lib")
#endif
#else
#if _DEBUG
///..\PointGreyCamera
#pragma comment(lib, "PointGreyCamerad.lib")
#else
#pragma comment(lib, "PointGreyCamera.lib")
#endif
#endif

using namespace boost::filesystem;
using namespace boost::system;


std::string	CFileStorage::m_file_name = "data.raw";
 long long CFileStorage::m_max_file_size = ONE_GB;
 std::map<int64_t, int64_t> CFileStorage::m_frame_offset_map = {};
 std::map<int64_t, int64_t> CFileStorage::m_frame_timestamp_seq_map = {};
 std::map<int64_t, int64_t> CFileStorage::m_frame_seq_timestamp_map_for_read = {};


using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;
using namespace apache::thrift::concurrency;

using namespace hawkeye;

std::mutex			g_resized_mutex;

void cmd_run(PlaybackCtrlServiceHandler* h, TThreadPoolServer** pp_server, uint16_t port) {
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);

	boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
	boost::shared_ptr<PlaybackCtrlServiceHandler> handler(h);
	boost::shared_ptr<TProcessor> processor(new PlaybackCtrlServiceProcessor(handler));
	boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
	boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());

	int workerCount = 16;//for 16 clients max!!!
	boost::shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(workerCount);
	boost::shared_ptr<StdThreadFactory> threadFactory = boost::shared_ptr<StdThreadFactory>(new StdThreadFactory());
	threadManager->threadFactory(threadFactory);
	threadManager->start();
	*pp_server = new TThreadPoolServer(processor, serverTransport, transportFactory, protocolFactory, threadManager);
	//TThreadedServer server(processor,	serverTransport,	transportFactory,	protocolFactory);


	printf("Starting the server...\n");
	(*pp_server)->serve();

	delete *pp_server;
	printf("done.\n");
	
	WSACleanup();
}

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

int onCameraConnectStatusChanged(char* camera_ip, int status, void* ctx){

	if (camera_ip == NULL) {
		return 0;
	}

	if (strcmp(g_cs.m_camera_name, camera_ip) == 0) {
		fprintf(stdout, "%s %s %d!!!\n", __FUNCTION__, camera_ip, status);
		if (status == 0) {
			g_cs.m_camera->Stop();
			g_cs.m_camera->DestroyOtherObjects();
		}
		else {
			if (g_cs.m_camera->m_last_is_grabbing) {
				g_cs.m_camera->CreateOtherObjects();
				g_cs.m_camera->Start();
			}
		}
	}
	return 1;
}


CustomStruct g_cs;
int main(int argc, char **argv) {//.\\AcqurieStore.exe server_port gige_server_name camera_index data_port(eg. .\\AcquireStore.exe 9090 55555 camera_ip file_path)
	timeBeginPeriod(1);

	std::map<std::string, std::string>	Parameters_map;

	uint16_t	server_port = 9090;
	uint16_t	data_port   = 55555;
	std:string	camera_ip;
	long long raw_file_max_size_in_GB = 10;
	std::string   data_file_name;

	//Parameters 
	if (argc >= 2) {//thrift server port
		server_port = ::atoi(argv[1]);
		fprintf(stdout, "server_port(%d)!\n", server_port);
	}
	
	if (argc >= 3) {//data port 
		data_port = ::atoi(argv[2]);
		fprintf(stdout, "data_port(%d)!\n", data_port);
	}

	if (argc >= 4) {//camera_ip
		camera_ip = argv[3];
		fprintf(stdout, "camera_ip(%s)!\n", camera_ip.c_str());
	}

	if (argc >= 5) {//raw_file_max_size in GB
		raw_file_max_size_in_GB = ::atoll(argv[4]);
		fprintf(stdout, "raw_file_max_size_in_GB(%d)!\n", raw_file_max_size_in_GB);
	}

	if (argc >= 6) {
		data_file_name.assign(argv[5]);
		fprintf(stdout, "data_file_name(%s)!\n", data_file_name.c_str());
	}

	//start Create Resource
	std::map<std::string, std::map<int32_t, std::string>>  cameras = {};
	CCamera::FindCamera(&cameras);

	if (cameras.find(camera_ip) == cameras.end()) {
		fprintf(stdout, "This PC has no the camera(%s)!\n", camera_ip.c_str());
		exit(-1);
	}

	CFileStorage::m_max_file_size = raw_file_max_size_in_GB* ONE_GB;

	g_cs.m_data_port = data_port;

	g_cs.m_camera = new CCamera(cameras[camera_ip].begin()->second.c_str(), cameras[camera_ip].begin()->first);
//	g_cs.m_camera = new CCamera(camera_ip.c_str());
	if (!g_cs.m_camera->CreateDevice()) {
		fprintf(stdout, "CreateDevice failed!\n");
		return 0;
	}

	g_cs.m_camera->GetCurrentIPAddress(g_cs.m_camera_name, sizeof g_cs.m_camera_name);
	CCamera::RegisterConnectionEventCallback(onCameraConnectStatusChanged, &g_cs);//camera reconnect function!!!

	if (data_file_name.size() > 0) {
		CFileStorage::m_file_name = data_file_name;
	}
	else {//auto find a driver!!!
		char camera_ip[30];
		g_cs.m_camera->GetCurrentIPAddress(camera_ip, sizeof camera_ip);
		//CFileStorage::m_file_name = std::string(camera_ip)  + ".raw";

		char drives_c[256] = { 0 };
		DWORD ds = GetLogicalDriveStrings(sizeof drives_c, drives_c);
		if (ds == 0) {
			DWORD err = GetLastError();
			fprintf(stdout, "%s GetLogicalDrives failed! err %d\n", __FUNCTION__, err);
			exit(0);
		}
		for (int i = ds; i >= 0; --i) {
			if (drives_c[i] > 'C' && drives_c[i] <= 'Z') {
				fprintf(stdout, "%s driver %s was found!\n", __FUNCTION__, &drives_c[i]);
				//CHECK!!!
				if (check_avaiable_driver(&drives_c[i])) {
					CFileStorage::m_file_name.assign(std::string(&drives_c[i]) + std::string(camera_ip) + ".raw");
					break;
				}
			}
		}
	}


	g_cs.m_file_storage_object_for_write_thread = new CFileStorage();
	g_cs.m_playback_thread = new CPlaybackCtrlThread();
	g_cs.m_post_processor = new CPostProcessor();
	g_cs.m_snd_data_thread = new CSendData();
	g_cs.m_file_storage_object_for_read = new CFileStorage();

	//init
	g_cs.m_image_w = g_cs.m_camera->GetImageWidth();
	g_cs.m_image_h = g_cs.m_camera->GetImageHeight();
	g_cs.m_frame_rate = g_cs.m_camera->GetFrameRate();


	//start
	g_cs.m_file_storage_object_for_write_thread->start();
	g_cs.m_playback_thread->start();
	//do not start here!!! g_cs.m_post_processor_thread->start();

	sleep(1000);
	//FIXME:
	g_cs.m_snd_data_thread->m_data_transport_mode = 2;
	std::thread  data_thread;
	TThreadPoolServer* data_server = NULL;
	if (g_cs.m_snd_data_thread->m_data_transport_mode == 2) {//thrift
		PlaybackCtrlServiceHandler* handler2 = new PlaybackCtrlServiceHandler();
		data_thread = std::thread(cmd_run, handler2, &data_server, g_cs.m_data_port);
	}

	g_cs.m_snd_data_thread->init(g_cs.m_data_port);
	g_cs.m_snd_data_thread->set_parameters(GET_IMAGE_BUFFER_SIZE(g_cs.m_image_w, g_cs.m_image_h));
	g_cs.m_snd_data_thread->start();

	TThreadPoolServer* cmd_server = NULL;
	PlaybackCtrlServiceHandler* handler = new PlaybackCtrlServiceHandler();
	std::thread cmd_thread(cmd_run,handler, &cmd_server,server_port);

	//cmd_thread.join();
	while (!g_cs.m_exited) {
		
		fprintf(stdout, "FPS statistics:cam(%.2f),soft(%.2f),disk_w(%.2f),snd(%.2f)\r", \
			g_cs.m_camera->GetGrabFPS(), g_cs.m_soft_grab_counter.GetFPS(), g_cs.m_file_storage_object_for_write_thread->m_fps_counter.GetFPS(), g_cs.m_snd_data_thread->m_soft_snd_counter.GetFPS());
	

		//fprintf(stdout, "\n2####m_last_live_play_seq %d, m_frame_counter %d, m_play_frame_gap %d m_snd_live_frame_flag %d\n", g_cs.m_last_live_play_seq, g_cs.m_frame_counter, g_cs.m_play_frame_gap, g_cs.m_snd_live_frame_flag);
	
		if (g_cs.m_start_grab_time != 0 && time(NULL) - g_cs.m_start_grab_time > 5) {
			if (g_cs.m_camera->m_grabbing == TRUE && g_cs.m_camera->GetGrabFPS() < 1) {
#ifndef _DEBUG
//maybe cause a problem  here and onCameraConnectStatusChanged will execute the flow actions at the same time
				g_cs.m_camera->Stop();
				g_cs.m_camera->DestroyOtherObjects();
				g_cs.m_camera->CreateOtherObjects();
				g_cs.m_camera->Start();

				g_cs.m_start_grab_time = time(NULL);
#endif
			}
		}


		if (g_cs.m_camera->m_grabbing == TRUE) {
			((CFPSCounter*)g_cs.m_camera->m_GrabFPSCounter)->statistics(__FUNCTION__, FALSE, FALSE);
		}

		sleep(1000);
	}

	if (cmd_server) {
		cmd_server->getServerTransport()->close();//////////////////////////////////////////////
		cmd_server->stop();
		fprintf(stdout, "%s 1\n", __FUNCTION__);
			//if (cmd_thread.joinable()) 
		{
		//		fprintf(stdout, "%s 2\n", __FUNCTION__);
		//		cmd_thread.join();
		//		fprintf(stdout, "%s 3\n", __FUNCTION__);

		}
	}

	if (data_server) {
		data_server->getServerTransport()->close();///////////////////////////////////////////////////
		data_server->stop();
		//if (data_thread.joinable())
		//	data_thread.join();
	}

	g_cs.m_playback_thread->stop();
	g_cs.m_file_storage_object_for_write_thread->stop();
	g_cs.m_snd_data_thread->stop();

	g_cs.m_camera->Stop();
	sleep(300);
	g_cs.m_camera->DestroyOtherObjects();
	g_cs.m_camera->DestroyDevice();

	exit(0);//exit

	delete g_cs.m_camera;
	delete g_cs.m_file_storage_object_for_write_thread;
	delete g_cs.m_file_storage_object_for_read;
	delete g_cs.m_snd_data_thread;
	delete g_cs.m_post_processor;
	delete g_cs.m_playback_thread;

	timeEndPeriod(1);
	return 0;
}
