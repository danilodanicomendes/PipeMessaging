#include "DedicatedServerAdmin.h"
#include "Server.h"

DedicatedServerAdmin::DedicatedServerAdmin(Server * server, TCHAR login[TAMLOGIN], TCHAR password[TAMPASS], short int tipo,
										   HANDLE hPipe) : 
	DedicatedServer(server, login, password, tipo, hPipe) {};


DWORD WINAPI DedicatedServerAdmin::threadMethod(LPVOID param) {
	BOOL ret = FALSE;
	TCHAR buf[256];
	DWORD n;
	TCHAR msg[256];
	int i = 0;

	_tprintf(TEXT("[SERVER-%d-ADMIN] Client connecting..\n"), GetCurrentThreadId());

	while (1) {
		ret = ReadFile(hPipe, buf, sizeof(buf), &n, NULL);
		buf[n / sizeof(TCHAR)] = '\0';
		if (!ret || !n)
			break;
		_tprintf(TEXT("[SERVER-%d-ADMIN] Recebi %d bytes: '%s'... (ReadFile)\n"), GetCurrentThreadId(), n, buf);

		for (i = 0; i < server->getServers()->size(); i++) {
			if (server->getServers()->at(i) != NULL)
				WriteFile(server->getServers()->at(i)->getPipe(), buf, _tcslen(buf)*sizeof(TCHAR), &n, NULL);
		}

	}
	_tprintf(TEXT("[SERVER-%d-ADMIN] Disconnecting pipe..\n"), GetCurrentThreadId());
	if (!DisconnectNamedPipe(hPipe)){
		_tperror(TEXT("[ERROR] Disconnecting pipe."));
		ExitThread(-1);
	}

	CloseHandle(hPipe);
	return 0;
}