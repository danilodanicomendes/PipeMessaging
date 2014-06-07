#pragma once
#include <Window.h>
#include "Window.h"
#include "Thread.h"
#include "WndPrivado.h"

// Janela Chat public para testes por enquanto.
class WndInicial : public Window<WndInicial> {
	HWND hUsersListBox;
	HWND hMsgEdit;
	HWND hChatHistoricListBox;
	HWND hSendButton;

	HANDLE mutex;

	WndPrivado * janelaPrivada;

	static bool doQuit;

	UTILIZADOR users[NUMUTILIZADORES];
	int total;
	UTILIZADOR online[NUMUTILIZADORES];
	int totalonline;

	CHAT chatPublico;

	//Threads dados
	Thread ThreadListaUsers;
	Thread ThreadMsgsChat;

	// Identificador da thread e serve para diferênciar thread no distributor
	static DWORD Tindex;
	
	// Flag das threads. Serve para elas terminarem o ciclo ininito em que estão.
	static bool stopListaUsers;
	static bool stopMsgsChat;

	//Funções

	void onCreate();

	// Não precisa ser WINAPI porque a função ThreadDistributor passa para esta thread
	DWORD ThreadListaUsersFunction();
	DWORD ThreadMsgsChatFunction();

	void onClick();
	
	void onStartChatPrivado(LPARAM lParam);

	void onOpcao();
	static BOOL CALLBACK DialogOpcao(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// Para criar ou apagar utilizadores. DLL ainda não tem funções para isto, assumo que
	// não seja para implementar ainda para esta primeira fase.
	void onGerir(); 
	static BOOL CALLBACK DialogGerir(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void onHelp();

	void onSobre();

	void onLogout();

public:
	// Construtor
	WndInicial(LPCTSTR menu, HINSTANCE hInst);

	/*
		Como a função da thread tem que ser static (dentro da class, fora podia ser global por ex),
		fiz esta classe para distribuir as funções, recebendo o ponteiro this reencaminha para quem
		de direito e posso tratar de forma non-static o que quiser.
	*/
	static DWORD WINAPI ThreadDistributor(LPVOID param);

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};