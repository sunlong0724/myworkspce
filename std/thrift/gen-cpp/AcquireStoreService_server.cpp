// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.




#include "AcquireStoreService.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "Camera.h"
#include "ZmqTransportDataImpl.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ::hawkeye;


class AcquireStoreServiceHandler : virtual public AcquireStoreServiceIf {
 public:
  AcquireStoreServiceHandler() {
    // Your initialization goes here
  }

  CCamera	*m_camera;

  int32_t start(const int32_t snd_frame_rate) {
	  // Your implementation goes here
	  printf("start\n");
	  CPostProcessor* pProcessor = (CPostProcessor*)m_camera->get_SinkBayerDataCallback_ctx();
	  pProcessor->set_send_frame_rate(snd_frame_rate, m_camera->GetFrameRate());
	  if (m_camera->CreateOtherObjects()) {
		  m_camera->Start();
		  return TRUE;
	  }
	  return FALSE;
  }

  int32_t stop() {
	  // Your implementation goes here
	  printf("stop\n");
	  m_camera->Stop();
	  m_camera->DestroyOtherObjects();
	  return TRUE;
  }

  int32_t set_snd_frame_rate(const int32_t snd_frame_rate, const int32_t full_frame_rate) {
	  // Your implementation goes here
	  printf("set_snd_frame_rate\n");
	  CPostProcessor* pProcessor = (CPostProcessor*)m_camera->get_SinkBayerDataCallback_ctx();
	  return pProcessor->set_send_frame_rate(snd_frame_rate, m_camera->GetFrameRate());
  }

  int32_t set_store_file(const int32_t flag, const std::string& file_name) {
	  // Your implementation goes here
	  printf("set_store_file\n");
	  if (0 == CStoreFile::m_file_name.size()) {
		  char ip[30] = { 0 };
		  if (m_camera->GetCurrentIPAddress(ip, sizeof ip)) {
			  std::string name(ip);
			  CStoreFile::m_file_name = name + ".raw";
		  }
	  }
	  CPostProcessor* pProcessor = (CPostProcessor*)m_camera->get_SinkBayerDataCallback_ctx();
	  return pProcessor->m_store_file_flag = flag;
  }

  int32_t do_pause(const int32_t flag) {
	  // Your implementation goes here
	  printf("do_pause\n");
	  if (flag == TRUE) {
		  m_camera->Stop();
	  }
	  else if (flag == FALSE) {
		  m_camera->Start();
	  }
	  return TRUE;
  }

  int32_t forward_play(const int64_t frame_seq, const int32_t snd_frame_rate) {
	  // Your implementation goes here
	  printf("forward_play\n");
	  return 0;
  }

  int32_t backward_play(const int64_t frame_seq, const int32_t snd_frame_rate) {
	  // Your implementation goes here
	  printf("backward_play\n");
	  return 0;
  }

  int32_t forward_play_temporary(const int64_t frame_seq, const int32_t snd_frame_rate) {
	  // Your implementation goes here
	  printf("forward_play_temporary\n");
	  return 0;
  }

  int32_t backward_play_temporary(const int64_t frame_seq, const int32_t snd_frame_rate) {
	  // Your implementation goes here
	  printf("backward_play_temporary\n");
	  return 0;
  }

  int32_t set_exposure_time(const double microseconds) {
	  // Your implementation goes here
	  printf("set_exposure_time\n");
	  return m_camera->SetExposureTime(microseconds);
  }

  int32_t set_gain_by_sensor_all(const double gain) {
	  // Your implementation goes here
	  printf("set_gain_by_sensor_all\n");
	  return m_camera->SetGainBySensorAll(gain);
  }

  int32_t set_gain_by_sensor_analog(const double gain) {
	  // Your implementation goes here
	  printf("set_gain_by_sensor_analog\n");
	  return m_camera->SetGainBySensorAnalog(gain);
  }

  int32_t set_gain_by_sensor_digital(const double gain) {
	  // Your implementation goes here
	  printf("set_gain_by_sensor_digital\n");
	  return m_camera->SetGainBySensorDigital(gain);
  }

  int32_t set_frame_rate(const double rate) {
	  // Your implementation goes here
	  printf("set_frame_rate\n");
	  return m_camera->SetFrameRate(rate);
  }

  int32_t enable_turbo_transfer(const int32_t enabled) {
	  // Your implementation goes here
	  printf("enable_turbo_transfer\n");
	  return m_camera->EnableTurboTransfer(enabled);
  }

  int32_t set_pixel_fmt(const std::string& pixel_fmt) {
	  // Your implementation goes here
	  printf("set_pixel_fmt\n");
	  return m_camera->SetPixelFormat(pixel_fmt.c_str(), pixel_fmt.length());
  }

  int32_t set_offset_x(const int32_t offset_x) {
	  // Your implementation goes here
	  printf("set_offset_x\n");
	  return m_camera->SetOffsetX(offset_x);
  }

  int32_t set_offset_y(const int32_t offset_y) {
	  // Your implementation goes here
	  printf("set_offset_y\n");
	  return m_camera->SetOffsetY(offset_y);
  }

  int32_t set_image_width(const int32_t width) {
	  // Your implementation goes here
	  printf("set_image_width\n");
	  return m_camera->SetImageWidth(width);
  }

  int32_t set_image_height(const int32_t height) {
	  // Your implementation goes here
	  printf("set_image_height\n");
	  return m_camera->SetImageHeight(height);
  }

  double get_exposure_time() {
	  // Your implementation goes here
	  printf("get_exposure_time\n");
	  return m_camera->GetExposureTime();
  }

  double get_gain_by_sensor_all() {
	  // Your implementation goes here
	  printf("get_gain_by_sensor_all\n");
	  return m_camera->GetGainBySensorAll();
  }

  double get_gain_by_sensor_analog() {
	  // Your implementation goes here
	  printf("get_gain_by_sensor_analog\n");
	  return m_camera->GetGainBySensorAnalog();
  }

  double get_gain_by_sensor_digital() {
	  // Your implementation goes here
	  printf("get_gain_by_sensor_digital\n");
	  return m_camera->GetGainBySensorDigital();
  }

  double get_frame_rate() {
	  // Your implementation goes here
	  printf("get_frame_rate\n");
	  return m_camera->GetFrameRate();
  }

  void get_exposure_time_range(MinMaxStruct& _return) {
	  // Your implementation goes here
	  printf("get_exposure_time_range\n");
	  m_camera->GetExposureTimeRange(&_return.min, &_return.max);//FIXME: should check the value of return!
  }

  void get_gain_range_by_sensor_all(MinMaxStruct& _return) {
	  // Your implementation goes here
	  printf("get_gain_range_by_sensor_all\n");
	  m_camera->GetGainBySensorAllRange(&_return.min, &_return.max);//FIXME: should check the value of return!
  }

  void get_gain_range_by_sensor_analog(MinMaxStruct& _return) {
	  // Your implementation goes here
	  printf("get_gain_range_by_sensor_analog\n");
	  m_camera->GetGainBySensorAnalogRange(&_return.min, &_return.max);//FIXME: should check the value of return!
  }

  void get_gain_range_by_sensor_digital(MinMaxStruct& _return) {
	  // Your implementation goes here
	  printf("get_gain_range_by_sensor_digital\n");
	  m_camera->GetGainBySensorDigitalRange(&_return.min, &_return.max);//FIXME: should check the value of return!
  }

  void get_frame_rate_range(MinMaxStruct& _return) {
	  // Your implementation goes here
	  printf("get_frame_rate_range\n");
	  m_camera->GetFrameRateRange(&_return.min, &_return.max);//FIXME: should check the value of return!
  }

  int32_t get_image_width() {
	  // Your implementation goes here
	  printf("get_image_width\n");
	  return m_camera->GetImageWidth();
  }

  int32_t get_image_height() {
	  // Your implementation goes here
	  printf("get_image_height\n");
	  return m_camera->GetImageHeight();
  }


  int32_t get_height_max() {
	  // Your implementation goes here
	  printf("get_height_max\n");
	  return m_camera->GetHeightMax();
  }

  int32_t get_width_max() {
	  // Your implementation goes here
	  printf("get_width_max\n");
	  return m_camera->GetWidthMax();
  }

  int32_t get_offset_x() {
	  // Your implementation goes here
	  printf("get_offset_x\n");
	  return m_camera->GetOffsetX();
  }

  int32_t get_offset_y() {
	  // Your implementation goes here
	  printf("get_offset_y\n");
	  return m_camera->GetOffsetY();
  }

  int32_t is_enabled_turbo_transfer() {
	  // Your implementation goes here
	  printf("is_enabled_turbo_transfer\n");
	  return m_camera->IsEnabledTurboTransfer();
  }

  void get_pixel_fmt(std::string& _return) {
	  // Your implementation goes here
	  printf("get_pixel_fmt\n");
	  char val[256] = { 0 };
	  m_camera->GetPixelFormat(val, sizeof val);
	  _return.assign(val);
  }

  void get_user_defined_name(std::string& _return) {
	  // Your implementation goes here
	  printf("get_user_defined_name\n");
	  char val[256] = { 0 };
	  m_camera->GetUserDefinedName(val, sizeof val);
	  _return.assign(val);
  }

  void get_device_serial_number(std::string& _return) {
	  // Your implementation goes here
	  printf("get_device_serial_number\n");
	  char val[256] = { 0 };
	  m_camera->GetDeviceSerialNumber(val, sizeof val);
	  _return.assign(val);
  }

  void get_current_ip_address(std::string& _return) {
	  // Your implementation goes here
	  printf("get_current_ip_address\n");

	  char val[256] = { 0 };
	  m_camera->GetCurrentIPAddress(val, sizeof val);
	  _return.assign(val);
  }

  double get_grab_fps() {
	  // Your implementation goes here
	  printf("get_grab_fps\n");
	  return m_camera->GetGrabFPS();
  }

  double get_process_fps() {
	  // Your implementation goes here
	  printf("get_process_fps\n");
	  return m_camera->GetProcessFPS();
  }

  void save_feature(std::string& _return) {
	  // Your implementation goes here
	  printf("save_feature\n");
	  m_camera->SaveFeatures(m_camera->GetUserDefinedName());
	  FILE* fp = fopen(m_camera->GetUserDefinedName(), "r");
	  if (fp) {
		  char buf[1024 * 10] = { 0 };
		  int r = fread(buf, 1, sizeof buf, fp);
		  _return.append(buf, r);
		  fclose(fp);
	  }
  }

  int32_t update_feature(const std::string& content) {
	  // Your implementation goes here
	  printf("update_feature\n");
	  FILE* fp = fopen(m_camera->GetUserDefinedName(), "w");
	  if (fp) {
		  fwrite(content.data(), 1, content.size(), fp);
		  fclose(fp);
	  }
	  return m_camera->LoadFeatures(m_camera->GetUserDefinedName());
  }


};


