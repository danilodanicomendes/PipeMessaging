#include "DedicatedServerAdmin.h"
#include "Server.h"

DedicatedServerAdmin::DedicatedServerAdmin(Server * server, TCHAR login[TAMLOGIN], TCHAR password[TAMPASS], short int tipo,
										   HANDLE hPipe) : 
	DedicatedServer(server, login, password, tipo, hPipe) {};


DWORD WINAPI DedicatedServerAdmin::threadMethod(LPVOID param) {
	BOOL ret = FALSE;
	TCHAR buf[256];
	DWORD n;
	//HANDLE hPipe2 = (HANDLE) param;
	TCHAR msg[256];
	int i = 0;

	_tprintf(TEXT("[SERVIDOR-%d] Um cliente ligou-se...\n"), GetCurrentThreadId());

	while (1) {
		ret = ReadFile(hPipe, buf, sizeof(buf), &n, NULL);
		buf[n / sizeof(TCHAR)] = '\0';
		if (!ret || !n)
			break;
		_tprintf(TEXT("[SERVIDOR-%d] Recebi %d bytes: '%s'... (ReadFile)\n"), GetCurrentThreadId(), n, buf);

		for (i = 0; i < server->getServers()->size(); i++) {
			if (server->getServers()->at(i) != NULL)
				WriteFile(server->getServers()->at(i)->getPipe(), buf, _tcslen(buf)*sizeof(TCHAR), &n, NULL);
		}

	}
	_tprintf(TEXT("[SERVIDOR-%d] Vou desligar o pipe... (DisconnectNamedPipe/CloseHandle)\n"), GetCurrentThreadId());
	if (!DisconnectNamedPipe(hPipe)){
		_tperror(TEXT("Erro ao desligar o pipe!"));
		ExitThread(-1);
	}

	CloseHandle(hPipe);
	return 0;
}