#pragma once
#include <Windows.h>
#include "config.h"

class Server;

/*
	Abstract class
	Function: Defines the behavior of the created thread, depending on the logged in user

	Contructor: 
*/
class DedicatedServer {
private:
	DedicatedServer() {};
protected:
	// Pointer do main server, mainly because of broadcast messages
	Server * server;

	HANDLE hThread;
	HANDLE hPipe;

	UTILIZADOR user;
public:
	// Constructor
	DedicatedServer(Server * server, TCHAR login[TAMLOGIN], TCHAR password[TAMPASS], short int tipo, HANDLE hPipe);

	// Destructor
	// TO DO - Implementar fechamento dos handles todos abertos
	~DedicatedServer() {};

	// Virtual method
	virtual DWORD WINAPI threadMethod(LPVOID param) = 0;

	// Gets & Sets
	void setHandleThread(HANDLE handleThread);
	void setHandlePipe(HANDLE handlePipe);
	HANDLE * getPipe();
};