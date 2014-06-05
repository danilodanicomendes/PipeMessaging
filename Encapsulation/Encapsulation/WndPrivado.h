#pragma once
#include <Window.h>
#include "Window.h"
#include "Thread.h"


class WndPrivado : public Window<WndPrivado> {
	HWND hUsersListBox2;
	HWND hMsgEdit2;
	HWND hChatHistoricListBox2;
	HWND hSendButton2;

	HANDLE mutex;

	static bool doQuit;

	CHAT chatPrivado;

	// Thread dados
	Thread ThreadMsgsChat;

	static bool stopMsgsChat;

	// Fun��es

	void onCreate();

	// N�o precisa ser WINAPI porque a fun��o ThreadDistributor passa para esta thread
	DWORD ThreadMsgsChatFunction();

	void onClick();

	void onOpcao();
	static BOOL CALLBACK DialogOpcao(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// Para criar ou apagar utilizadores. DLL ainda n�o tem fun��es para isto, assumo que
	// n�o seja para implementar ainda para esta primeira fase.
	void onGerir(); 
	static BOOL CALLBACK DialogGerir(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void onHelp();

	void onSobre();

	void onLogout();
public:
	// Construtor
	WndPrivado(LPCTSTR menu, HINSTANCE hInst);

	/*
		Como a fun��o da thread tem que ser static (dentro da class, fora podia ser global por ex),
		fiz esta classe para distribuir as fun��es, recebendo o ponteiro this reencaminha para quem
		de direito e posso tratar de forma non-static o que quiser.
	*/
	static DWORD WINAPI ThreadDistributor(LPVOID param);

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};