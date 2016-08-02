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

std::string	CFileStorage::m_file_name = "";
 long long CFileStorage::m_max_file_size = ONE_GB;
 std::map<int64_t, int64_t> CFileStorage::m_frame_offset_map = {};

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;
using namespace apache::thrift::concurrency;

using namespace hawkeye;

void cmd_run(PlaybackCtrlServiceHandler* h, uint16_t port) {
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

	int workerCount = 1;
	boost::shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(workerCount);
	boost::shared_ptr<StdThreadFactory> threadFactory = boost::shared_ptr<StdThreadFactory>(new StdThreadFactory());
	threadManager->threadFactory(threadFactory);
	threadManager->start();
	TThreadPoolServer server(processor, serverTransport, transportFactory, protocolFactory, threadManager);
	//TThreadedServer server(processor,	serverTransport,	transportFactory,	protocolFactory);


	printf("Starting the server...\n");
	server.serve();
	printf("done.\n");

	WSACleanup();
}

CustomStruct g_cs;

int main(int argc, char **argv) {//.\\AcqurieStore.exe server_port gige_server_name camera_index data_port(eg. .\\AcquireStore.exe 9090 name 1 55555)

	uint16_t server_port = 9090;
	char*	 gige_server_name = NULL;
	uint16_t camera_index = 0;
	uint16_t data_port = 55555;
	static long long raw_file_max_size_in_GB = 10;

	timeBeginPeriod(1);
	
	if (argc >= 2) {//thrift server port
		server_port = ::atoi(argv[1]);
	}
	if (argc >= 3) {//gige_server_name
		gige_server_name = argv[2];
	}
	else {
		fprintf(stdout, "No GigEServer found!\n!");
		return 0;
	}
	if (argc >= 4) {//the index of the camera on the gigeServer
		camera_index = ::atoi(argv[3]);
	}
	if (argc >= 5) {//data port 
		data_port = ::atoi(argv[4]);
	}

	if (argc >= 6) {//raw_file_max_size in GB
		raw_file_max_size_in_GB = ::atoll(argv[5]);
	}


	CFileStorage::m_max_file_size = raw_file_max_size_in_GB* ONE_GB;
	PlaybackCtrlServiceHandler* handler = new PlaybackCtrlServiceHandler();
	g_cs.m_data_port = data_port;

	g_cs.m_camera = new CCamera(gige_server_name, camera_index);
	if (!g_cs.m_camera->CreateDevice()) {
		fprintf(stdout, "CreateDevice failed!\n");
		return 0;
	}
	g_cs.m_camera->RegisterConnectionEventCallback();
	g_cs.m_how_many_frames = -1;
	g_cs.m_how_many_frames_flag = FALSE;

	g_cs.m_playback_thread = new CPlaybackCtrlThread();
	g_cs.m_post_processor_thread = new CPostProcessor();
	g_cs.m_snd_data_thread = new CSendData();
	g_cs.m_file_storage_object_for_write = new CFileStorage();
	g_cs.m_file_storage_object_for_read = new CFileStorage();

	//init
	g_cs.m_image_w = g_cs.m_camera->GetImageWidth();
	g_cs.m_image_h = g_cs.m_camera->GetImageHeight();
	g_cs.m_frame_rate = g_cs.m_camera->GetFrameRate();

	//start
	g_cs.m_playback_thread->start();
	//do not start here!!! g_cs.m_post_processor_thread->start();

	g_cs.m_snd_data_thread->init(g_cs.m_data_port);
	g_cs.m_snd_data_thread->start();

	std::thread cmd_thread(cmd_run,handler, server_port);
	cmd_thread.join();

	g_cs.m_playback_thread->stop();
	g_cs.m_snd_data_thread->stop();

	g_cs.m_camera->DestroyDevice();

	timeEndPeriod(1);

	delete g_cs.m_camera;
	delete g_cs.m_file_storage_object_for_write;
	delete g_cs.m_file_storage_object_for_read;
	delete g_cs.m_snd_data_thread;
	delete g_cs.m_post_processor_thread;
	delete g_cs.m_playback_thread;

	return 0;
}
