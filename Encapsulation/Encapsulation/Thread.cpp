#include "Thread.h"

HANDLE Thread::getThreadHandle() {
	return ThreadHwnd;
}
void Thread::setThreadHandle(HANDLE ThreadHwnd) {
	this->ThreadHwnd = ThreadHwnd;
}

DWORD Thread::getThreadId() {
	return ThreadId;
}
void Thread::setThreadId(DWORD ThreadId) {
	this->ThreadId = ThreadId;
}