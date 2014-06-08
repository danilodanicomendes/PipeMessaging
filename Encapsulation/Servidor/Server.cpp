#include <tchar.h>
#include <process.h> // _beginthreadex()
#include <errno.h> // erro da thread
#include "Server.h"

using namespace std;

Server::Server() : PIPE_NAME(TEXT("\\\\.\\pipe\\PipeMessaging")) {

}

void Server::run() {
	HANDLE hPipe;
	HANDLE hThread;

	while(1) {
		_tprintf(TEXT("[SERVER] Creating instance of '%s')\n"), PIPE_NAME);
		hPipe = (CreateNamedPipe(PIPE_NAME, PIPE_ACCESS_DUPLEX, PIPE_WAIT | PIPE_TYPE_MESSAGE
			| PIPE_READMODE_MESSAGE, PIPE_UNLIMITED_INSTANCES, sizeof(UTILIZADOR[100]), sizeof(UTILIZADOR[100]), 0, NULL));
		if(hPipe == INVALID_HANDLE_VALUE){
			_tperror(TEXT("[ERROR] Client limit reached."));
			break;
		}

		_tprintf(TEXT("[SERVER] Waiting client...\n"));
		if(!ConnectNamedPipe(hPipe, NULL)) {
			_tperror(TEXT("[ERROR] Connecting client."));
			exit(-1);
		}
		
		// TO DO: Validate user
		if (validUser(&hPipe)) {
			// User has inputed valid login
			// Start Dedicated server 
			//Server * server, TCHAR login[TAMLOGIN], TCHAR password[TAMPASS], short int tipo, HANDLE hThread, HANDLE hPipe
			servers.push_back(new DedicatedServerAdmin(this, TEXT("admin"), TEXT("admin"), 2, hPipe));
			// Begin thread for dedicated server according to logged user, with an instance of the pipe
			// Maybe put inside constructor of DedicatedServerAdmin/User? Doesn't seem safe
			hThread = (HANDLE) _beginthreadex(NULL, 0, (unsigned (__stdcall*)(void*)) ThreadDistributor,
				this, 0, NULL);
			if (errno == EAGAIN || errno == EINVAL){
				_tperror(TEXT("[ERROR] Creating thread."));
				exit(0);
			}
			// Set thread handle
			servers.at(servers.size()-1)->setHandleThread(hThread);
			// Just to certify that the thread has time to be distributed 
			Sleep(100);
		}
	}
}

vector <DedicatedServer*> * Server::getServers() {return &servers;}

DWORD WINAPI Server::ThreadDistributor(LPVOID param) {
	return ((Server*)param)->servers.at(((Server*)param)->servers.size()-1)->threadMethod(param);
}

const BOOL Server::validUser(HANDLE * hPipe) {
	BOOL ret;
	DWORD numberOfBytesRead, numberOfBytesWritten;
	UTILIZADOR user;
	validation is = validation::FAILED;
	
	ret = ReadFile(*hPipe, &user, sizeof(user), &numberOfBytesRead, NULL);
	
	_tprintf(TEXT("[SERVIDOR-%d] Recebi %s %s\n"), GetCurrentThreadId(), user.login, user.password);
	// Insert Validation here
	WriteFile(*hPipe, &is, sizeof(TCHAR), &numberOfBytesWritten, NULL);

	return true;
}