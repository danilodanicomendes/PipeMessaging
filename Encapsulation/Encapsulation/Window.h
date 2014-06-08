#pragma once
#include <windows.h>
#include "resource.h"
//#include "DLLTP.h"
#include "config.h"

using namespace std;

/*  Class Window
Faz de wrapper class, distribuindo os eventos que o WndProc pelas janelas em questão
usando static polymorphism e Curiously Recurring Template Pattern (CRTP).

O WndProc tem que ficar como static neste caso porque não pode ter o parametro this.

Nota: Não está dividida em .cpp porque as templates têm que estar todas num ficheiro, pois
é um padrão (não uma classe) que indica ao compilador como fazer as coisas.
*/
template <typename T> class Window {
protected:
	// Handler da janela, são específicos de cada janela
	HWND hWnd;

	static BOOL started;

	static HANDLE hPipePublic;
private:
	HINSTANCE hInstance;
	// Conta numero total de janelas criadas por causa do ponteiro a usar no WndProc
	static int nWindows;
	// É o indixe da janela em questão
	int index;
	
	const TCHAR * WindowClassName;

	// Define características específicas da janela
	WNDCLASSEX _WndClsEx;

	// Define tratamento de eventos da janela
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		Window * WindowPtr = (Window *) GetWindowLongPtr(hWnd, 0); // é o this da class

		if (WindowPtr == NULL)
			return(DefWindowProc(hWnd,msg,wParam,lParam));
		else
			return static_cast<T*>(WindowPtr)->WndProc(hWnd, msg, wParam, lParam);

		return(0);
	}

public:
	// Construtor
	Window(LPCTSTR lpWindowName, DWORD dwStyle, int x, int y,
		int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, TCHAR * WindowClassName, 
		unsigned int backgroundImg, LPCTSTR menu, HINSTANCE hInst) {
			index = nWindows;
			nWindows++;
			// Estes procedimentos só precisam ser executados uma vez
			if(!started) {
				hInstance = hInst; //GetModuleHandle(NULL);
				// Regista Janela
				ATOM ola = RegisterWindowClass(hInstance, WindowClassName, backgroundImg, menu);
				started = TRUE;
			}
			// getHandler
			hWnd = ::CreateWindow(WindowClassName, lpWindowName,
				dwStyle, x, y, nWidth, nHeight, hWndParent,
				hMenu, hInstance, 0);

			SetWindowLongPtr(hWnd, 0, (long) this);
	}

	// Regista Janela. ATOM é uma tabela de strings e identificadores correspondentes
	// Talvez fosse mais fácil passar o preenchimento para as derivadas...
	ATOM RegisterWindowClass(HINSTANCE hInstance, TCHAR * WindowClassName, 
		unsigned int backgroundImg, LPCTSTR hMenu) {
		_WndClsEx.cbSize = sizeof(WNDCLASSEX);
		_WndClsEx.style = CS_HREDRAW | CS_VREDRAW;
		_WndClsEx.lpfnWndProc = WndProc;
		_WndClsEx.cbClsExtra = 0;
		//_WndClsEx.cbWndExtra = sizeof(Window *); // Arranja espaço para guardar o ponteiro
		_WndClsEx.cbWndExtra = sizeof(int);
		_WndClsEx.hInstance = hInstance; // Inicializar isto
		_WndClsEx.hIcon = LoadIcon(hInstance, (LPCTSTR) IDI_ICON1); // Sem icon por enquanto
		_WndClsEx.hbrBackground = CreatePatternBrush(LoadBitmap( hInstance, MAKEINTRESOURCE(backgroundImg))); // Imagem varia consoante janela
		_WndClsEx.lpszMenuName = (LPCWSTR) MAKEINTRESOURCE(hMenu);
		_WndClsEx.lpszClassName = WindowClassName; // Inicializar isto
		_WndClsEx.hIconSm = NULL;
		_WndClsEx.hCursor = LoadCursor(NULL, IDC_ARROW);
		return RegisterClassEx(&_WndClsEx);
	}

	/* Serve para mostrar janela e para fazer update da janela*/
	BOOL showWindow(int iCmdShow){
		if (!ShowWindow(hWnd, iCmdShow))
			return FALSE;					

		if (!UpdateWindow(hWnd))		
			return FALSE;					

		return TRUE;
	}

	HINSTANCE gethInstance() {return hInstance;}
};

template <typename T> BOOL Window<T>::started = FALSE;
template <typename T> int Window<T>::nWindows = 0;
template <typename T>  HANDLE Window<T>::hPipePublic = NULL;