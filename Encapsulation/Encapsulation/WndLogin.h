#pragma once
#include <Window.h>
#include "Window.h"

/* Janela de Login
   
   Nota: Para apanhar WM_CHAR quando se tem focus na edit � preciso fazer uma subclass da janela edit
         Caso tenha tempo..
*/
class WndLogin : public Window<WndLogin> {
	static unsigned int backgroundImg;

	HWND hStaticUser;
	HWND hStaticPass;

	HWND hUser;
	HWND hPass;
	HWND hLoginButton;

	PAINTSTRUCT paintStruct;

	UTILIZADOR userLoginInfo;

	// Serve esta vari�vel para dizer se � para fechar janela ou aplica��o
	static bool doQuit;

	// Fun��es

	bool getStarted();

	void onCreate();

	void onClick();

	void onEditUserFocus();

	// Altera fundo dos controlos static. Retorna brush utilizada
	LRESULT setStaticsBk(WPARAM wParam, LPARAM lParam);

	// Altera fundo dos controlos edits. Retorna brush utilizada
	LRESULT setEditsBk(WPARAM wParam, LPARAM lParam);

	// If registry doesnt have IP from server DialogBox
	static BOOL CALLBACK DialogSetPipeToRegistry(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	int onOK_DialogSetPipeToRegistry(HWND hWnd);
public:
	// Construtor possu� caracter�sticas da janela. Desta forma n�o me engano.
	WndLogin(HINSTANCE hInst);

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};