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
  AgentServerServiceHandler() {
    // Your initialization goes here
  }

  std::map<std::string, std::map<int32_t, std::string>>		    m_total_cameras;
  std::set<std::string>										m_hold_camera_names;
  std::map<std::string, std::shared_ptr<CCamera>>			m_hold_cameras;

  void find_cameras(std::map<std::string, std::map<int32_t, std::string> > & _return) {
    // Your implementation goes here
	printf("find_cameras\n");
	m_total_cameras.clear();
	CCamera::FindCamera(&m_total_cameras);
	CCamera::RegisterServerCallback(&m_hold_cameras);
	
	_return = m_total_cameras;
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
  int32_t exec_acquire_store(const std::string& cmdline) {
	  // Your implementation goes here
	  printf("exec_acquire_store\n");
#if 1
	  STARTUPINFO si = { sizeof(si) };
	  PROCESS_INFORMATION pi;
	  si.dwFlags = STARTF_USESHOWWINDOW;
	  si.wShowWindow = TRUE;
	  fprintf(stdout, "%s cmd:%s\n", __FUNCTION__, cmdline.c_str());
	  BOOL bRet = ::CreateProcess(NULL, (char*)cmdline.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
	  int error = GetLastError();
	  if (bRet) {
		  ::CloseHandle(pi.hThread);
		  ::CloseHandle(pi.hProcess);
		  printf(" The process id:%d \n", pi.dwProcessId);
		  printf(" The main thread id of the process:%d \n", pi.dwThreadId);
		  return pi.dwProcessId;
	  }
	  else {
		  printf("error code:%d\n", error);
		  return -1;
	  }
#endif	
  }

  int32_t kill_acquire_store(const int64_t process_id) {
	  // Your implementation goes here
	  printf("kill_acquire_store\n");
	  HANDLE hPrc;
	  if (0 == process_id) return FALSE;

	  hPrc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);  // Opens handle to the process.
	  if (!TerminateProcess(hPrc, 0))  {
		  CloseHandle(hPrc);
		  return FALSE;
	  }
	  else
		  WaitForSingleObject(hPrc, 300); // At most ,waite 300  millisecond.

	  CloseHandle(hPrc);
	  return TRUE;
  }

};