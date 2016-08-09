// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "PlaybackCtrlService.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "Camera.h"
#include "defs.h"
#include "PlaybackCtrl.h"
#include "PostProcessor.h"
#include "TransportData.h"
#include "FileStorage.h"

extern int SinkBayerDatasCallbackImpl(unsigned char* buffer, int buffer_len, int lost, void* context);
extern inline int processor_sink_data_cb(unsigned char* data, int data_len, void* ctx);

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ::hawkeye;

extern CustomStruct g_cs;

inline void calc_frame_rate_some(const int32_t play_frame_rate, const int32_t sample) {
	g_cs.m_play_frame_rate = play_frame_rate;
	g_cs.m_frame_rate = g_cs.m_camera->GetFrameRate();
	g_cs.m_play_frame_gap = sample;
	//g_cs.m_last_play_seq = 0;
}

class PlaybackCtrlServiceHandler : virtual public PlaybackCtrlServiceIf {
 public:
  PlaybackCtrlServiceHandler() {
    // Your initialization goes here
  }

  int32_t get_data_port() {
	  // Your implementation goes here
	  //printf("get_data_port/PingPong\n");
	  return g_cs.m_data_port;
  }

  int32_t set_play_frame_rate(const int32_t play_frame_rate, const int32_t sample_gap) {
	  // Your implementation goes here
	  printf("set_play_frame_rate\n");
     calc_frame_rate_some(play_frame_rate,sample_gap);
	  return 1;
  }


  int32_t set_play_frame_resolution(const int32_t w, const int32_t h) {
    // Your implementation goes here
    printf("set_play_frame_resolution(%d %d)\n", w, h);
	g_cs.m_play_frame_w = w;
	g_cs.m_play_frame_h = h;

	g_cs.m_processor_data_flag = (g_cs.m_image_w != g_cs.m_play_frame_w || g_cs.m_image_h != g_cs.m_play_frame_h);
	if (g_cs.m_processor_data_flag) {
		g_cs.m_post_processor_thread->set_parameters(g_cs.m_image_w, g_cs.m_image_h, g_cs.m_play_frame_w, g_cs.m_play_frame_h);
		g_cs.m_post_processor_thread->set_sink_callback(processor_sink_data_cb, &g_cs);
		g_cs.m_post_processor_thread->start();
	}
	else {
		g_cs.m_post_processor_thread->stop();
	}

	g_cs.m_snd_data_thread->set_parameters(GET_IMAGE_BUFFER_SIZE(w, h));

	return 1;
  }

  int32_t set_store_file(const int32_t flag) {
	  // Your implementation goes here
	  printf("set_store_file\n");
	  return g_cs.m_store_file_flag = flag;
  }

  int32_t start_grab() {
	  // Your implementation goes here
	  printf("start_grab\n");
	  g_cs.m_camera->SetSinkBayerDataCallback(SinkBayerDatasCallbackImpl, &g_cs);
	  if (!g_cs.m_camera->CreateOtherObjects()) {
		  fprintf(stdout, "%s CreateOtherObjects failed!\n", __FUNCTION__);
	  }
	  g_cs.m_camera->Start();
	  return 1;
  }

  int32_t stop_grab() {
	  // Your implementation goes here
	  printf("1stop_grab\n");
	  g_cs.m_playback_thread->m_status = Pb_STATUS_NONE;
	  printf("2stop_grab \n");
	  g_cs.m_camera->Stop();
	  printf("3stop_grab \n");
	  g_cs.m_camera->DestroyOtherObjects();
	  printf("4stop_grab \n");
	  return 1;
  }

  int32_t play_pause() {
	  // Your implementation goes here
	  printf("play_pause %d %d\n", g_cs.m_playback_thread->m_status, g_cs.m_playback_thread->m_last_status);
	  if (g_cs.m_playback_thread->m_last_status == Pb_STATUS_NONE) {
		  g_cs.m_playback_thread->m_last_status = g_cs.m_playback_thread->m_status;
		  return g_cs.m_playback_thread->m_status = Pb_STATUS_PLAY_PAUSE;
	  }
	  else {
		   g_cs.m_playback_thread->m_status = g_cs.m_playback_thread->m_last_status;
		   g_cs.m_playback_thread->m_last_status = Pb_STATUS_NONE;
		   return g_cs.m_playback_thread->m_status;
	  }
  }

  int32_t play_live(const int32_t play_frame_rate, const int32_t sample_gap) {
	  // Your implementation goes here
	  printf("play_live\n");
	  calc_frame_rate_some(play_frame_rate, sample_gap);
	  g_cs.m_snd_live_frame_flag = TRUE;
	  g_cs.m_last_live_play_seq = g_cs.m_frame_counter;

	  g_cs.m_playback_thread->m_start_play_frame_no_begin = 0;

	  g_cs.m_playback_thread->m_last_status = Pb_STATUS_NONE;
	  return g_cs.m_playback_thread->m_status = Pb_STATUS_PLAY_CAMERAS;
  }

  int32_t play_forward(const int32_t play_frame_rate, const int32_t sample_gap) {
	  // Your implementation goes here
	  printf("play_forward\n");
	  calc_frame_rate_some(play_frame_rate, sample_gap);
	  g_cs.m_snd_live_frame_flag = FALSE;
	  g_cs.m_playback_thread->m_start_play_frame_no = g_cs.m_snd_data_thread->m_last_snd_seq - 1;//update m_start_play_frame_no as m_last_snd_seq-1
	  g_cs.m_playback_thread->m_last_status = Pb_STATUS_NONE;
	  return g_cs.m_playback_thread->m_status = Pb_STATUS_PLAY_FORWARD;
  }

  int32_t play_backward(const int32_t play_frame_rate, const int32_t sample_gap) {
	  // Your implementation goes here
	  printf("play_backward\n");
	  calc_frame_rate_some(play_frame_rate, sample_gap);
	  g_cs.m_snd_live_frame_flag = FALSE;
	  if (g_cs.m_playback_thread->m_start_play_frame_no_begin == 0) {
		  g_cs.m_playback_thread->m_start_play_frame_no_begin = g_cs.m_playback_thread->m_start_play_frame_no = g_cs.m_file_storage_object_for_read->m_frame_offset_map.rbegin()->first - 1;/*g_cs.m_frame_counter - 1;*/
	  }
	  else {
		  g_cs.m_playback_thread->m_start_play_frame_no = g_cs.m_snd_data_thread->m_last_snd_seq - 1;//update m_start_play_frame_no as m_last_snd_seq-1
	  }
	 
	  printf("start_backward_play_temp m_start_play_frame_no %d, m_start_play_frame_no_begin %d\n", g_cs.m_playback_thread->m_start_play_frame_no, g_cs.m_playback_thread->m_start_play_frame_no_begin);
	  fprintf(stdout, "%s map size(%lld),beg(seq:%lld,offset:%lld),end(seq:%lld,offset:%lld)\n", __FUNCTION__, g_cs.m_file_storage_object_for_write_thread->m_frame_offset_map.size(), \
		  g_cs.m_file_storage_object_for_write_thread->m_frame_offset_map.begin()->first, g_cs.m_file_storage_object_for_write_thread->m_frame_offset_map.begin()->second, \
		  g_cs.m_file_storage_object_for_write_thread->m_frame_offset_map.rbegin()->first, g_cs.m_file_storage_object_for_write_thread->m_frame_offset_map.rbegin()->second);
	  g_cs.m_playback_thread->m_last_status = Pb_STATUS_NONE;
	  return g_cs.m_playback_thread->m_status = Pb_STATUS_PLAY_BACKWARD;
  }

  int32_t play_from_a2b(const int64_t from, const int64_t to) {
	  // Your implementation goes here
	  printf("play_from_a2b %d %d\n", from ,to);
	  g_cs.m_playback_thread->m_from_a2b_to = to;
	  g_cs.m_playback_thread->m_from_a2b_from = g_cs.m_playback_thread->m_from_a2b_index = from;

	  g_cs.m_playback_thread->m_toward_2b = TRUE;
	  g_cs.m_playback_thread->m_last_status = Pb_STATUS_NONE;
	  return g_cs.m_playback_thread->m_status = Pb_STATUS_PLAY_FROM_A2B_LOOP;
  }


  double get_camera_grab_fps() {
	  // Your implementation goes here
	  return g_cs.m_camera->GetGrabFPS();
  }

  double get_soft_grab_fps() {
	  // Your implementation goes here
	  //printf("get_soft_grab_fps\n");
	  return g_cs.m_soft_grab_counter.GetFPS();
  }

  double get_soft_snd_fps() {
	  // Your implementation goes here
	  //printf("get_soft_snd_fps\n");
	  return g_cs.m_snd_data_thread->m_soft_snd_counter.GetFPS();
  }
  
  double get_file_write_fps() {
	  // Your implementation goes here
	  printf("get_file_write_fps\n");
	  return g_cs.m_file_storage_object_for_write_thread->m_fps_counter.GetFPS();
  }

  int32_t set_exposure_time(const double microseconds) {
	  // Your implementation goes here
	  printf("set_exposure_time\n");
	  return g_cs.m_camera->SetExposureTime(microseconds);
  }

  int32_t set_gain_by_sensor_all(const double gain) {
	  // Your implementation goes here
	  printf("set_gain_by_sensor_all\n");
	  return g_cs.m_camera->SetGainBySensorAll(gain);
  }

  int32_t set_gain_by_sensor_analog(const double gain) {
	  // Your implementation goes here
	  printf("set_gain_by_sensor_analog\n");
	  return g_cs.m_camera->SetGainBySensorAnalog(gain);
  }

  int32_t set_gain_by_sensor_digital(const double gain) {
	  // Your implementation goes here
	  printf("set_gain_by_sensor_digital\n");
	  return g_cs.m_camera->SetGainBySensorDigital(gain);
  }

  int32_t set_frame_rate(const double rate) {
	  // Your implementation goes here
	  printf("set_frame_rate\n");
	  return g_cs.m_camera->SetFrameRate(rate);
  }

  int32_t enable_turbo_transfer(const int32_t enabled) {
	  // Your implementation goes here
	  printf("enable_turbo_transfer\n");
	  return g_cs.m_camera->EnableTurboTransfer(enabled);
  }

  int32_t set_pixel_fmt(const std::string& pixel_fmt) {
	  // Your implementation goes here
	  printf("set_pixel_fmt\n");
	  return g_cs.m_camera->SetPixelFormat(pixel_fmt.c_str(), pixel_fmt.length());
  }

  int32_t set_offset_x(const int32_t offset_x) {
	  // Your implementation goes here
	  printf("set_offset_x\n");
	  return g_cs.m_camera->SetOffsetX(offset_x);
  }

  int32_t set_offset_y(const int32_t offset_y) {
	  // Your implementation goes here
	  printf("set_offset_y\n");
	  return g_cs.m_camera->SetOffsetY(offset_y);
  }

  int32_t set_image_width(const int32_t width) {
	  // Your implementation goes here
	  printf("set_image_width\n");
	  return g_cs.m_camera->SetImageWidth(width);
  }

  int32_t set_image_height(const int32_t height) {
	  // Your implementation goes here
	  printf("set_image_height\n");
	  return g_cs.m_camera->SetImageHeight(height);
  }

  double get_exposure_time() {
	  // Your implementation goes here
	  printf("get_exposure_time\n");
	  return g_cs.m_camera->GetExposureTime();
  }

  double get_gain_by_sensor_all() {
	  // Your implementation goes here
	  printf("get_gain_by_sensor_all\n");
	  return g_cs.m_camera->GetGainBySensorAll();
  }

  double get_gain_by_sensor_analog() {
	  // Your implementation goes here
	  printf("get_gain_by_sensor_analog\n");
	  return g_cs.m_camera->GetGainBySensorAnalog();
  }

  double get_gain_by_sensor_digital() {
	  // Your implementation goes here
	  printf("get_gain_by_sensor_digital\n");
	  return g_cs.m_camera->GetGainBySensorDigital();
  }

  double get_frame_rate() {
	  // Your implementation goes here
	  printf("get_frame_rate\n");
	  return g_cs.m_camera->GetFrameRate();
  }

  void get_exposure_time_range(MinMaxStruct& _return) {
	  // Your implementation goes here
	  printf("get_exposure_time_range\n");
	  g_cs.m_camera->GetExposureTimeRange(&_return.min, &_return.max);//FIXME: should check the value of return!
  }

  void get_gain_range_by_sensor_all(MinMaxStruct& _return) {
	  // Your implementation goes here
	  printf("get_gain_range_by_sensor_all\n");
	  g_cs.m_camera->GetGainBySensorAllRange(&_return.min, &_return.max);//FIXME: should check the value of return!
  }

  void get_gain_range_by_sensor_analog(MinMaxStruct& _return) {
	  // Your implementation goes here
	  printf("get_gain_range_by_sensor_analog\n");
	  g_cs.m_camera->GetGainBySensorAnalogRange(&_return.min, &_return.max);//FIXME: should check the value of return!
  }

  void get_gain_range_by_sensor_digital(MinMaxStruct& _return) {
	  // Your implementation goes here
	  printf("get_gain_range_by_sensor_digital\n");
	  g_cs.m_camera->GetGainBySensorDigitalRange(&_return.min, &_return.max);//FIXME: should check the value of return!
  }

  void get_frame_rate_range(MinMaxStruct& _return) {
	  // Your implementation goes here
	  printf("get_frame_rate_range\n");
	  g_cs.m_camera->GetFrameRateRange(&_return.min, &_return.max);//FIXME: should check the value of return!
  }

  int32_t get_image_width() {
	  // Your implementation goes here
	  printf("get_image_width\n");
	  return g_cs.m_camera->GetImageWidth();
  }

  int32_t get_image_height() {
	  // Your implementation goes here
	  printf("get_image_height\n");
	  return g_cs.m_camera->GetImageHeight();
  }


  int32_t get_height_max() {
	  // Your implementation goes here
	  printf("get_height_max\n");
	  return g_cs.m_camera->GetHeightMax();
  }

  int32_t get_width_max() {
	  // Your implementation goes here
	  printf("get_width_max\n");
	  return g_cs.m_camera->GetWidthMax();
  }

  int32_t get_offset_x() {
	  // Your implementation goes here
	  printf("get_offset_x\n");
	  return g_cs.m_camera->GetOffsetX();
  }

  int32_t get_offset_y() {
	  // Your implementation goes here
	  printf("get_offset_y\n");
	  return g_cs.m_camera->GetOffsetY();
  }

  int32_t is_enabled_turbo_transfer() {
	  // Your implementation goes here
	  printf("is_enabled_turbo_transfer\n");
	  return g_cs.m_camera->IsEnabledTurboTransfer();
  }

  void get_pixel_fmt(std::string& _return) {
	  // Your implementation goes here
	  printf("get_pixel_fmt\n");
	  char val[256] = { 0 };
	  g_cs.m_camera->GetPixelFormat(val, sizeof val);
	  _return.assign(val);
  }

  void get_user_defined_name(std::string& _return) {
	  // Your implementation goes here
	  printf("get_user_defined_name\n");
	  char val[256] = { 0 };
	  g_cs.m_camera->GetUserDefinedName(val, sizeof val);
	  _return.assign(val);
  }

  void get_device_serial_number(std::string& _return) {
	  // Your implementation goes here
	  printf("get_device_serial_number\n");
	  char val[256] = { 0 };
	  g_cs.m_camera->GetDeviceSerialNumber(val, sizeof val);
	  _return.assign(val);
  }

  void get_current_ip_address(std::string& _return) {
	  // Your implementation goes here
	  printf("get_current_ip_address\n");

	  char val[256] = { 0 };
	  g_cs.m_camera->GetCurrentIPAddress(val, sizeof val);
	  _return.assign(val);
  }

  double get_grab_fps() {
	  // Your implementation goes here
	  printf("get_grab_fps\n");
	  return g_cs.m_camera->GetGrabFPS();
  }

  double get_process_fps() {
	  // Your implementation goes here
	  printf("get_process_fps\n");
	  return g_cs.m_camera->GetProcessFPS();
  }

  void save_feature(std::string& _return) {
	  // Your implementation goes here
	  printf("save_feature\n");
	  g_cs.m_camera->SaveFeatures(g_cs.m_camera->GetUserDefinedName());
	  FILE* fp = fopen(g_cs.m_camera->GetUserDefinedName(), "r");
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
	  FILE* fp = fopen(g_cs.m_camera->GetUserDefinedName(), "w");
	  if (fp) {
		  fwrite(content.data(), 1, content.size(), fp);
		  fclose(fp);
	  }
	  return g_cs.m_camera->LoadFeatures(g_cs.m_camera->GetUserDefinedName());
  }


};



