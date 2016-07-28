// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "AgentServerService.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>


#include "Camera.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ::hawkeye;

class AgentServerServiceHandler : virtual public AgentServerServiceIf {
 public:
	 std::map<std::string, std::pair<std::string, int>>		m_total_cameras;
	 std::set<std::string>								    m_hold_camera_names;
	 std::map<std::string, std::shared_ptr<CCamera>>		m_hold_cameras;

  AgentServerServiceHandler() {
    // Your initialization goes here
  }

  void find_cameras(std::vector<std::string> & _return) {
    // Your implementation goes here
    printf("find_cameras\n");
	m_total_cameras.clear();
	CCamera::FindCamera(&m_total_cameras);
	CCamera::RegisterServerCallback(&m_hold_cameras);
	for (auto& a : m_total_cameras) {
		_return.push_back(a.first);
	}
  }

  int32_t add_cameras(const std::vector<std::string> & l) {
    // Your implementation goes here
    printf("add_cameras\n");
	int count = 0;
	for (int i = 0; i < l.size(); ++i) {
		for (auto& it = m_total_cameras.begin(); it != m_total_cameras.end(); ++it) {
			if (strcmp(l[i].c_str(), it->first.c_str()) == 0) {
				++count;
				m_hold_camera_names.insert(it->first);
			}
		}
	}
	return count;
  }

  void get_hold_cameras(std::vector<std::string> & _return) {
    // Your implementation goes here
    printf("get_hold_cameras\n");

	_return.clear();
	for (auto& a : m_hold_camera_names) {
		_return.push_back(a);
	}
  }

  int32_t del_cameras(const std::vector<std::string> & l) {
    // Your implementation goes here
    printf("del_cameras\n");

	int count = 0;
	for (int i = 0; i < l.size(); ++i) {
		std::set<std::string>::iterator it = m_hold_camera_names.find(l[i]);
		if (it != m_hold_camera_names.end()) {
			m_hold_camera_names.erase(it);
			++count;
		}
	}
	return count;
  }

};

