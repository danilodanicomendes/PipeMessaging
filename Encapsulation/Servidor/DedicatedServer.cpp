#include "DedicatedServer.h"

DedicatedServer::DedicatedServer(Server * server, TCHAR login[TAMLOGIN], TCHAR password[TAMPASS], short int tipo, 
								 HANDLE hPipe) : server(server), hPipe(hPipe) {

	_stprintf_s(user.login, sizeof(user.login)/sizeof(TCHAR), login);
	_stprintf_s(user.password, sizeof(user.password)/sizeof(TCHAR), password);
	user.tipo = tipo;
}

void DedicatedServer::setHandleThread(HANDLE handleThread) {this->hThread = handleThread;}

void DedicatedServer::setHandlePipe(HANDLE handlePipe) {this->hPipe = handlePipe;}

HANDLE * DedicatedServer::getPipe() {return &hPipe;}