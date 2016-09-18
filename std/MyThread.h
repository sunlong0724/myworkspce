#ifndef __MY_THREAD_H__
#define __MY_THREAD_H__
#define DLL_API __declspec(dllexport)  

#include <thread>

class DLL_API CMyThread {

public:
	CMyThread();

	virtual ~CMyThread();
	
	void start();

	void stop();

protected:
	virtual void run();
	bool			m_exited;

private:
	std::thread		m_impl;
};

#endif // !__PROCESSOR_H__
