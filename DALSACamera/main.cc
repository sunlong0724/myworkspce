#include "Camera.h"

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
	BOOL result = SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	result = SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	signal(SIGINT, sig_cb);
	
	//std::vector<std::shared_ptr<CCamera>> cameras;

	std::map<std::string, std::map<int32_t, std::string>>  cameras;

	BOOL ret = CCamera::FindCamera(&cameras);
	CCamera* camera = new CCamera(cameras.begin()->second.begin()->second.c_str(), cameras.begin()->second.begin()->first);
	camera->RegisterConnectionEventCallback();

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