
#include "Camera.h"
#if 1
#include <string>
#include <thread>
#include <signal.h>
#include <vector>
#include <functional>

FILE* g_err_fp;

bool g_running_flag = true;
void sig_cb(int sig)
{
	if (sig == SIGINT) {
		fprintf(stdout, "%s\n", __FUNCTION__);
		g_running_flag = false;
	}
}

int main(int argc, char** argv) {
	//BOOL result = SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	//result = SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	signal(SIGINT, sig_cb);
	
	//std::vector<std::shared_ptr<CCamera>> cameras;

	std::map<std::string, std::map<int32_t, std::string>>  cameras;

	std::string ip("192.168.0.6");
	BOOL ret = CCamera::FindCamera(&cameras);
	//CCamera* camera = new CCamera("192.168.0.9");
	CCamera* camera = new CCamera(cameras[ip].begin()->second.c_str(), cameras[ip].begin()->first);
	camera->RegisterConnectionEventCallback(NULL, NULL);

	if (camera->CreateDevice() && camera->CreateOtherObjects()) {
		camera->Start();
	}

	while (1) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	//camera->EnableColorConvert(TRUE);

	//if (ret){
	//	for (auto& a : cameras) {
	//		if (a->CreateDevice()) {
	//			a->EnableColorConvert(TRUE);
	//			//a->CreateOtherObjects();
	//			//a->Start();
	//		}
	//	}
	//}
	//else {
	//	printf("No device found!\n");
	//}

	//BOOL flag = FALSE;

	//auto& a = cameras[0];
	//for (auto& b : cameras) {
	//	if (strcmp(b->GetUserDefinedName(), "S1100466") == 0) {
	//		a = b;
	//		if (a->CreateOtherObjects()) {
	//			a->Start();
	//		}
	//	}
	//	else {
	//		b->DestroyDevice();
	//	}
	//}
	//while (g_running_flag) {
	//	
	//	//for (auto& a : cameras) 
	//	{
	//		if (!flag) {
	//			a->DumpRawImage();
	//			a->DumpRgbImage();
	//			//a->Stop();
	//			int offsest_x =160 , offset_y = 0;
	//			int width, height;
	//			char* fmt = "BayerRG10";//"BayerRG8"

	//			//a->SetExposureTime(1000);
	//			//a->SetFrameRate(20.0f);
	//		
	//			//fprintf(stdout, "turboTransferEnable:%d\n", a->IsEnabledTurboTransfer());
	//			//a->EnableTurboTransfer();

	//			//a->SetImageWidth(960);
	//			//a->SetOffsetX(offsest_x);

	//			//a->SetPixelFormat(fmt, strlen(fmt));

	//			//width = a->GetImageWidth();
	//			//val test
	//			//double val = .0f;
	//			//val = a->GetGainBySensorAnalog();
	//			//fprintf(stdout, "SensorAnalog grain: %.2f\n", val);
	//			//val = a->GetGainBySensorDigital();
	//			//fprintf(stdout, "SensorDigital grain: %.2f\n", val);

	//			//val = a->GetGainBySensorAll();
	//			//fprintf(stdout, "SensorAll grain: %.2f\n", val);
	//			//a->SetGainBySensorAll(5.0f);

	//			//Range test
	//			//double min, max;
	//			//BOOL bStatus = a->GetExposureTimeRange(&min, &max);
	//			//BOOL bStatus = a->GetGainBySensorDigitalRange(&min, &max);
	//			//BOOL bStatus = a->GetGainBySensorAnalogRange(&min, &max);
	//			//BOOL bStatus = a->GetFrameRateRange(&min, &max);
	//			flag = TRUE;
	//		}
	//		fprintf(stdout, "grab fps: %.2f\n", a->GetGrabFPS());
	//		fprintf(stdout, "process fps: %.2f\n", a->GetProcessFPS());
	//	}

	//	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	//}

	//for (auto& a : cameras) {
	//	a->Stop();
	//	a->DestroyOtherObjects();
	//	a->DestroyDevice();
	//}

	return 0;
}
#endif

///////////////////////////////////////////////////////////////////

#if 0
#include "FlyCapture2.h"
#pragma comment(lib, "FlyCapture2.lib")

using namespace FlyCapture2;

//void PrintBuildInfo()
//{
//	FC2Version fc2Version;
//	Utilities::GetLibraryVersion(&fc2Version);
//	char version[128];
//	sprintf(
//		version,
//		"FlyCapture2 library version: %d.%d.%d.%d\n",
//		fc2Version.major, fc2Version.minor, fc2Version.type, fc2Version.build);
//
//	printf(version);
//
//	char timeStamp[512];
//	sprintf(timeStamp, "Application build date: %s %s\n\n", __DATE__, __TIME__);
//
//	printf(timeStamp);
//}

//void PrintCameraInfo(CameraInfo* pCamInfo)
//{
//	printf(
//		"\n*** CAMERA INFORMATION ***\n"
//		"Serial number - %u\n"
//		"Camera model - %s\n"
//		"Camera vendor - %s\n"
//		"Sensor - %s\n"
//		"Resolution - %s\n"
//		"Firmware version - %s\n"
//		"Firmware build time - %s\n\n",
//		pCamInfo->serialNumber,
//		pCamInfo->modelName,
//		pCamInfo->vendorName,
//		pCamInfo->sensorInfo,
//		pCamInfo->sensorResolution,
//		pCamInfo->firmwareVersion,
//		pCamInfo->firmwareBuildTime);
//}

static void PrintError(Error error)
{
	error.PrintErrorTrace();
}

int RunSingleCamera(PGRGuid guid)
{
	const int k_numImages = 10;

	Error error;
	Camera cam;

	// Connect to a camera
	error = cam.Connect(&guid);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}

	// Get the camera information
	CameraInfo camInfo;
	error = cam.GetCameraInfo(&camInfo);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}

	//PrintCameraInfo(&camInfo);

	// Start capturing images
	error = cam.StartCapture();
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}

	Image rawImage;
	for (int imageCnt = 0; imageCnt < k_numImages; imageCnt++)
	{
		// Retrieve an image
		error = cam.RetrieveBuffer(&rawImage);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			continue;
		}

		printf("Grabbed image %d\n", imageCnt);

		// Create a converted image
		Image convertedImage;

		// Convert the raw image
		error = rawImage.Convert(PIXEL_FORMAT_MONO8, &convertedImage);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}

		// Create a unique filename
		char filename[512];
		sprintf(filename, "FlyCapture2Test-%u-%d.pgm", camInfo.serialNumber, imageCnt);

		// Save the image. If a file format is not passed in, then the file
		// extension is parsed to attempt to determine the file format.
		error = convertedImage.Save(filename);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
	}

	// Stop capturing images
	error = cam.StopCapture();
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}

	// Disconnect the camera
	error = cam.Disconnect();
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}

	return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
//	PrintBuildInfo();

	Error error;

	// Since this application saves images in the current folder
	// we must ensure that we have permission to write to this folder.
	// If we do not have permission, fail right away.
	FILE* tempFile = fopen("test.txt", "w+");
	if (tempFile == NULL)
	{
		printf("Failed to create file in current folder.  Please check permissions.\n");
		return -1;
	}
	fclose(tempFile);
	remove("test.txt");

	BusManager busMgr;
	unsigned int numCameras;
	error = busMgr.GetNumOfCameras(&numCameras);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}

	printf("Number of cameras detected: %u\n", numCameras);

	for (unsigned int i = 0; i < numCameras; i++)
	{
		PGRGuid guid;
		error = busMgr.GetCameraFromIndex(i, &guid);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}

		RunSingleCamera(guid);
	}

	printf("Done! Press Enter to exit...\n");

	return 0;
}
#endif