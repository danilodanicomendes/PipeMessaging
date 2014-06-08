#include "config.h"

int Autenticar(HANDLE &hPipePublic, TCHAR *login, TCHAR *pass) {
	UTILIZADOR account;
	DWORD numberOfBytesWritten, numberOfBytesRead;
	validation result;

	// Fill login struct (user & password)
	_tcscpy_s(account.login, login);
	_tcscpy_s(account.password, pass);
	// Send login
	if (!WriteFile(hPipePublic, &account, sizeof(account), &numberOfBytesWritten, NULL)) {
		//_tperror(TEXT("[ERROR] Error sending message to server...\n"));
		exit(0);
	}
	// Give time to server stay on ReadFile
	Sleep(100);
	// TO DO: Validação do readfile
	ReadFile(hPipePublic, &result, sizeof(result), &numberOfBytesRead, NULL);
	
	return (int)result;
}

void setPipeName() {
	TCHAR pipeAddress[TAMTEXTO];
	HKEY hKey;
	DWORD result, version;

	RegGetValue(HKEY_LOCAL_MACHINE, TEXT("Software\\PipeMessaging"), TEXT("PipeAddress"), 
		RRF_RT_ANY, NULL, pipeAddress, &result);

	_tcscpy_s(PIPE_NAME, _tcslen(PIPE_NAME)*sizeof(TCHAR), pipeAddress);
}

int LerListaUtilizadores(UTILIZADOR *utilizadores) {return 0;}
int LerListaUtilizadoresRegistados(UTILIZADOR *utilizadores) {return 0;}
int IniciarConversa(TCHAR *utilizador) {return 0;}
int DesligarConversa() {return 0;}
int EnviarMensagemPrivada(TCHAR *msg) {return 0;}
void EnviarMensagemPública(TCHAR *msg) {return;}
CHAT LerInformacaoInicial() {CHAT a; return a;}
MENSAGEM LerMensagensPublicas() {MENSAGEM a; return a;}
MENSAGEM LerMensagensPrivadas() {MENSAGEM a; return a;}
int Sair() {return 0;}
int Desligar() {return 0;}