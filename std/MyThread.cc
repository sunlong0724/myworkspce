#include "MyThread.h"



CMyThread::CMyThread() :m_exited(true) {
}

CMyThread::~CMyThread() {}

void CMyThread::start() {
	if (m_exited) {
		m_exited = false;
		m_impl = std::thread(&CMyThread::run, this);
	}
}

void CMyThread::stop() {
	m_exited = true;
	if (m_impl.joinable()) {
		m_impl.join();
	}
}

void CMyThread::run() {}
