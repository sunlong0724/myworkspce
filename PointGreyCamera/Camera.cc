#include "Camera.h"

#include "FlyCapture2.h"
#include "FPSCounter.h"

#include <iostream>
#include <string>

#pragma comment(lib, "FlyCapture2.lib")

#define CORSERVER_MAX_STRLEN 30
#define MAX_SERIVAL_NUM_LEN CORSERVER_MAX_STRLEN
#define IMAGE_WIDTH  1280
#define IMAGE_HEIGHT 720

using namespace FlyCapture2;
using namespace std;

void* CCamera::m_dummy_rgb_fp = NULL;
CameraConnectStatusChanged CCamera::m_status_changed_cb = NULL;
void* CCamera::m_status_changed_cb_ctx = NULL;

void PrintError(Error error){
	error.PrintErrorTrace();
}

void PrintCameraInfo(CameraInfo* pCamInfo){
	cout << "\n*** CAMERA INFORMATION ***\n"
		<< "Serial number - " << pCamInfo->serialNumber << endl
		<< "Camera model - " << pCamInfo->modelName << endl
		<< "Camera vendor - " << pCamInfo->vendorName << endl
		<< "Sensor - " << pCamInfo->sensorInfo << endl
		<< "Resolution - " << pCamInfo->sensorResolution << endl
		<< "Firmware version - " << pCamInfo->firmwareVersion << endl
		<< "Firmware build time - " << pCamInfo->firmwareBuildTime << endl << endl;
}

void onHaveDataCb(class Image *pImage, const void *pCallbackData) {
	CCamera* camera = (CCamera*)pCallbackData;
	Error		 error;
	unsigned int iImageSize = 0;
	//Get the size of the buffer associated with the image, in bytes. Returns the size of the buffer in bytes.
	iImageSize = pImage->GetDataSize();

	((CFPSCounter*)camera->m_GrabFPSCounter)->statistics(__FUNCTION__, TRUE);

	if (camera->m_sink_bayer_cb) {
		camera->m_sink_bayer_cb(pImage->GetData(), pImage->GetCols()*pImage->GetRows(), 0, camera->m_ctx0);
	}
}


CCamera::CCamera(const char* camera_ip) :m_AcqDevice(NULL), m_Buffers(NULL), m_Xfer(NULL), m_sink_bayer_cb(NULL), m_ctx0(NULL),
m_sink_rgb_cb(NULL), m_ctx1(NULL), m_dummy_bayer_fp(NULL),  m_bEnableColorConvert(FALSE), \
m_connection_status(UNKNOWN), m_grabbing(FALSE), m_reconnect_flag(FALSE), m_last_is_grabbing(FALSE), m_last_is_connected(FALSE) {
	if (camera_ip == NULL) {
		throw std::exception("Error: Camera IP is NULL");
	}
	else {
		m_Buffers = new char[strlen(camera_ip) + 1];
		strcpy((char*)m_Buffers, camera_ip);
		m_GrabFPSCounter = new CFPSCounter;

		IPAddress	ipAddress;
		Error error ;
		std::string ip_str((char*)m_Buffers);
		ipAddress.octets[0] = std::atoi((char*)m_Buffers);

		size_t pos = ip_str.find('.');
		ipAddress.octets[1] = std::atoi(&((char*)m_Buffers)[pos + 1]);

		pos = ip_str.find('.', pos + 1);
		ipAddress.octets[2] = std::atoi(&((char*)m_Buffers)[pos + 1]);

		pos = ip_str.find('.', pos + 1);
		ipAddress.octets[3] = std::atoi(&((char*)m_Buffers)[pos + 1]);

		//PGRGuid guid;
		m_Pro = new PGRGuid;
		error = ((BusManager*)m_dummy_rgb_fp)->GetCameraFromIPAddress(ipAddress, (PGRGuid*)m_Pro);
		if (error != PGRERROR_OK) {
			PrintError(error);
			throw std::exception( "GetCameraFromIPAddress failed!\n" );;
		}

		m_AcqDevice = new Camera;
		((Camera*)m_AcqDevice)->Connect((PGRGuid*)m_Pro);

		if (error != PGRERROR_OK) {
			PrintError(error);
			//throw exception!!!
			throw std::exception("Camera connect failed!\n");
		}

		// Get the camera information
		CameraInfo camInfo;
		error = ((Camera*)m_AcqDevice)->GetCameraInfo(&camInfo);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			throw std::exception("GetCameraInfo failed!\n");
		}

		PrintCameraInfo(&camInfo);
	}
}

CCamera::CCamera(const char* serverName, int index) :m_AcqDevice(NULL), m_Buffers(NULL), m_Xfer(NULL), m_sink_bayer_cb(NULL), m_ctx0(NULL),
		m_sink_rgb_cb(NULL), m_ctx1(NULL), m_dummy_bayer_fp(NULL),  m_bEnableColorConvert(FALSE), \
	m_connection_status(UNKNOWN) , m_grabbing(FALSE), m_reconnect_flag(FALSE), m_last_is_grabbing(FALSE), m_last_is_connected(FALSE){

	std::map <std::string, std::map<int32_t, std::string>> cameras;
	CCamera::FindCamera(&cameras);

	bool flag = false;
	for (std::map<std::string, std::map<int32_t, std::string>>::iterator it = cameras.begin(); it != cameras.end(); ++it) {
		if (it->second.begin()->first == index) {
			flag = true;
			break;
		}
	}

	if (!flag) {
		char err_info[100];
		sprintf(err_info, "%s CCamera index(%d) is not exist!\n",__FUNCTION__, index);
		throw std::exception(err_info);
	}

	//PGRGuid guid;
	m_Pro = new PGRGuid;
	Error error = ((BusManager*)m_dummy_rgb_fp)->GetCameraFromIndex(index, (PGRGuid*)m_Pro);
	if (error != PGRERROR_OK) {
		PrintError(error);
		throw std::exception("GetCameraFromIndex failed!\n");;
	}

	m_AcqDevice = new Camera;
	((Camera*)m_AcqDevice)->Connect((PGRGuid*)m_Pro);

	if (error != PGRERROR_OK) {
		PrintError(error);
		//throw exception!!!
		throw std::exception("Camera connect failed!\n");
	}

	// Get the camera information
	CameraInfo camInfo;
	error = ((Camera*)m_AcqDevice)->GetCameraInfo(&camInfo);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		throw std::exception("GetCameraInfo failed!\n");
	}
	PrintCameraInfo(&camInfo);
	
	//camInfo.ipAddress
	std::string ip;
	int i = 0;
	for (; i < 3; ++i) {
		ip.append(std::to_string(camInfo.ipAddress.octets[i]) + ".");
	}
	ip.append(std::to_string(camInfo.ipAddress.octets[i]));

	m_Buffers = new char[strlen(ip.c_str()) + 1];
	strcpy((char*)m_Buffers, ip.c_str());
	m_GrabFPSCounter = new CFPSCounter;

}

void CCamera::SetSinkBayerDataCallback(CameraSinkDataCallback cb, void* ctx) {
	m_sink_bayer_cb = cb;
	m_ctx0 = ctx;
}

void CCamera::SetSinkRGBDataCallback(CameraSinkDataCallback cb, void* ctx) {
	m_sink_rgb_cb = cb;
	m_ctx1 = ctx;
}

CCamera::~CCamera() {
	((Camera*)m_AcqDevice)->Disconnect();

	delete m_Buffers;
	delete m_Pro;
	delete m_AcqDevice;
	delete m_GrabFPSCounter;
}

void CCamera::Start() {
	if (m_grabbing) {
		return;
	}
	m_grabbing = true;
	Error error;
	error = ((Camera*)m_AcqDevice)->StartCapture(onHaveDataCb, this);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		cout << "Error starting to capture images." << endl;
		throw std::exception("StartCapture failed!\n");
	}
	
	return;
}

void CCamera::Stop() {
	if (m_grabbing == false) {
		return;
	}

	m_grabbing = false;
	Error		 error;
	error = ((Camera*)m_AcqDevice)->StopCapture();
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		cout << "Error stop to capture images." << endl;
		return;
	}
	((Camera*)m_AcqDevice)->SetCallback(NULL, NULL);
}

BOOL CCamera::CreateDevice() {
	if (CCamera::m_dummy_rgb_fp == NULL) {
		CCamera::m_dummy_rgb_fp = new BusManager;//no destroy BusManager anywhere!!!
	}
	return m_last_is_connected = TRUE;
}
BOOL CCamera::CreateOtherObjects(){
	return TRUE;
}

void CCamera::DestroyDevice() {

}

void CCamera::DestroyOtherObjects() {
}

static void busEventCallbackArrival(void* pParameter, unsigned int serialNumber) {

	Error error;
	PGRGuid pGuid;
	Camera* cam = new Camera;
	cam->Connect(&pGuid);

	if (error != PGRERROR_OK) {
		PrintError(error);
		//throw exception!!!
		throw std::exception("Camera connect failed!\n");
	}

	// Get the camera information
	CameraInfo camInfo;
	error = cam->GetCameraInfo(&camInfo);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		throw std::exception("GetCameraInfo failed!\n");
	}
	PrintCameraInfo(&camInfo);

	//camInfo.ipAddress
	std::string ip;
	int i = 0;
	for (; i < 3; ++i) {
		ip.append(std::to_string(camInfo.ipAddress.octets[i]) + ".");
	}
	ip.append(std::to_string(camInfo.ipAddress.octets[i]));
	delete cam;


	if (CCamera::m_status_changed_cb) {
		CCamera::m_status_changed_cb((char*)ip.c_str(), TRUE, CCamera::m_status_changed_cb_ctx);
	}
}


static void busEventCallbackRemoval(void* pParameter, unsigned int serialNumber) {
	Error error;
	PGRGuid pGuid;
	
	error = ((BusManager*)CCamera::m_dummy_rgb_fp)->GetCameraFromSerialNumber(
		serialNumber,
		&pGuid);
	if (error != PGRERROR_OK) {
		PrintError(error);
		return;
	}

	Camera* cam = new Camera;
	cam->Connect(&pGuid);

	if (error != PGRERROR_OK) {
		PrintError(error);
		//throw exception!!!
		throw std::exception("Camera connect failed!\n");
	}

	// Get the camera information
	CameraInfo camInfo;
	error = cam->GetCameraInfo(&camInfo);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		throw std::exception("GetCameraInfo failed!\n");
	}
	PrintCameraInfo(&camInfo);

	//camInfo.ipAddress
	std::string ip;
	int i = 0;
	for (; i < 3; ++i) {
		ip.append(std::to_string(camInfo.ipAddress.octets[i]) + ".");
	}
	ip.append(std::to_string(camInfo.ipAddress.octets[i]));
	delete cam;


	if (CCamera::m_status_changed_cb) {
		CCamera::m_status_changed_cb((char*)ip.c_str(), FALSE, CCamera::m_status_changed_cb_ctx);
	}

}

void CCamera::RegisterConnectionEventCallback(CameraConnectStatusChanged cb, void* ctx) {
	m_status_changed_cb = cb;
	m_status_changed_cb_ctx = ctx;
	Error error;
	CallbackHandle _cb_handle;

	error = ((BusManager*)m_dummy_rgb_fp)->RegisterCallback(busEventCallbackArrival,
		ARRIVAL,
		NULL,
		&_cb_handle
	);

	if (error != PGRERROR_OK) {
		PrintError(error);
	}

	error = ((BusManager*)m_dummy_rgb_fp)->RegisterCallback(busEventCallbackRemoval,
		REMOVAL,
		NULL,
		&_cb_handle
	);

	if (error != PGRERROR_OK) {
		PrintError(error);
	}
}


BOOL CCamera::FindCamera(std::map<std::string, std::map<int32_t, std::string>>  *cameras){

	Error		 error;
	unsigned int numCameras;

	if (cameras == NULL) {
		return FALSE;
	}

	if (CCamera::m_dummy_rgb_fp == NULL) {
		CCamera::m_dummy_rgb_fp = new BusManager;
	}

	error = ((BusManager*)m_dummy_rgb_fp)->GetNumOfCameras(&numCameras);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return FALSE;
	}

	Camera* p_camera = NULL;
	// Connect to all detected cameras and attempt to set them to
	// the same video mode and frame rate
	for (unsigned int uiCamera = 0; uiCamera < numCameras; uiCamera++)
	{
		p_camera = new Camera();

		PGRGuid guid;
		error = ((BusManager*)m_dummy_rgb_fp)->GetCameraFromIndex(uiCamera, &guid);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return FALSE;
		}

		// Connect to a camera
		error = p_camera->Connect(&guid);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return FALSE;
		}

		// Get the camera information
		CameraInfo camInfo;
		error = p_camera->GetCameraInfo(&camInfo);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return FALSE;
		}

		PrintCameraInfo(&camInfo);

		//camInfo.ipAddress
		std::string ip;
		int i = 0;
		for (; i < 3; ++i) {
			ip.append(std::to_string(camInfo.ipAddress.octets[i]) + ".");
		}
		ip.append(std::to_string(camInfo.ipAddress.octets[i]));

		std::map<int32_t, string> the_singal_camera;
		the_singal_camera[uiCamera] = std::string();

		(*cameras)[ip] = the_singal_camera;

		p_camera->Disconnect();
		delete p_camera;
		p_camera = NULL;
	}

	return TRUE;
}


BOOL CCamera::SetExposureTime(double microseconds) {
	return TRUE;
}

BOOL CCamera::SetGainBySensorAll(double val) {
	Property prop;
	prop.type = GAIN;
	prop.absControl = true;
	prop.onePush = false;
	prop.onOff = true;
	prop.autoManualMode = false;
	prop.absValue = val;
	((Camera*)m_AcqDevice)->SetProperty(&prop);
	return TRUE;
}

BOOL CCamera::SetGainBySensorAnalog(double val) {
	return TRUE;
}

BOOL CCamera::SetGainBySensorDigital(double val) {
	return TRUE;
}

BOOL CCamera::SetFrameRate(double val) {
	Property prop;
	prop.type = FRAME_RATE;
	prop.absControl = true;
	prop.onePush = false;
	prop.onOff = true;
	prop.autoManualMode = false;
	prop.absValue = val;
	((Camera*)m_AcqDevice)->SetProperty(&prop);

	return TRUE;
}

BOOL CCamera::EnableTurboTransfer(BOOL bEnable) {
	return TRUE;
}

BOOL CCamera::SetImageWidth(INT64 width) {
	return TRUE;
}


BOOL CCamera::SetImageHeight(INT64 height) {
	return TRUE;
}

BOOL CCamera::SetOffsetX(INT64 offset_x) {
	return TRUE;
}
BOOL CCamera::SetOffsetY(INT64 offset_y) {
	return TRUE;
}


BOOL CCamera::SetPixelFormat(const char* val, int len) { //BayerRG8 BayerRG10
	return TRUE;
}


double CCamera::GetExposureTime() {
	Property prop;
	prop.type = SHUTTER;
	((Camera*)m_AcqDevice)->GetProperty(&prop);
	return prop.absValue;
}

double CCamera::GetGainBySensorAll() {
	Property prop;
	prop.type = GAIN;
	((Camera*)m_AcqDevice)->GetProperty(&prop);
	return prop.absValue;
}


double CCamera::GetGainBySensorAnalog() {
	return TRUE;
}

double CCamera::GetGainBySensorDigital() {
	return TRUE;
}

double CCamera::GetFrameRate() {
	Property prop;
	prop.type = FRAME_RATE;
	((Camera*)m_AcqDevice)->GetProperty(&prop);
	return prop.absValue;
}

INT64 CCamera::GetHeightMax() {
	return TRUE;
}

INT64 CCamera::GetWidthMax() {
	return TRUE;
}

INT64	CCamera::GetImageWidth() {
	int width = 0;
	Error error;
	Image rawImage;
	// Start capturing images
	error = ((Camera*)m_AcqDevice)->StartCapture();
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		fprintf(stdout, "%s StartCapture failed!\n", __FUNCTION__);
		goto end;
	}

	for (int imageCnt = 0; imageCnt < 10; imageCnt++)
	{
		// Retrieve an image
		error = ((Camera*)m_AcqDevice)->RetrieveBuffer(&rawImage);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			goto end;
		}
		else {
			width = rawImage.GetCols();
			break;
		}
	}

end:
	((Camera*)m_AcqDevice)->StopCapture();
	return width;
}

INT64	CCamera::GetImageHeight() {
	Error error;
	int height = 0;
	Image rawImage;
	// Start capturing images
	error = ((Camera*)m_AcqDevice)->StartCapture();
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		fprintf(stdout, "%s StartCapture failed!\n", __FUNCTION__);
		goto end;
	}

	for (int imageCnt = 0; imageCnt < 10; imageCnt++)
	{
		// Retrieve an image
		error = ((Camera*)m_AcqDevice)->RetrieveBuffer(&rawImage);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			goto end;
		}
		else {
			height = rawImage.GetRows();
			break;
		}
	}

end:
	((Camera*)m_AcqDevice)->StopCapture();
	return rawImage.GetCols();
}

INT64 CCamera::GetOffsetX() {
	return TRUE;
}
INT64 CCamera::GetOffsetY() {
	return TRUE;
}

BOOL CCamera::IsEnabledTurboTransfer() {
	return TRUE;
}

BOOL CCamera::GetUserDefinedName(char* buf, int len) {
	return TRUE;
}
const char*	CCamera::GetUserDefinedName() {
	return NULL;
}


BOOL CCamera::GetDeviceSerialNumber(char* val, int len) {
	return TRUE;
}


BOOL CCamera::SaveFeatures(const char* configFile) {
	return TRUE;
}

BOOL  CCamera::LoadFeatures(const char *configFile) {
	return TRUE;
}


double CCamera::GetGrabFPS() {
	return ((CFPSCounter*)m_GrabFPSCounter)->GetFPS();
}
double CCamera::GetProcessFPS() {
	return TRUE;
}

void  CCamera::DumpRawImage() {
	if (m_dummy_bayer_fp)
		return;
	std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	char file_name[100] = {0};
	snprintf(file_name, sizeof file_name, "%lld.raw", now.count());
	m_dummy_bayer_fp = fopen(file_name, "wb");
}

void   CCamera::DumpRgbImage() {
}


BOOL CCamera::GetPixelFormat(char* buf, int len){
	return TRUE;
}




BOOL CCamera::GetFeatureRange(const char* featureName, double* min, double* max) {
	return TRUE;
}

BOOL CCamera::GetExposureTimeRange(double* min, double* max) {
	return TRUE;
}

BOOL CCamera::GetGainBySensorAnalogRange(double* min, double* max) {
	return TRUE;
}

BOOL CCamera::GetGainBySensorDigitalRange(double* min, double* max) {
	return TRUE;
}

BOOL CCamera::GetFrameRateRange(double* min, double* max) {
	return TRUE;
}

BOOL CCamera::GetGainBySensorAllRange(double* min, double* max) {
	return TRUE;
}

BOOL CCamera::EnableColorConvert(BOOL bEnable) {
	return TRUE;
}

BOOL CCamera::GetCurrentIPAddress(char* val, int len) {
	strcpy(val, (char*)m_Buffers);
	return TRUE;
}

BOOL CCamera::GetDeviceModelName(char* val, int len) {
	return TRUE;
}

