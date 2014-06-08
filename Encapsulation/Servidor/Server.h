#pragma once
#include <Windows.h>
#include <vector>
#include <string>
#include "DedicatedServer.h"
#include "DedicatedServerAdmin.h"
#include "DedicatedServerUser.h"

using namespace std;

class Server {
	// Dedicated server handles
	vector <DedicatedServer*> servers;
	// Pipe name
	TCHAR * PIPE_NAME;
	// Responsible for reading inputed user and sending response
	// return validation::SUCCESS || validation::FAILED
	const BOOL validUser(HANDLE * hPipe);
public:
	// Constructor
	Server();

	void run();

	vector <DedicatedServer*> * getServers();

	static DWORD WINAPI ThreadDistributor(LPVOID param);
};