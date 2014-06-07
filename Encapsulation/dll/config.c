#include "config.h"

int Autenticar(TCHAR *login, TCHAR *pass) {
	UTILIZADOR account;
	DWORD numberOfBytesWritten;

	// Create pipe
	if (!WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER)) {
        //_tprintf(TEXT("[ERROR] Error connecting to server. Try again later...\n"), PIPE_NAME);
		exit(0);
    }
	if ((hPipePublic = CreateFile(PIPE_NAME, GENERIC_WRITE | GENERIC_READ, 0, NULL, 
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == NULL) {
        //_tprintf(TEXT("[ERROR] Error connecting to server. Try again later...\n"), PIPE_NAME);
		exit(0);
    }
	// Fill login struct (user & password)
	_tcscpy_s(account.login, login);
	_tcscpy_s(account.password, pass);
	// Send login
	if (!WriteFile(hPipePublic, &account, sizeof(UTILIZADOR), &numberOfBytesWritten, NULL)) {
		//_tperror(TEXT("[ERROR] Error sending message to server...\n"));
		exit(0);
	}
}