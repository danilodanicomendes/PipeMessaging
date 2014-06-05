#pragma once
#include <Windows.h>

// Serve esta classe para guardar dados referentes às threads
class Thread {
	HANDLE ThreadHwnd;
	DWORD ThreadId;
public:
	HANDLE getThreadHandle();
	void setThreadHandle(HANDLE ThreadHwnd);

	DWORD getThreadId();
	void setThreadId(DWORD ThreadId);
};