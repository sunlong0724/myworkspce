#ifndef __GIGECAMERA_H__
#define __GIGECAMERA_H__

#define DLL_API __declspec(dllexport)  

#include "SapClassBasic.h"
#include "SapManager.h"

#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include "FPSCounter.h"

typedef int (*SinkDataCallback)(unsigned char*, int, void*);


class DLL_API CCamera {

private:
	CCamera(const char* _servName);
public:
	CCamera(const char* serverName, int index);
	virtual ~CCamera();
     
	static BOOL FindCamera(std::map<std::string, std::pair<std::string, int>> *cameras);
	static void RegisterServerCallback(std::map<std::string, std::shared_ptr<CCamera>>	 *cameras);

	void		SetSinkBayerDataCallback(SinkDataCallback cb, void* ctx);
	void		SetSinkRGBDataCallback(SinkDataCallback cb, void* ctx);

	void		Start();
	void		Stop();

	BOOL		EnableColorConvert(BOOL bEnable = FALSE);//output fmt:SapFormatRGB888

	BOOL		CreateDevice();
	BOOL		CreateOtherObjects();
	void		DestroyDevice();
	void		DestroyOtherObjects();

	BOOL		SetExposureTime(double microseconds);
	BOOL		SetGainBySensorAll(double value);
	BOOL		SetGainBySensorAnalog(double value);
	BOOL		SetGainBySensorDigital(double value);
	BOOL		SetFrameRate(double value);
	BOOL		EnableTurboTransfer(BOOL bEnable = TRUE);
	BOOL		SetPixelFormat(const char* val, int len);//BayerRG8 BayerRG10
	BOOL		SetOffsetX(INT64 x);
	BOOL		SetOffsetY(INT64 y);
	BOOL		SetImageWidth(INT64 width);
	BOOL		SetImageHeight(INT64 height);
	BOOL		SetWhiteBalance(double val1, double val2, double val3);//FIXME:
	

	double		GetExposureTime();
	double      GetGainBySensorAll();//C1920
	double		GetGainBySensorAnalog();//C1280
	double		GetGainBySensorDigital();//C1280
	double		GetFrameRate();

	BOOL		GetExposureTimeRange(double* min, double* max);
	BOOL		GetGainBySensorAllRange(double* min, double* max);
	BOOL		GetGainBySensorAnalogRange(double* min, double* max);
	BOOL		GetGainBySensorDigitalRange(double* min, double* max);
	BOOL		GetFrameRateRange(double* min, double* max);
	
	INT64		GetHeightMax();
	INT64		GetWidthMax();
	INT64		GetOffsetX();
	INT64		GetOffsetY();
	INT64		GetImageWidth();
	INT64		GetImageHeight();
	BOOL		IsEnabledTurboTransfer();
	BOOL		GetPixelFormat(char* val, int len);
	BOOL		GetWhiteBalance(double* pval1, double* pval2, double* pval3);//FIXME:

	BOOL		GetUserDefinedName(char* val, int len);
	const char*	GetUserDefinedName();
	BOOL		GetDeviceSerialNumber(char* val, int len);
	BOOL		GetCurrentIPAddress(char* val, int len);
	BOOL		GetDeviceModelName(char* val, int len);
	

	//must not start grab
	BOOL		SaveFeatures(const char* configFile);
	BOOL		LoadFeatures(const char *configFile);

	double		GetGrabFPS();
	double		GetProcessFPS();
	void		DumpRawImage();
	void		DumpRgbImage();

	//utility
	void*		get_SinkBayerDataCallback_ctx(){
		return m_ctx0;
	}

	void*		get_SinkRGBDataCallback_ctx() {
		return m_ctx1;
	}

private:
	BOOL				GetFeatureRange(const char* featureName, double* min, double* max);

	enum ConnectionStatus
	{
		UNKNOWN,
		DISCONNECTED,
		CONNECTED,
	};
	ConnectionStatus	m_connection_status;
	BOOL				m_grabbing;
	void				set_reconnect_flag(BOOL flag = TRUE) {
		m_reconnect_flag = flag;
	}

public:
	SapAcqDevice		*m_AcqDevice;
	SapBuffer			*m_Buffers;
	SapColorConversion  *m_ColorConv;
	SapTransfer			*m_Xfer;
	//SapView			*m_View;
	SapProcessing		*m_Pro;

	SinkDataCallback	m_sink_bayer_cb;
	void				*m_ctx0;

	SinkDataCallback	m_sink_rgb_cb;
	void				*m_ctx1;
	
	void				*m_dummy_bayer_fp;
	void				*m_dummy_rgb_fp;
	CFPSCounter			m_GrabFPSCounter;
	CFPSCounter			m_ProcessFPSCounter;

	char				m_UserDefinedName[256];
	char				m_AcqServerName[256];
	BOOL				m_bEnableColorConvert;
	BOOL				m_reconnect_flag;
	BOOL				m_last_is_grabbing;
	BOOL				m_last_is_connected;
};

#endif