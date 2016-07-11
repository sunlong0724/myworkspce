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
		  
#include "AcquireStoreService_server.cpp"

std::string	CStoreFile::m_file_name = "";

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;
using namespace apache::thrift::concurrency;

using namespace hawkeye;

void cmd_run(AcquireStoreServiceHandler* h, uint16_t port) {
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);

	boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
	boost::shared_ptr<AcquireStoreServiceHandler> handler(h);
	boost::shared_ptr<TProcessor> processor(new AcquireStoreServiceProcessor(handler));
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

int main(int argc, char **argv) {//.\\AcqurieStore.exe server_port gige_server_name camera_index data_port(eg. .\\AcqurieStore.exe 9090 name 1 55555)

	uint16_t server_port = 9090;
	char*	 gige_server_name = NULL;
	uint16_t camera_index = 0;
	uint16_t data_port = 55555;
	
	if (argc >= 2) {
		server_port = ::atoi(argv[1]);
	}
	if (argc >= 3) {
		gige_server_name = argv[2];
	}
	else {
		fprintf(stdout, "No GigEServer found!\n!");
		return 0;
	}
	if (argc >= 4) {
		camera_index = ::atoi(argv[3]);
	}
	if (argc >= 5) {
		data_port = ::atoi(argv[4]);
	}


	AcquireStoreServiceHandler* handler = new AcquireStoreServiceHandler();
	handler->m_data_port = data_port;

	CCamera* camera = new CCamera(gige_server_name, camera_index);
	if (!camera->CreateDevice()) {
		fprintf(stdout, "CreateDevice failed!\n");
		return 0;
	}



	handler->m_camera = camera;

	std::thread cmd_thread(cmd_run,handler, server_port);
	cmd_thread.join();

	camera->DestroyDevice();
	delete camera;
	return 0;
}
