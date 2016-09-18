
#include "Camera.h"
#include "SapMyProcessing.h"


#include "SapClassBasic.h"
#include "SapManager.h"
#include "FPSCounter.h"


#define CORSERVER_MAX_STRLEN 30
#define MAX_SERIVAL_NUM_LEN CORSERVER_MAX_STRLEN
#define IMAGE_WIDTH  1280
#define IMAGE_HEIGHT 720

void XferCallback(SapXferCallbackInfo *pInfo);
void ProCallback(SapProCallbackInfo *pInfo);
void SapManCallbackMy(SapManCallbackInfo *info);

void* CCamera::m_dummy_rgb_fp = NULL;
CameraConnectStatusChanged CCamera::m_status_changed_cb = NULL;
void* CCamera::m_status_changed_cb_ctx = NULL;

CCamera::CCamera(const char* camera_ip) :m_AcqDevice(NULL), m_Buffers(NULL), m_Xfer(NULL), m_sink_bayer_cb(NULL), m_ctx0(NULL),
m_sink_rgb_cb(NULL), m_ctx1(NULL), m_dummy_bayer_fp(NULL), /*m_dummy_rgb_fp(NULL),*/ m_bEnableColorConvert(FALSE), \
m_connection_status(UNKNOWN), m_grabbing(FALSE), m_reconnect_flag(FALSE), m_last_is_grabbing(FALSE), m_last_is_connected(FALSE) {
	std::map<std::string, std::map<int32_t, std::string>>  cams;
	FindCamera(&cams);

	if (cams.find(camera_ip) != cams.end()) {
		int index = cams[camera_ip].begin()->first;
		std::string serv_name = cams[camera_ip].begin()->second;
		
		SapManager::SetDisplayStatusMode(SapManager::StatusLog);//FIXME: set err report mode 

		SapLocation loc(serv_name.c_str(), index);
		m_AcqDevice = new SapAcqDevice(loc);
		m_Buffers = new SapBufferWithTrash(15, (SapAcqDevice*)m_AcqDevice);
		m_ColorConv = new SapColorConversion((SapBuffer*)m_Buffers);
		m_Xfer = new SapAcqDeviceToBuf((SapAcqDevice*)m_AcqDevice, (SapBuffer*)m_Buffers, XferCallback, this);
		m_Pro = new SapMyProcessing((SapBuffer*)m_Buffers, (SapColorConversion*)m_ColorConv, ProCallback, this);
		memset(m_UserDefinedName, 0x00, sizeof m_UserDefinedName);
		memset(m_AcqServerName, 0x00, sizeof m_AcqServerName);

		//init
		strcpy(m_AcqServerName, loc.GetServerName());
		m_index = index;

		m_GrabFPSCounter = new CFPSCounter;
		m_ProcessFPSCounter = new CFPSCounter;

	}
	else {
		char info[100];
		sprintf(info, "%s CCamera(%s) new failed!\n", __FUNCTION__, camera_ip);
		std::exception e(info);
		throw e;
	}
}

CCamera::CCamera(const char* serverName, int index) :m_AcqDevice(NULL), m_Buffers(NULL), m_Xfer(NULL), m_sink_bayer_cb(NULL), m_ctx0(NULL),
		m_sink_rgb_cb(NULL), m_ctx1(NULL), m_dummy_bayer_fp(NULL), /*m_dummy_rgb_fp(NULL),*/ m_bEnableColorConvert(FALSE), \
	m_connection_status(UNKNOWN) , m_grabbing(FALSE), m_reconnect_flag(FALSE), m_last_is_grabbing(FALSE), m_last_is_connected(FALSE){

	SapManager::SetDisplayStatusMode(SapManager::StatusLog);//FIXME: set err report mode 

	SapLocation loc(serverName, index);
	m_AcqDevice = new SapAcqDevice(loc);
	m_Buffers = new SapBufferWithTrash(15, (SapAcqDevice*)m_AcqDevice);
	m_ColorConv = new SapColorConversion((SapBuffer*)m_Buffers);
	m_Xfer = new SapAcqDeviceToBuf((SapAcqDevice*)m_AcqDevice, (SapBuffer*)m_Buffers, XferCallback, this);
	m_Pro = new SapMyProcessing((SapBuffer*)m_Buffers, (SapColorConversion*)m_ColorConv, ProCallback, this);
	memset(m_UserDefinedName, 0x00, sizeof m_UserDefinedName);
	memset(m_AcqServerName, 0x00, sizeof m_AcqServerName);

	//init
	strcpy(m_AcqServerName, loc.GetServerName());
	m_index = index;

	m_GrabFPSCounter = new CFPSCounter;
	m_ProcessFPSCounter = new CFPSCounter;

	//g_err_fp = fopen("log.txt", "w");
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
		delete m_Pro;
		delete m_Xfer;
		delete m_ColorConv;
		delete m_Buffers;
		delete m_AcqDevice;

		delete m_GrabFPSCounter;
		delete m_ProcessFPSCounter;

		//fclose(g_err_fp);
	}

void CCamera::Start() {
	if (!m_grabbing) {
		if (((SapTransfer*)m_Xfer)->Grab()) {
			m_last_is_grabbing = m_grabbing = TRUE;
			fprintf(stdout, "grab start!\n");
			m_lost = 0;
		}
	}
}

void CCamera::Stop() {
	if (m_grabbing) {
		fprintf(stdout, "1%s!\n", __FUNCTION__);
		((SapTransfer*)m_Xfer)->Abort();
		fprintf(stdout, "2%s!\n", __FUNCTION__);
		m_grabbing = FALSE;
	}
}

BOOL CCamera::CreateDevice() {
	// Create acquisition object
	if (m_AcqDevice && !*(SapAcqDevice*)m_AcqDevice && !((SapAcqDevice*)m_AcqDevice)->Create()) {
		DestroyDevice();
		return FALSE;
	}


	//int fc = 0;
	//((SapAcqDevice*)m_AcqDevice)->GetFileCount(&fc);
	//char file_name[100] = { 0 };
	//for (int i = 0; i < fc; ++i) {
	//
	//	((SapAcqDevice*)m_AcqDevice)->GetFileNameByIndex(i, file_name, sizeof file_name);
	//}

	//BOOL ret = ((SapAcqDevice*)m_AcqDevice)->SetConfigName("UserSet1");

	return m_last_is_connected = TRUE;
}
BOOL CCamera::CreateOtherObjects(){
	SapBayer::Align lBayerAlign = SapBayer::AlignRGGB;
	if (m_AcqDevice) {
		//check bayer availability
		if (((SapAcqDevice*)m_AcqDevice)->IsRawBayerOutput() == FALSE) {
			fprintf(stdout, "Software bayer not supported on this camera!\n");
			DestroyOtherObjects();
			return FALSE;
		}

		//FIXME:
		char sPixelFormatValue[50] = "BayerRG8";
		BOOL bStatus = FALSE;
		bStatus = SetPixelFormat(sPixelFormatValue, strlen(sPixelFormatValue));
		lBayerAlign = SapBayer::AlignRGGB;
	}
	// Enable/Disable bayer conversion
	// This call may require to modify the acquisition output format.
	// For this reason, it has to be done after creating the acquisition object but before
	// creating the output buffer object.
	if (m_bEnableColorConvert){
		if (m_ColorConv && ((SapColorConversion*)m_ColorConv)->Enable(TRUE, FALSE)) {
			
		}else {
			fprintf(stdout, "Color Convert Enable(TRUE, FALSE) failed!\n");
		}
	}



	// Create buffer objects
	if (m_Buffers && !*(SapBuffer*)m_Buffers) {
		//if (!((SapBuffer*)m_Buffers)->SetType(SapBuffer::TypeContiguous)) {
		//	fprintf(stdout, "%s SetType failed\n", __FUNCTION__);
		//	DestroyOtherObjects();
		//	return FALSE;
		//}

		if (!((SapBuffer*)m_Buffers)->Create()) {
			DestroyOtherObjects();
			return FALSE;
		}
		// Clear all buffers
		((SapBuffer*)m_Buffers)->Clear();
	}

	// Create bayer object
	if (m_ColorConv && !*(SapColorConversion*)m_ColorConv && !((SapColorConversion*)m_ColorConv)->Create()) {
		DestroyOtherObjects();
		return FALSE;
	}
	else {
		if (((SapColorConversion*)m_ColorConv)->IsSoftwareEnabled()) {
			((SapColorConversion*)m_ColorConv)->SetOutputFormat(SapFormatRGB888);//fixed?
		}
	}

	// Set the align
	((SapColorConversion*)m_ColorConv)->SetAlign(lBayerAlign);

	// Create transfer object
	if (m_Xfer && !*(SapTransfer*)m_Xfer) {
		if (!((SapTransfer*)m_Xfer)->Create()) {
			DestroyOtherObjects();
			return FALSE;
		}
		((SapTransfer*)m_Xfer)->SetAutoEmpty(TRUE);
		//((SapTransfer*)m_Xfer)->Init(TRUE); // initialize tranfer object and reset source/destination index
	}


	// Create processing object
	if (m_Pro && !*(SapProcessing*)m_Pro) {
		if (!((SapProcessing*)m_Pro)->Create()) {
			DestroyOtherObjects();
			return FALSE;
		}
		//((SapProcessing*)m_Pro)->SetAutoEmpty(TRUE);
	}
	return TRUE;
}



void CCamera::DestroyDevice() {
	// Destroy acquisition object
	if (m_AcqDevice && *(SapAcqDevice*)m_AcqDevice) ((SapAcqDevice*)m_AcqDevice)->Destroy();
}

void CCamera::DestroyOtherObjects() {

	// Destroy processing object
	if (m_Pro && *(SapProcessing*)m_Pro) 
		((SapProcessing*)m_Pro)->Destroy();
	fprintf(stdout, "%s 1\n", __FUNCTION__);
	// Destroy transfer object
	if (m_Xfer && *(SapTransfer*)m_Xfer) 
		((SapTransfer*)m_Xfer)->Destroy();
	fprintf(stdout, "%s 2\n", __FUNCTION__);

	// Destroy view object
	//if (m_View && *m_View) m_View->Destroy();
	fprintf(stdout, "%s 3\n", __FUNCTION__);
	// Destroy bayer object
	if (m_ColorConv && *(SapColorConversion*)m_ColorConv) 
		((SapColorConversion*)m_ColorConv)->Destroy();
	fprintf(stdout, "%s 4\n", __FUNCTION__);
	// Destroy buffer object
	if (m_Buffers && *(SapBuffer*)m_Buffers) 
		((SapBuffer*)m_Buffers)->Destroy();
	fprintf(stdout, "%s 5\n", __FUNCTION__);
}

void CCamera::RegisterConnectionEventCallback(CameraConnectStatusChanged cb, void* ctx) {
	m_status_changed_cb = cb;
	m_status_changed_cb_ctx = ctx;
	SapManager::RegisterServerCallback(CORMAN_VAL_EVENT_TYPE_SERVER_DISCONNECTED | CORMAN_VAL_EVENT_TYPE_SERVER_CONNECTED, SapManCallbackMy, NULL);
}


BOOL CCamera::FindCamera(std::map<std::string, std::map<int32_t, std::string>>  *cameras){
	
	
	int serverCount = SapManager::GetServerCount();
	if (serverCount <= 1) {
		return FALSE;
	}

	printf("%s GetServerCount:%d\n",__FUNCTION__, serverCount);
	//cameras->clear();
	for (int serverIndex = 1; serverIndex < serverCount; ++serverIndex) {
		char serverName[CORSERVER_MAX_STRLEN];
		SapManager::GetServerName(serverIndex, serverName, sizeof(serverName));//FIXME!!!
		int cameraCount = SapManager::GetResourceCount(serverName, SapManager::ResourceAcqDevice);

		for (int cameraIndex = 0; cameraIndex < cameraCount; cameraIndex++)
		{
			char cameraName[CORPRM_GETSIZE(CORACQ_PRM_LABEL)];
			SapManager::GetResourceName(serverName, SapManager::ResourceAcqDevice, cameraIndex, cameraName, sizeof(cameraName));

			//"Module #1, not available"
			if (std::string(cameraName).find("not available") != std::string::npos) {
				fprintf(stdout, "%s, a camera not available!\n", __FUNCTION__);
				continue;
			}

			printf("%d: %s\n", cameraIndex + 1, cameraName);

			std::map<int, std::string> tmp;
			tmp[cameraIndex] = std::string(serverName);//UserName as camera's ip

			if (strlen(cameraName) > 0) {
				cameras->insert(std::make_pair(std::string(cameraName), tmp));
			}

			//char camera_ip[30] = { 0 };
			//CCamera* camera = new CCamera(serverName, cameraIndex);
			//if (camera->CreateDevice()) {
			//	camera->GetCurrentIPAddress(camera_ip, sizeof camera_ip);
			//	camera->DestroyDevice();

			//	if (strlen(camera_ip) > 0) {
			//		cameras->insert(std::make_pair(std::string(camera_ip), tmp));
			//	}
			//}
			//delete camera;
		}
	}

	return TRUE;
}




BOOL CCamera::SetExposureTime(double microseconds) {
	return ((SapAcqDevice*)m_AcqDevice)->SetFeatureValue("ExposureTime", microseconds);
}

BOOL CCamera::SetGainBySensorAll(double val) {
	((SapAcqDevice*)m_AcqDevice)->SetFeatureValue("GainSelector", "SensorAll");//FIXME:
	return ((SapAcqDevice*)m_AcqDevice)->SetFeatureValue("Gain", val);
}

BOOL CCamera::SetGainBySensorAnalog(double val) {

	((SapAcqDevice*)m_AcqDevice)->SetFeatureValue("GainSelector", "SensorAnalog");//FIXME:
	return ((SapAcqDevice*)m_AcqDevice)->SetFeatureValue("Gain", val);
}

BOOL CCamera::SetGainBySensorDigital(double val) {
	((SapAcqDevice*)m_AcqDevice)->SetFeatureValue("GainSelector", "SensorDigital");//FIXME:
	return ((SapAcqDevice*)m_AcqDevice)->SetFeatureValue("Gain", val);
}

BOOL CCamera::SetFrameRate(double val) {

	return ((SapAcqDevice*)m_AcqDevice)->SetFeatureValue("AcquisitionFrameRate", val);
}

BOOL CCamera::EnableTurboTransfer(BOOL bEnable) {
	return ((SapAcqDevice*)m_AcqDevice)->SetFeatureValue("turboTransferEnable", bEnable);
}

BOOL CCamera::SetImageWidth(INT64 width) {

	return ((SapAcqDevice*)m_AcqDevice)->SetFeatureValue("Width", width) ;
}


BOOL CCamera::SetImageHeight(INT64 height) {

	return ((SapAcqDevice*)m_AcqDevice)->SetFeatureValue("Height", height);
}

BOOL CCamera::SetOffsetX(INT64 offset_x) {

	return ((SapAcqDevice*)m_AcqDevice)->SetFeatureValue("OffsetX", offset_x);
}
BOOL CCamera::SetOffsetY(INT64 offset_y) {

	return ((SapAcqDevice*)m_AcqDevice)->SetFeatureValue("OffsetY", offset_y);
}


BOOL CCamera::SetPixelFormat(const char* val, int len) { //BayerRG8 BayerRG10
												   // determine the bayer alignement with the PixelFormat feature

	BOOL bPixelFormatAvailable = FALSE;
	if (0 != len && NULL != val) {
		return ((SapAcqDevice*)m_AcqDevice)->IsFeatureAvailable("PixelFormat", &bPixelFormatAvailable) && bPixelFormatAvailable && ((SapAcqDevice*)m_AcqDevice)->SetFeatureValue("PixelFormat", val);
	}
	return FALSE;
}


double CCamera::GetExposureTime() {
	double val = .0f;
	if (((SapAcqDevice*)m_AcqDevice)->GetFeatureValue("ExposureTime", &val) == FALSE) {
		fprintf(stdout, "%s Failed!\n", __FUNCTION__);
	}
	return val;
}

double CCamera::GetGainBySensorAll() {
	double val = .0f;
	((SapAcqDevice*)m_AcqDevice)->SetFeatureValue("GainSelector", "SensorAll");//FIXME:
	if (((SapAcqDevice*)m_AcqDevice)->GetFeatureValue("Gain", &val) == FALSE) {
		fprintf(stdout, "%s Failed!\n", __FUNCTION__);
	}
	return val;
}


double CCamera::GetGainBySensorAnalog() {
	double val = .0f;
	((SapAcqDevice*)m_AcqDevice)->SetFeatureValue("GainSelector", "SensorAnalog");//FIXME:
	if (((SapAcqDevice*)m_AcqDevice)->GetFeatureValue("Gain", &val) == FALSE) {
		fprintf(stdout, "%s Failed!\n", __FUNCTION__);
	}
	return val;
}

double CCamera::GetGainBySensorDigital() {
	double val = .0f;
	((SapAcqDevice*)m_AcqDevice)->SetFeatureValue("GainSelector", "SensorDigital");//FIXME:
	if (((SapAcqDevice*)m_AcqDevice)->GetFeatureValue("Gain", &val) == FALSE) {
		fprintf(stdout, "%s Failed!\n", __FUNCTION__);
	}
	return val;
}

double CCamera::GetFrameRate() {
	double val = .0f;
	if (((SapAcqDevice*)m_AcqDevice)->GetFeatureValue("AcquisitionFrameRate", &val) == FALSE) {
		fprintf(stdout, "%s Failed!\n", __FUNCTION__);
	}
	return val;
}

INT64 CCamera::GetHeightMax() {
	INT64 val = 0;
	if (((SapAcqDevice*)m_AcqDevice)->GetFeatureValue("HeightMax", &val) == FALSE) {
		fprintf(stdout, "%s Failed!\n", __FUNCTION__);
	}
	return val;
}

INT64 CCamera::GetWidthMax() {
	INT64 val = 0;
	if (((SapAcqDevice*)m_AcqDevice)->GetFeatureValue("WidthMax", &val) == FALSE) {
		fprintf(stdout, "%s Failed!\n", __FUNCTION__);
	}
	return val;
}

INT64	CCamera::GetImageWidth() {
	INT64 val = 0;
	if (((SapAcqDevice*)m_AcqDevice)->GetFeatureValue("Width", &val) == FALSE) {
		fprintf(stdout, "%s Failed!\n", __FUNCTION__);
	}
	return val;
}

INT64	CCamera::GetImageHeight() {
	INT64 val = 0;
	if (((SapAcqDevice*)m_AcqDevice)->GetFeatureValue("Height", &val) == FALSE) {
		fprintf(stdout, "%s Failed!\n", __FUNCTION__);
	}
	return val;
}

INT64 CCamera::GetOffsetX() {
	INT64 val = 0;
	if (((SapAcqDevice*)m_AcqDevice)->GetFeatureValue("OffsetX", &val) == FALSE) {
		fprintf(stdout, "%s Failed!\n", __FUNCTION__);
	}
	return val;
}
INT64 CCamera::GetOffsetY() {
	INT64 val = 0;
	if (((SapAcqDevice*)m_AcqDevice)->GetFeatureValue("OffsetY", &val) == FALSE) {
		fprintf(stdout, "%s Failed!\n", __FUNCTION__);
	}
	return val;
}

BOOL CCamera::IsEnabledTurboTransfer() {
	BOOL val = 0;
	if (((SapAcqDevice*)m_AcqDevice)->GetFeatureValue("turboTransferEnable", &val) == FALSE) {
		fprintf(stdout, "%s Failed!\n", __FUNCTION__);
	}
	return val;
}

BOOL CCamera::GetUserDefinedName(char* buf, int len) {
	fprintf(stdout, "%s \n", __FUNCTION__);
	return  ((SapAcqDevice*)m_AcqDevice)->GetFeatureValue("DeviceUserID", buf, len);
}
const char*	CCamera::GetUserDefinedName() {
	if (strlen(m_UserDefinedName) == 0) {
		if (GetUserDefinedName(m_UserDefinedName, sizeof m_UserDefinedName))
			return m_UserDefinedName;
	}
	return m_UserDefinedName;
}


BOOL CCamera::GetDeviceSerialNumber(char* val, int len) {
	fprintf(stdout, "%s \n", __FUNCTION__);
	return  ((SapAcqDevice*)m_AcqDevice)->GetFeatureValue("DeviceSerialNumber", val, len);
}


BOOL CCamera::SaveFeatures(const char* configFile) {
	if (configFile == NULL) return FALSE;
	
	return  ((SapAcqDevice*)m_AcqDevice)->UpdateFeaturesToDevice() && ((SapAcqDevice*)m_AcqDevice)->SaveFeatures(configFile);
}

BOOL  CCamera::LoadFeatures(const char *configFile) {
	if (configFile == NULL) return FALSE;
	return  ((SapAcqDevice*)m_AcqDevice)->LoadFeatures(configFile);
}


double CCamera::GetGrabFPS() {
	return ((CFPSCounter*)m_GrabFPSCounter)->GetFPS();
}
double CCamera::GetProcessFPS() {
	return ((CFPSCounter*)m_GrabFPSCounter)->GetFPS();
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
	if (m_dummy_rgb_fp)
		return;
	std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	char file_name[100] = { 0 };
	snprintf(file_name, sizeof file_name, "%lld.rgb", now.count());
	m_dummy_rgb_fp = fopen(file_name, "wb");
}


BOOL CCamera::GetPixelFormat(char* buf, int len){
	// determine the bayer alignement with the PixelFormat feature
	BOOL bPixelFormatAvailable = FALSE;
	char sPixelFormatValue[50] = { 0 };
	if (((SapAcqDevice*)m_AcqDevice)->IsFeatureAvailable("PixelFormat", &bPixelFormatAvailable))	{
		if (bPixelFormatAvailable && ((SapAcqDevice*)m_AcqDevice)->GetFeatureValue("PixelFormat", sPixelFormatValue, _countof(sPixelFormatValue)))	{
			snprintf(buf, len, "%s", sPixelFormatValue);
			return TRUE;
		}
	}
	return FALSE;
}



BOOL CCamera::GetFeatureRange(const char* featureName, double* min, double* max) {
	BOOL bAvailable = FALSE;
	BOOL bStatus = FALSE;
	if (featureName && ((SapAcqDevice*)m_AcqDevice)->IsFeatureAvailable(featureName, &bAvailable) && bAvailable) {
		SapFeature feature(this->m_AcqServerName);
		bStatus = feature.Create();
		if (bStatus) {
			bStatus = ((SapAcqDevice*)m_AcqDevice)->GetFeatureInfo(featureName, &feature);
			if (bStatus) {
				bStatus = feature.GetMax(max) && feature.GetMin(min);
			}
		}
		feature.Destroy();
	}
	return bStatus;
}

BOOL CCamera::GetExposureTimeRange(double* min, double* max) {
	const char* pFeatureName = "ExposureTime";
	return GetFeatureRange(pFeatureName, min, max);
}

BOOL CCamera::GetGainBySensorAnalogRange(double* min, double* max) {
	const char* pFeatureName = "Gain";
	((SapAcqDevice*)m_AcqDevice)->SetFeatureValue("GainSelector", "SensorAnalog");//FIXME:
	return GetFeatureRange(pFeatureName, min, max);
}

BOOL CCamera::GetGainBySensorDigitalRange(double* min, double* max) {
	const char* pFeatureName = "Gain";
	((SapAcqDevice*)m_AcqDevice)->SetFeatureValue("GainSelector", "SensorDigital");//FIXME:
	return GetFeatureRange(pFeatureName, min, max);
}

BOOL CCamera::GetFrameRateRange(double* min, double* max) {
	const char* pFeatureName = "AcquisitionFrameRate";
	return GetFeatureRange(pFeatureName, min, max);
}

BOOL CCamera::GetGainBySensorAllRange(double* min, double* max) {
	const char* pFeatureName = "Gain";
	((SapAcqDevice*)m_AcqDevice)->SetFeatureValue("GainSelector", "SensorAll");//FIXME:
	return GetFeatureRange(pFeatureName, min, max);
}

BOOL CCamera::EnableColorConvert(BOOL bEnable) {
	return m_bEnableColorConvert = bEnable;
}

BOOL CCamera::GetCurrentIPAddress(char* val, int len) {
	BOOL bAvailable = FALSE;
	if (0 != len && NULL != val) {
		if (((SapAcqDevice*)m_AcqDevice)->IsFeatureAvailable("GevCurrentIPAddress", &bAvailable) && bAvailable) {
			INT64 ip = 0;
			in_addr addr;
			((SapAcqDevice*)m_AcqDevice)->GetFeatureValue("GevCurrentIPAddress", &ip);
			ulong ip_ul = ntohl(ip);
			memcpy(&addr, &ip_ul, sizeof(ip_ul));//FIXME:
			std::string strIp = inet_ntoa(addr);
			strcpy(val, strIp.c_str());
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CCamera::GetDeviceModelName(char* val, int len) {
	BOOL bAvailable = FALSE;
	if (0 != len && NULL != val) {
		if (((SapAcqDevice*)m_AcqDevice)->IsFeatureAvailable("DeviceModelName", &bAvailable) && bAvailable) {
			((SapAcqDevice*)m_AcqDevice)->GetFeatureValue("DeviceModelName", val, len);
			return TRUE;
		}
	}
	return FALSE;
}

void XferCallback(SapXferCallbackInfo *pInfo) {
	CCamera *camera = (CCamera *)pInfo->GetContext();
	if (camera == NULL) {
		fprintf(stdout, "%s %p\n", __FUNCTION__, camera);
		return;
	}

	

	SapBuffer* pBuffer = NULL;
	pBuffer = ((SapBuffer*)camera->m_Buffers);
	// If grabbing in trash buffer, do not display the image, update the
	// appropriate number of frames on the status bar instead
	if (pInfo->IsTrash() ) {
		((CFPSCounter*)camera->m_GrabFPSCounter)->statistics(__FUNCTION__, FALSE, FALSE);
		fprintf(stdout, "Frames acquired in trash buffer: %03d\n", pInfo->GetEventCount());
		//pBuffer->Next();
	}
	else {
	 
		//((SapProcessing*)camera->m_Pro)->Execute();
#if 1
		PUINT8 pData = NULL;
		
		SapBuffer::State state;
		int index = pBuffer->GetIndex();
		pBuffer->GetState(index, &state);
		if (state != SapBuffer::StateFull) {
			//fprintf(stdout, "%s StateOverflow \n", __FUNCTION__);
			return;
		}
		BOOL status = pBuffer->GetAddress(index,(void**)&pData);
		if (status && pData) {
			//fprintf(stdout, "%s %d %x\r\n", __FUNCTION__, status, index);
			if (camera->m_sink_bayer_cb) {
				int64_t ret = camera->m_sink_bayer_cb(pData, pBuffer->GetWidth() * pBuffer->GetHeight(), camera->m_lost,camera->m_ctx0);//FIXME: bufferlen 
				//fprintf(stdout, "%s send %lld bytes!\r\n", __FUNCTION__,  ret);
			}
			pBuffer->ReleaseAddress(pData);
			((CFPSCounter*)camera->m_GrabFPSCounter)->statistics(__FUNCTION__,FALSE);
		}
		else {
			fprintf(stdout, "%s %d %p\r\n", __FUNCTION__, status, pData);
			((CFPSCounter*)camera->m_GrabFPSCounter)->statistics(__FUNCTION__, FALSE,FALSE);
		}
#endif
		
	}
}

//
// This function is called each time a buffer has been processed by the processing object
//
void ProCallback(SapProCallbackInfo *pInfo) {
	CCamera *camera = (CCamera *)pInfo->GetContext();
	if (camera == NULL)
		return;
#if 0
	// Check if color conversion was enabled and if it's been done by software
	if (((SapColorConversion*)camera->m_ColorConv)->IsSoftwareEnabled())
	{
		// Show current buffer index and execution time in millisecond
		//fprintf(stdout, "%s Color conversion = %5.2f ms\n", __FUNCTION__, camera->m_Pro->GetTime());

		SapBuffer* pBuffer = NULL;
		pBuffer = ((SapColorConversion*)camera->m_ColorConv)->GetOutputBuffer();
		PUINT8 pData = NULL;
		BOOL status = pBuffer->GetAddress(pBuffer->GetIndex(), (void**)&pData);

		if (status) {
			//fprintf(stdout, "%s %d %x\r\n", __FUNCTION__, status, pData);
			if (camera->m_sink_rgb_cb) {
				camera->m_sink_rgb_cb(pData, pBuffer->GetPitch() * pBuffer->GetHeight(), camera->m_lost, camera->m_ctx1);//FIXME: bufferlen 
			}

			if (camera->m_dummy_rgb_fp) {
				fwrite(pData, 1, pBuffer->GetPitch()  * pBuffer->GetHeight(), (FILE*)camera->m_dummy_rgb_fp);
				fclose((FILE*)(camera->m_dummy_rgb_fp));
				camera->m_dummy_rgb_fp = NULL;
			}
			((CFPSCounter*)camera->m_ProcessFPSCounter)->statistics(__FUNCTION__, FALSE);
		}

		pBuffer->ReleaseAddress(pData);
	}
#endif


	SapBuffer* pBuffer = NULL;
	pBuffer = ((SapColorConversion*)camera->m_ColorConv)->GetOutputBuffer();

	PUINT8 pData = NULL;
	int index = pBuffer->GetIndex();
	BOOL status = pBuffer->GetAddress(index, (void**)&pData);
	if (status && pData) {
		//fprintf(stdout, "%s %d %x\r\n", __FUNCTION__, status, index);
		if (camera->m_sink_bayer_cb) {
			int64_t ret = camera->m_sink_bayer_cb(pData, pBuffer->GetWidth() * pBuffer->GetHeight(), camera->m_lost, camera->m_ctx0);//FIXME: bufferlen 
																																		//fprintf(stdout, "%s send %lld bytes!\r\n", __FUNCTION__,  ret);
		}
		pBuffer->ReleaseAddress(pData);
		((CFPSCounter*)camera->m_GrabFPSCounter)->statistics(__FUNCTION__, FALSE);
	}
	else {
		fprintf(stdout, "%s %d %p\r\n", __FUNCTION__, status, pData);
		((CFPSCounter*)camera->m_GrabFPSCounter)->statistics(__FUNCTION__, FALSE, FALSE);
	}

}

void SapManCallbackMy(SapManCallbackInfo *pInfo) {
	int type = pInfo->GetEventType();
	int status = (type == SapManager::EventServerDisconnected) ? 0 : 1;

	

	char serverName[100];
	SapManager::GetServerName(pInfo->GetServerIndex(), serverName, sizeof(serverName));

	int cameraIndex = 0;
	char cameraName[100] = {0};
	SapManager::GetResourceName(serverName, SapManager::ResourceAcqDevice, pInfo->GetResourceIndex(), cameraName, sizeof(cameraName));

	if (CCamera::m_status_changed_cb) {
		CCamera::m_status_changed_cb(cameraName, status, CCamera::m_status_changed_cb_ctx);
	}

	return;



	CCamera* camera = (CCamera*)pInfo->GetContext();

	if (camera == NULL) {
		return;
	}

	if (pInfo->GetResourceIndex() != camera->m_index || strcmp(serverName, camera->m_AcqServerName) != 0) {
		return;
	}

	fprintf(stdout, "Camera(%d,%s) status:%d.(Disconnected(%d),Connected(%d))\n", pInfo->GetResourceIndex(), serverName, type, SapManager::EventServerDisconnected, SapManager::EventServerConnected);

	if (type == SapManager::EventServerDisconnected) {
		camera->Stop();
	}
	else if (type == SapManager::EventServerConnected) {
		char val[30] = {0};
		camera->GetCurrentIPAddress(val, sizeof val);
		fprintf(stdout, "%s %s Connected!!!\n", __FUNCTION__, val);
		if (camera->m_last_is_connected) {
			fprintf(stdout, "%s m_last_is_connected %d\n", __FUNCTION__, camera->m_last_is_connected);
		}
		if (camera->m_last_is_grabbing) {
			fprintf(stdout, "%s m_last_is_grabbing %d\n", __FUNCTION__, camera->m_last_is_grabbing);
			camera->Start();
		}
	}
	else {
		fprintf(stdout, "#######%s status %d\n", __FUNCTION__, type);
	}
	

}

