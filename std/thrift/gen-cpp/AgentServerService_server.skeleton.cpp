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

  void find_cameras(std::map<std::string, std::map<int32_t, std::string> > & _return) {
    // Your implementation goes here
    printf("find_cameras\n");
  }

  int32_t exec_program(const std::string& cmdline) {
    // Your implementation goes here
    printf("exec_program\n");
  }

  int32_t kill_program(const int64_t process_id) {
    // Your implementation goes here
    printf("kill_program\n");
  }

  void get_disk_info(std::map<std::string, std::vector<double> > & _return) {
    // Your implementation goes here
    printf("get_disk_info\n");
  }

  int32_t get_cpu_usage() {
    // Your implementation goes here
    printf("get_cpu_usage\n");
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
