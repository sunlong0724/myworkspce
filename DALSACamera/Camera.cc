
#include "Camera.h"
#include "SapMyProcessing.h"


#define CORSERVER_MAX_STRLEN 30
#define MAX_SERIVAL_NUM_LEN CORSERVER_MAX_STRLEN
#define IMAGE_WIDTH  1280
#define IMAGE_HEIGHT 720

void XferCallback(SapXferCallbackInfo *pInfo);
void ProCallback(SapProCallbackInfo *pInfo);
void SapManCallbackMy(SapManCallbackInfo *info);


CCamera::CCamera(const char* serverName, int index) :m_AcqDevice(NULL), m_Buffers(NULL), m_Xfer(NULL), m_sink_bayer_cb(NULL), m_ctx0(NULL),
		m_sink_rgb_cb(NULL), m_ctx1(NULL), m_dummy_bayer_fp(NULL), m_dummy_rgb_fp(NULL), m_bEnableColorConvert(FALSE), m_connection_status(UNKNOWN) , m_grabbing(FALSE), m_reconnect_flag(FALSE) {

	SapManager::SetDisplayStatusMode(SapManager::StatusLog);//FIXME: set err report mode 

	SapLocation loc(serverName, index);
	m_AcqDevice = new SapAcqDevice(loc);
	m_Buffers = new SapBufferWithTrash(5, m_AcqDevice);
	m_ColorConv = new SapColorConversion(m_Buffers);
	m_Xfer = new SapAcqDeviceToBuf(m_AcqDevice, m_Buffers, XferCallback, this);
	m_Pro = new SapMyProcessing(m_Buffers, m_ColorConv, ProCallback, this);
	memset(m_UserDefinedName, 0x00, sizeof m_UserDefinedName);
	memset(m_AcqServerName, 0x00, sizeof m_AcqServerName);

	//init
	strcpy(m_AcqServerName, loc.GetServerName());
	m_index = index;

	g_err_fp = fopen("log.txt", "w");
}

void CCamera::SetSinkBayerDataCallback(SinkDataCallback cb, void* ctx) {
	m_sink_bayer_cb = cb;
	m_ctx0 = ctx;
}

void CCamera::SetSinkRGBDataCallback(SinkDataCallback cb, void* ctx) {
	m_sink_rgb_cb = cb;
	m_ctx1 = ctx;
}

CCamera::~CCamera() {
		delete m_Pro;
		delete m_Xfer;
		delete m_ColorConv;
		delete m_Buffers;
		delete m_AcqDevice;
		fclose(g_err_fp);
	}

void CCamera::Start() {
	if (!m_grabbing) {
		if (m_Xfer->Grab()) {
			m_last_is_grabbing = m_grabbing = TRUE;
			fprintf(stdout, "grab start!\n");
		}
	}
}

void CCamera::Stop() {
	if (m_grabbing) {
		m_Xfer->Abort();
		m_grabbing = FALSE;
	}
}

BOOL CCamera::CreateDevice() {
	// Create acquisition object
	if (m_AcqDevice && !*m_AcqDevice && !m_AcqDevice->Create()) {
		DestroyDevice();
		return FALSE;
	}
	return m_last_is_connected = TRUE;
}
BOOL CCamera::CreateOtherObjects(){
	SapBayer::Align lBayerAlign = SapBayer::AlignRGGB;
	if (m_AcqDevice) {
		//check bayer availability
		if (m_AcqDevice->IsRawBayerOutput() == FALSE) {
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
		if (m_ColorConv && m_ColorConv->Enable(TRUE, FALSE)) {
			
		}else {
			fprintf(stdout, "Color Convert Enable(TRUE, FALSE) failed!\n");
		}
	}
	

	// Create buffer objects
	if (m_Buffers && !*m_Buffers) {
		if (!m_Buffers->Create()) {
			DestroyOtherObjects();
			return FALSE;
		}
		// Clear all buffers
		m_Buffers->Clear();
	}

	// Create bayer object
	if (m_ColorConv && !*m_ColorConv && !m_ColorConv->Create()) {
		DestroyOtherObjects();
		return FALSE;
	}
	else {
		if (m_ColorConv->IsSoftwareEnabled()) {
			m_ColorConv->SetOutputFormat(SapFormatRGB888);//fixed?
		}
	}

	// Set the align
	m_ColorConv->SetAlign(lBayerAlign);

	// Create transfer object
	if (m_Xfer && !*m_Xfer) {
		if (!m_Xfer->Create()) {
			DestroyOtherObjects();
			return FALSE;
		}
		m_Xfer->SetAutoEmpty(FALSE);
	}

	// Create processing object
	if (m_Pro && !*m_Pro) {
		if (!m_Pro->Create()) {
			DestroyOtherObjects();
			return FALSE;
		}
		m_Pro->SetAutoEmpty(TRUE);
	}
	return TRUE;
}

void CCamera::DestroyDevice() {
	// Destroy acquisition object
	if (m_AcqDevice && *m_AcqDevice) m_AcqDevice->Destroy();
}

void CCamera::DestroyOtherObjects() {
	// Destroy processing object
	if (m_Pro && *m_Pro) m_Pro->Destroy();

	// Destroy transfer object
	if (m_Xfer && *m_Xfer) m_Xfer->Destroy();

	// Destroy view object
	//if (m_View && *m_View) m_View->Destroy();

	// Destroy bayer object
	if (m_ColorConv && *m_ColorConv) m_ColorConv->Destroy();

	// Destroy buffer object
	if (m_Buffers && *m_Buffers) m_Buffers->Destroy();

}

void CCamera::RegisterConnectionEventCallback() {
	SapManager::RegisterServerCallback(CORMAN_VAL_EVENT_TYPE_SERVER_DISCONNECTED | CORMAN_VAL_EVENT_TYPE_SERVER_CONNECTED, SapManCallbackMy, this);
}


BOOL CCamera::FindCamera(std::map<std::string, std::map<int32_t, std::string>>  *cameras){
	
	
	int serverCount = SapManager::GetServerCount();
	if (serverCount <= 1) {
		return FALSE;
	}

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
			tmp[cameraIndex] = std::string(serverName);
			cameras->insert(std::make_pair(std::string(cameraName), tmp) );
		}
	}

	return TRUE;
}

BOOL CCamera::SetExposureTime(double microseconds) {
	return m_AcqDevice->SetFeatureValue("ExposureTime", microseconds);
}

BOOL CCamera::SetGainBySensorAll(double val) {
	m_AcqDevice->SetFeatureValue("GainSelector", "SensorAll");//FIXME:
	return m_AcqDevice->SetFeatureValue("Gain", val);
}

BOOL CCamera::SetGainBySensorAnalog(double val) {

	m_AcqDevice->SetFeatureValue("GainSelector", "SensorAnalog");//FIXME:
	return m_AcqDevice->SetFeatureValue("Gain", val);
}

BOOL CCamera::SetGainBySensorDigital(double val) {
	m_AcqDevice->SetFeatureValue("GainSelector", "SensorDigital");//FIXME:
	return m_AcqDevice->SetFeatureValue("Gain", val);
}

BOOL CCamera::SetFrameRate(double val) {

	return m_AcqDevice->SetFeatureValue("AcquisitionFrameRate", val);
}

BOOL CCamera::EnableTurboTransfer(BOOL bEnable) {
	return m_AcqDevice->SetFeatureValue("turboTransferEnable", bEnable);
}

BOOL CCamera::SetImageWidth(INT64 width) {

	return m_AcqDevice->SetFeatureValue("Width", width) ;
}


BOOL CCamera::SetImageHeight(INT64 height) {

	return m_AcqDevice->SetFeatureValue("Height", height);
}

BOOL CCamera::SetOffsetX(INT64 offset_x) {

	return m_AcqDevice->SetFeatureValue("OffsetX", offset_x);
}
BOOL CCamera::SetOffsetY(INT64 offset_y) {

	return m_AcqDevice->SetFeatureValue("OffsetY", offset_y);
}


BOOL CCamera::SetPixelFormat(const char* val, int len) { //BayerRG8 BayerRG10
												   // determine the bayer alignement with the PixelFormat feature

	BOOL bPixelFormatAvailable = FALSE;
	if (0 != len && NULL != val) {
		return m_AcqDevice->IsFeatureAvailable("PixelFormat", &bPixelFormatAvailable) && bPixelFormatAvailable && m_AcqDevice->SetFeatureValue("PixelFormat", val);
	}
	return FALSE;
}


double CCamera::GetExposureTime() {
	double val = .0f;
	if (m_AcqDevice->GetFeatureValue("ExposureTime", &val) == FALSE) {
		fprintf(stdout, "%s Failed!\n", __FUNCTION__);
	}
	return val;
}

double CCamera::GetGainBySensorAll() {
	double val = .0f;
	m_AcqDevice->SetFeatureValue("GainSelector", "SensorAll");//FIXME:
	if (m_AcqDevice->GetFeatureValue("Gain", &val) == FALSE) {
		fprintf(stdout, "%s Failed!\n", __FUNCTION__);
	}
	return val;
}


double CCamera::GetGainBySensorAnalog() {
	double val = .0f;
	m_AcqDevice->SetFeatureValue("GainSelector", "SensorAnalog");//FIXME:
	if (m_AcqDevice->GetFeatureValue("Gain", &val) == FALSE) {
		fprintf(stdout, "%s Failed!\n", __FUNCTION__);
	}
	return val;
}

double CCamera::GetGainBySensorDigital() {
	double val = .0f;
	m_AcqDevice->SetFeatureValue("GainSelector", "SensorDigital");//FIXME:
	if (m_AcqDevice->GetFeatureValue("Gain", &val) == FALSE) {
		fprintf(stdout, "%s Failed!\n", __FUNCTION__);
	}
	return val;
}

double CCamera::GetFrameRate() {
	double val = .0f;
	if (m_AcqDevice->GetFeatureValue("AcquisitionFrameRate", &val) == FALSE) {
		fprintf(stdout, "%s Failed!\n", __FUNCTION__);
	}
	return val;
}

INT64 CCamera::GetHeightMax() {
	INT64 val = 0;
	if (m_AcqDevice->GetFeatureValue("HeightMax", &val) == FALSE) {
		fprintf(stdout, "%s Failed!\n", __FUNCTION__);
	}
	return val;
}

INT64 CCamera::GetWidthMax() {
	INT64 val = 0;
	if (m_AcqDevice->GetFeatureValue("WidthMax", &val) == FALSE) {
		fprintf(stdout, "%s Failed!\n", __FUNCTION__);
	}
	return val;
}

INT64	CCamera::GetImageWidth() {
	INT64 val = 0;
	if (m_AcqDevice->GetFeatureValue("Width", &val) == FALSE) {
		fprintf(stdout, "%s Failed!\n", __FUNCTION__);
	}
	return val;
}

INT64	CCamera::GetImageHeight() {
	INT64 val = 0;
	if (m_AcqDevice->GetFeatureValue("Height", &val) == FALSE) {
		fprintf(stdout, "%s Failed!\n", __FUNCTION__);
	}
	return val;
}

INT64 CCamera::GetOffsetX() {
	INT64 val = 0;
	if (m_AcqDevice->GetFeatureValue("OffsetX", &val) == FALSE) {
		fprintf(stdout, "%s Failed!\n", __FUNCTION__);
	}
	return val;
}
INT64 CCamera::GetOffsetY() {
	INT64 val = 0;
	if (m_AcqDevice->GetFeatureValue("OffsetY", &val) == FALSE) {
		fprintf(stdout, "%s Failed!\n", __FUNCTION__);
	}
	return val;
}

BOOL CCamera::IsEnabledTurboTransfer() {
	BOOL val = 0;
	if (m_AcqDevice->GetFeatureValue("turboTransferEnable", &val) == FALSE) {
		fprintf(stdout, "%s Failed!\n", __FUNCTION__);
	}
	return val;
}

BOOL CCamera::GetUserDefinedName(char* buf, int len) {
	fprintf(stdout, "%s \n", __FUNCTION__);
	return  m_AcqDevice->GetFeatureValue("DeviceUserID", buf, len);
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
	return  m_AcqDevice->GetFeatureValue("DeviceSerialNumber", val, len);
}


BOOL CCamera::SaveFeatures(const char* configFile) {
	return  m_AcqDevice->SaveFeatures(configFile);
}

BOOL  CCamera::LoadFeatures(const char *configFile) {
	return  m_AcqDevice->LoadFeatures(configFile);
}


double CCamera::GetGrabFPS() {
	return m_GrabFPSCounter.GetFPS();
}
double CCamera::GetProcessFPS() {
	return m_ProcessFPSCounter.GetFPS();
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
	if (m_AcqDevice->IsFeatureAvailable("PixelFormat", &bPixelFormatAvailable))	{
		if (bPixelFormatAvailable && m_AcqDevice->GetFeatureValue("PixelFormat", sPixelFormatValue, _countof(sPixelFormatValue)))	{
			snprintf(buf, len, "%s", sPixelFormatValue);
			return TRUE;
		}
	}
	return FALSE;
}



BOOL CCamera::GetFeatureRange(const char* featureName, double* min, double* max) {
	BOOL bAvailable = FALSE;
	BOOL bStatus = FALSE;
	if (featureName && m_AcqDevice->IsFeatureAvailable(featureName, &bAvailable) && bAvailable) {
		SapFeature feature(this->m_AcqServerName);
		bStatus = feature.Create();
		if (bStatus) {
			bStatus = m_AcqDevice->GetFeatureInfo(featureName, &feature);
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
	m_AcqDevice->SetFeatureValue("GainSelector", "SensorAnalog");//FIXME:
	return GetFeatureRange(pFeatureName, min, max);
}

BOOL CCamera::GetGainBySensorDigitalRange(double* min, double* max) {
	const char* pFeatureName = "Gain";
	m_AcqDevice->SetFeatureValue("GainSelector", "SensorDigital");//FIXME:
	return GetFeatureRange(pFeatureName, min, max);
}

BOOL CCamera::GetFrameRateRange(double* min, double* max) {
	const char* pFeatureName = "AcquisitionFrameRate";
	return GetFeatureRange(pFeatureName, min, max);
}

BOOL CCamera::GetGainBySensorAllRange(double* min, double* max) {
	const char* pFeatureName = "Gain";
	m_AcqDevice->SetFeatureValue("GainSelector", "SensorAll");//FIXME:
	return GetFeatureRange(pFeatureName, min, max);
}

BOOL CCamera::EnableColorConvert(BOOL bEnable) {
	return m_bEnableColorConvert = bEnable;
}

BOOL CCamera::GetCurrentIPAddress(char* val, int len) {
	BOOL bAvailable = FALSE;
	if (0 != len && NULL != val) {
		if (m_AcqDevice->IsFeatureAvailable("PixelFormat", &bAvailable) && bAvailable) {
			INT64 ip = 0;
			in_addr addr;
			m_AcqDevice->GetFeatureValue("GevCurrentIPAddress", &ip);
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
		if (m_AcqDevice->IsFeatureAvailable("DeviceModelName", &bAvailable) && bAvailable) {
			m_AcqDevice->GetFeatureValue("DeviceModelName", val, len);
			return TRUE;
		}
	}
	return FALSE;
}

void XferCallback(SapXferCallbackInfo *pInfo) {
	CCamera *camera = (CCamera *)pInfo->GetContext();
	// If grabbing in trash buffer, do not display the image, update the
	// appropriate number of frames on the status bar instead
	if (pInfo->IsTrash()) {
		//fprintf(stdout, "%s 1 Failed!\n", __FUNCTION__);
		fprintf(stdout, "Frames acquired in trash buffer: %03d\n", pInfo->GetEventCount());
	}
	else {
		PUINT8 pData;
		SapBuffer* pBuffer = NULL;
		pBuffer = camera->m_Buffers;

		BOOL status = pBuffer->GetAddress(pBuffer->GetIndex(), (void**)&pData);
		if (status) {
			//fprintf(stdout, "%s %d %x\r\n", __FUNCTION__, status, pData);
			if (camera->m_sink_bayer_cb) {
				int64_t ret = camera->m_sink_bayer_cb(pData, pBuffer->GetWidth() * pBuffer->GetHeight(), camera->m_ctx0);//FIXME: bufferlen 
				//fprintf(stdout, "%s send %lld bytes!\r\n", __FUNCTION__,  ret);
			}
			if (camera->m_dummy_bayer_fp) {
				fwrite(pData, 1, pBuffer->GetWidth() * pBuffer->GetHeight(), (FILE*)camera->m_dummy_bayer_fp);
				fclose((FILE*)(camera->m_dummy_bayer_fp));
				camera->m_dummy_bayer_fp = NULL;
			}
		}
		else {
			fprintf(stdout, "%s %d %p\r\n", __FUNCTION__, status, pData);
		}

		pBuffer->ReleaseAddress(pData);

		camera->m_GrabFPSCounter.statistics(__FUNCTION__,FALSE);
		// Process current buffer (see Run member function into the SapMyProcessing.cpp file)
		camera->m_Pro->Execute();
	}
}

//
// This function is called each time a buffer has been processed by the processing object
//
void ProCallback(SapProCallbackInfo *pInfo) {
	CCamera *camera = (CCamera *)pInfo->GetContext();
	// Check if color conversion was enabled and if it's been done by software
	if (camera->m_ColorConv->IsSoftwareEnabled())
	{
		// Show current buffer index and execution time in millisecond
		//fprintf(stdout, "%s Color conversion = %5.2f ms\n", __FUNCTION__, camera->m_Pro->GetTime());

		SapBuffer* pBuffer = NULL;
		pBuffer = camera->m_ColorConv->GetOutputBuffer();
		PUINT8 pData = NULL;
		BOOL status = pBuffer->GetAddress(pBuffer->GetIndex(), (void**)&pData);

		if (status) {
			//fprintf(stdout, "%s %d %x\r\n", __FUNCTION__, status, pData);
			if (camera->m_sink_rgb_cb) {
				camera->m_sink_rgb_cb(pData, pBuffer->GetPitch() * pBuffer->GetHeight(), camera->m_ctx1);//FIXME: bufferlen 
			}

			if (camera->m_dummy_rgb_fp) {
				fwrite(pData, 1, pBuffer->GetPitch()  * pBuffer->GetHeight(), (FILE*)camera->m_dummy_rgb_fp);
				fclose((FILE*)(camera->m_dummy_rgb_fp));
				camera->m_dummy_rgb_fp = NULL;
			}
			camera->m_ProcessFPSCounter.statistics(__FUNCTION__, FALSE);
		}

		pBuffer->ReleaseAddress(pData);
	}
}

void SapManCallbackMy(SapManCallbackInfo *pInfo) {
	int type = pInfo->GetEventType();
	CCamera* camera = (CCamera*)pInfo->GetContext();

	fprintf(stdout, "connect status :%d\n", type);

	char serverName[30];
	SapManager::GetServerName(pInfo->GetServerIndex(), serverName, sizeof(serverName));

	if (pInfo->GetResourceIndex() != camera->m_index || strcmp(serverName, camera->m_AcqServerName) != 0) {
		return;
	}

	if (type == SapManager::EventServerDisconnected) {
		camera->Stop();
	}
	else if (type == SapManager::EventServerConnected) {
		if (camera->m_last_is_connected) {
			fprintf(stdout, "%s m_last_is_connected %d\n", __FUNCTION__, camera->m_last_is_connected);
		}
		if (camera->m_last_is_grabbing) {
			fprintf(stdout, "%s m_last_is_grabbing %d\n", __FUNCTION__, camera->m_last_is_grabbing);
			camera->Start();
		}
	}
}

