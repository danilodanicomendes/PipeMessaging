#pragma once
#include <Windows.h>
#include "DedicatedServer.h"

/*
	Represents a dedicated server for admin users
*/
class DedicatedServerAdmin : public DedicatedServer {
public:
	// Constructor : Server * server, TCHAR login[TAMLOGIN], TCHAR password[TAMPASS], short int tipo, HANDLE hPipe
	DedicatedServerAdmin(Server * server, TCHAR login[TAMLOGIN], TCHAR password[TAMPASS], 
		short int tipo, HANDLE hPipe);

	DWORD WINAPI threadMethod(LPVOID param);
};