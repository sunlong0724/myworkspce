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

#include "AgentServerService_server.cpp"


using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;
using namespace apache::thrift::concurrency;

using namespace hawkeye;

void cmd_run(AgentServerServiceHandler* h, uint16_t port) {
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);

	boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
	boost::shared_ptr<AgentServerServiceHandler> handler(h);
	boost::shared_ptr<TProcessor> processor(new AgentServerServiceProcessor(handler));
	boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
	boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());

	int workerCount = 16;
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

std::map<std::string, std::vector<double>>  GetDiskInfo() {
	char drives_c[256] = { 0 };
	DWORD ds = GetLogicalDriveStrings(sizeof drives_c, drives_c);
	if (ds == 0) {
		DWORD err = GetLastError();
		fprintf(stdout, "%s GetLogicalDrives failed! err %d\n", __FUNCTION__, err);
		exit(0);
	}
	std::map<std::string, std::vector<double>> disk_info;
	for (int i = ds; i >= 0; --i) {
		if (drives_c[i] >= 'C' && drives_c[i] <= 'Z') {
			fprintf(stdout, "%s driver %s was found!\n", __FUNCTION__, &drives_c[i]);
			//CHECK!!!
			std::vector<double>   info;

			ULARGE_INTEGER lpuse;
			ULARGE_INTEGER lptotal;
			ULARGE_INTEGER lpfree;
			GetDiskFreeSpaceEx(&drives_c[i], &lpuse, &lptotal, &lpfree);
			fprintf(stdout, "%s: use(%4.4fGB),tatal(%4.4fGB),free(%4.4fGB)\n", &drives_c[i], lpuse.QuadPart / 1024.0 / 1024.0 / 1024.0, lptotal.QuadPart / 1024.0 / 1024.0 / 1024.0, lpfree.QuadPart / 1024.0 / 1024.0 / 1024.0);
		
			info.push_back(lpuse.QuadPart / 1024.0 / 1024.0 / 1024.0);
			info.push_back(lptotal.QuadPart / 1024.0 / 1024.0 / 1024.0);
			info.push_back(lpfree.QuadPart / 1024.0 / 1024.0 / 1024.0);

			disk_info[std::string(&drives_c[i])] =  info;
		}
	}
	return disk_info;
}

//CPU使用率；使用方法：直接调用getCpuUsage（）函数  
//原理：记录一定时间内CPU的空闲时间和繁忙时间，然后计算得出  
__int64 CompareFileTime(FILETIME time1, FILETIME time2)
{
	__int64 a = time1.dwHighDateTime << 32 | time1.dwLowDateTime;
	__int64 b = time2.dwHighDateTime << 32 | time2.dwLowDateTime;

	return   (b - a);
}
int getCpuUsage()
{
	HANDLE hEvent;
	BOOL res;

	FILETIME preidleTime;
	FILETIME prekernelTime;
	FILETIME preuserTime;

	FILETIME idleTime;
	FILETIME kernelTime;
	FILETIME userTime;

	res = GetSystemTimes(&idleTime, &kernelTime, &userTime);

	preidleTime = idleTime;
	prekernelTime = kernelTime;
	preuserTime = userTime;

	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL); // 初始值为 nonsignaled ，并且每次触发后自动设置为nonsignaled  

	WaitForSingleObject(hEvent, 1000); //等待500毫秒  
	res = GetSystemTimes(&idleTime, &kernelTime, &userTime);
	int idle = CompareFileTime(preidleTime, idleTime);
	int kernel = CompareFileTime(prekernelTime, kernelTime);
	int user = CompareFileTime(preuserTime, userTime);
	int cpu = (kernel + user - idle) * 100 / (kernel + user);
	int cpuidle = (idle) * 100 / (kernel + user);
	//cout << "CPU利用率:" << cpu << "%" << "      CPU空闲率:" <<cpuidle << "%" <<endl;  
	preidleTime = idleTime;
	prekernelTime = kernelTime;
	preuserTime = userTime;
	return cpu;
}


int main(int argc, char **argv) {

	GetDiskInfo();
	int cpu = getCpuUsage();

	fprintf(stdout, "cpu:%d\n", cpu);
	uint16_t server_port = 9090;

	if (argc >= 2) {
		server_port = ::atoi(argv[1]);
	}

	AgentServerServiceHandler* handler = new AgentServerServiceHandler();

	std::thread cmd_thread(cmd_run, handler, server_port);
	cmd_thread.join();

	return 0;
}

