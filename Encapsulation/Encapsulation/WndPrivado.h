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

	// Funções

	void onCreate();

	// Não precisa ser WINAPI porque a função ThreadDistributor passa para esta thread
	DWORD ThreadMsgsChatFunction();

	void onClick();

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
	WndPrivado(LPCTSTR menu, HINSTANCE hInst);

	/*
		Como a função da thread tem que ser static (dentro da class, fora podia ser global por ex),
		fiz esta classe para distribuir as funções, recebendo o ponteiro this reencaminha para quem
		de direito e posso tratar de forma non-static o que quiser.
	*/
	static DWORD WINAPI ThreadDistributor(LPVOID param);

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};