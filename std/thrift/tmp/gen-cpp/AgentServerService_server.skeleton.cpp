// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "AgentServerService.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ::hawkeye;

class AgentServerServiceHandler : virtual public AgentServerServiceIf {
 public:
  AgentServerServiceHandler() {
    // Your initialization goes here
  }

  void find_cameras(std::vector<std::string> & _return) {
    // Your implementation goes here
    printf("find_cameras\n");
  }

  int32_t add_cameras(const std::vector<std::string> & l) {
    // Your implementation goes here
    printf("add_cameras\n");
  }

  void get_hold_cameras(std::vector<std::string> & _return) {
    // Your implementation goes here
    printf("get_hold_cameras\n");
  }

  int32_t del_cameras(const std::vector<std::string> & l) {
    // Your implementation goes here
    printf("del_cameras\n");
  }

  int32_t open(const std::string& camera_name) {
    // Your implementation goes here
    printf("open\n");
  }

  int32_t close(const std::string& camera_name) {
    // Your implementation goes here
    printf("close\n");
  }

  int32_t start(const std::string& camera_name, const int32_t display_frame_rate) {
    // Your implementation goes here
    printf("start\n");
  }

  int32_t stop(const std::string& camera_name) {
    // Your implementation goes here
    printf("stop\n");
  }

  int32_t set_exposure_time(const std::string& camera_name, const double microseconds) {
    // Your implementation goes here
    printf("set_exposure_time\n");
  }

  int32_t set_gain_by_sensor_all(const std::string& camera_name, const double gain) {
    // Your implementation goes here
    printf("set_gain_by_sensor_all\n");
  }

  int32_t set_gain_by_sensor_analog(const std::string& camera_name, const double gain) {
    // Your implementation goes here
    printf("set_gain_by_sensor_analog\n");
  }

  int32_t set_gain_by_sensor_digital(const std::string& camera_name, const double gain) {
    // Your implementation goes here
    printf("set_gain_by_sensor_digital\n");
  }

  int32_t set_frame_rate(const std::string& camera_name, const double rate) {
    // Your implementation goes here
    printf("set_frame_rate\n");
  }

  int32_t enable_turbo_transfer(const std::string& camera_name, const int32_t enabled) {
    // Your implementation goes here
    printf("enable_turbo_transfer\n");
  }

  int32_t set_pixel_fmt(const std::string& camera_name, const std::string& pixel_fmt) {
    // Your implementation goes here
    printf("set_pixel_fmt\n");
  }

  int32_t set_offset_x(const std::string& camera_name, const int32_t offset_x) {
    // Your implementation goes here
    printf("set_offset_x\n");
  }

  int32_t set_offset_y(const std::string& camera_name, const int32_t offset_y) {
    // Your implementation goes here
    printf("set_offset_y\n");
  }

  int32_t set_image_width(const std::string& camera_name, const int32_t width) {
    // Your implementation goes here
    printf("set_image_width\n");
  }

  int32_t set_image_height(const std::string& camera_name, const int32_t height) {
    // Your implementation goes here
    printf("set_image_height\n");
  }

  double get_exposure_time(const std::string& camera_name) {
    // Your implementation goes here
    printf("get_exposure_time\n");
  }

  double get_gain_by_sensor_all(const std::string& camera_name) {
    // Your implementation goes here
    printf("get_gain_by_sensor_all\n");
  }

  double get_gain_by_sensor_analog(const std::string& camera_name) {
    // Your implementation goes here
    printf("get_gain_by_sensor_analog\n");
  }

  double get_gain_by_sensor_digital(const std::string& camera_name) {
    // Your implementation goes here
    printf("get_gain_by_sensor_digital\n");
  }

  double get_frame_rate(const std::string& camera_name) {
    // Your implementation goes here
    printf("get_frame_rate\n");
  }

  void get_exposure_time_range(MinMaxStruct& _return, const std::string& camera_name) {
    // Your implementation goes here
    printf("get_exposure_time_range\n");
  }

  void get_gain_range_by_sensor_all(MinMaxStruct& _return, const std::string& camera_name) {
    // Your implementation goes here
    printf("get_gain_range_by_sensor_all\n");
  }

  void get_gain_range_by_sensor_analog(MinMaxStruct& _return, const std::string& camera_name) {
    // Your implementation goes here
    printf("get_gain_range_by_sensor_analog\n");
  }

  void get_gain_range_by_sensor_digital(MinMaxStruct& _return, const std::string& camera_name) {
    // Your implementation goes here
    printf("get_gain_range_by_sensor_digital\n");
  }

  void get_frame_rate_range(MinMaxStruct& _return, const std::string& camera_name) {
    // Your implementation goes here
    printf("get_frame_rate_range\n");
  }

  int32_t get_image_width(const std::string& camera_name) {
    // Your implementation goes here
    printf("get_image_width\n");
  }

  int32_t get_image_height(const std::string& camera_name) {
    // Your implementation goes here
    printf("get_image_height\n");
  }

  int32_t get_height_max(const std::string& camera_name) {
    // Your implementation goes here
    printf("get_height_max\n");
  }

  int32_t get_width_max(const std::string& camera_name) {
    // Your implementation goes here
    printf("get_width_max\n");
  }

  int32_t get_offset_x(const std::string& camera_name) {
    // Your implementation goes here
    printf("get_offset_x\n");
  }

  int32_t get_offset_y(const std::string& camera_name) {
    // Your implementation goes here
    printf("get_offset_y\n");
  }

  int32_t is_enabled_turbo_transfer(const std::string& camera_name) {
    // Your implementation goes here
    printf("is_enabled_turbo_transfer\n");
  }

  void get_pixel_fmt(std::string& _return, const std::string& camera_name) {
    // Your implementation goes here
    printf("get_pixel_fmt\n");
  }

  void get_user_defined_name(std::string& _return, const std::string& camera_name) {
    // Your implementation goes here
    printf("get_user_defined_name\n");
  }

  void get_device_serial_number(std::string& _return, const std::string& camera_name) {
    // Your implementation goes here
    printf("get_device_serial_number\n");
  }

  void get_current_ip_address(std::string& _return, const std::string& camera_name) {
    // Your implementation goes here
    printf("get_current_ip_address\n");
  }

  double get_grab_fps(const std::string& camera_name) {
    // Your implementation goes here
    printf("get_grab_fps\n");
  }

  double get_process_fps(const std::string& camera_name) {
    // Your implementation goes here
    printf("get_process_fps\n");
  }

  void save_feature(std::string& _return, const std::string& camera_name) {
    // Your implementation goes here
    printf("save_feature\n");
  }

  int32_t update_feature(const std::string& camera_name, const std::string& content) {
    // Your implementation goes here
    printf("update_feature\n");
  }

  int64_t ping_server(const int64_t seq) {
    // Your implementation goes here
    printf("ping_server\n");
  }

};

int main(int argc, char **argv) {
  int port = 9090;
  shared_ptr<AgentServerServiceHandler> handler(new AgentServerServiceHandler());
  shared_ptr<TProcessor> processor(new AgentServerServiceProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

