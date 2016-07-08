
#if 0
#include <windows.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>     /* atoi */
#include "FPSCounter.h"

#define IMAGE_WIDTH 1280
#define IMAGE_HEIGHT 720


bool g_running_flag = true;
void sig_cb(int sig)
{
	if (sig == SIGINT) {
		fprintf(stdout, "%s\n", __FUNCTION__);
		g_running_flag = false;
	}
}

#pragma comment(lib,"winmm.lib")
#define ONE_GB 1024*1024*1024

int main0000(int argc, char** argv) {

	BOOL result = SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	result = SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
	signal(SIGINT, sig_cb);

	static long long raw_file_max_size = 10;
	char* prefix = "test.raw";
	char file_name[256] = { 0 };
	static int sleep_time = 5;
	static int write_block = 1024;

	fprintf(stdout, "%s [file max size] [file name prefix]\n", argv[0]);
	fprintf(stdout, "   [file max size(GB):default 10GB] \n", argv[0]);
	fprintf(stdout, "   [file name prefix:default %s] \n", prefix);
	
	if (argc >= 2) {
		raw_file_max_size =::atol(argv[1]);
	}

	if (argc >=3){
		sleep_time =::atoi(argv[2]);
	}

	if (argc >= 4){
		write_block =  atoi(argv[3]);
	}
	
	raw_file_max_size *= ONE_GB;
	//std::chrono::time_point<std::chrono::high_resolution_clock> last_now, start,end;
	int counter = 0;

	int buffer_len = write_block*1024;
	char* buffer = new char[buffer_len];


	PCTSTR pszPathName = "test.raw";
	HANDLE hFile = NULL;

	CFPSCounter fps_counter;

	timeBeginPeriod(1);
	while (g_running_flag) {
	if (!hFile) {
			hFile = CreateFile(pszPathName, GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, NULL);
			if (!hFile) {
				fprintf(stdout, "fopen failed!!!\n");
				exit(0);
			}
		}

		fps_counter.statistics();
	
		DWORD nBytesWritten = 0;
		WriteFile(hFile, buffer, buffer_len, &nBytesWritten, NULL);

		LARGE_INTEGER FileSize;
		BOOL r = GetFileSizeEx(hFile, &FileSize);
		if (!r) {
			printf("GetFileSizeEx failed\n");
			return FALSE;
		}

		if (FileSize.QuadPart >= raw_file_max_size) {
			SetFilePointer(hFile,0, NULL, FILE_BEGIN);
		}

		::Sleep(sleep_time);
	}

	timeEndPeriod(1);

	if (hFile)
		CloseHandle(hFile);
	delete buffer;
	return 0;

}


int main1111(int argc, char** argv)

{
	signal(SIGINT, sig_cb);

	PCTSTR pszPathName = "test.raw";
	HANDLE hFile = CreateFile(pszPathName, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("File could not be opened.");
		return FALSE;
	}

	LARGE_INTEGER FileSize;

	BOOL r = GetFileSizeEx(hFile, &FileSize);
	if (!r) {
		printf("GetFileSizeEx failed\n");
		return FALSE;
	}

	//HANDLE hFileMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0,
	//	FileSize.QuadPart, NULL);

	//if (hFileMap == NULL) {
	//	CloseHandle(hFile);
	//	return FALSE;
	//}

	//PVOID pvFile = MapViewOfFile(hFileMap, FILE_MAP_WRITE, 0, 0, 0);

	//if (pvFile == NULL) {
	//	CloseHandle(hFileMap);
	//	CloseHandle(hFile);
	//	return FALSE;
	//}


	int buffer_len = 1024*1024*1;
	char* buffer = new char[buffer_len];

	DWORD nByteWrite = 0;

	long long offset = 0;

	CFPSCounter fps_counter;


	timeBeginPeriod(1);
	while (g_running_flag) {
		WriteFile(hFile, buffer, buffer_len, &nByteWrite, NULL);

		//memcpy((void*)((char*)pvFile + offset), buffer, buffer_len);
		//offset += buffer_len;

		fps_counter.statistics();

		::Sleep(1);

		if (offset >= FileSize.QuadPart)
			offset = 0;
	}
	timeEndPeriod(1);

	delete[] buffer;

	//UnmapViewOfFile(pvFile);
	//CloseHandle(hFileMap);

	SetFilePointer(hFile, FileSize.QuadPart, NULL, FILE_BEGIN);
	SetEndOfFile(hFile);//实际上不需要写入了。
	CloseHandle(hFile);

	return 0;
}

#endif

#include <string>
#include <stdio.h>
#include "ping.h"

int main(void)
{
	CPing objPing;

	PingReply reply;

	std::string DestIP("192.168");

	for (int i = 0; i < 1; ++i) {
		for (int j = 1; j < 256; ++j) {
			std::string tmp = DestIP;
			tmp.append("." + std::to_string(i) + "." + std::to_string(j));

			if (objPing.Ping((char*)tmp.c_str(), &reply, 10)) {
				printf("Reply from %s: bytes=%ld time=%ldms TTL=%ld\n", tmp.c_str(), reply.m_dwBytes, reply.m_dwRoundTripTime, reply.m_dwTTL);
			}
			else {
				printf("%s 请求超时。\n", tmp.c_str());
			}
		}
	}

	return 0;
}