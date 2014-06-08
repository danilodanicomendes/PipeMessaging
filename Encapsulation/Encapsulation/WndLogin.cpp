#include <windows.h>
#include <tchar.h>
#include "WndLogin.h"
#include "WndInicial.h"
#include "resource.h"


WndLogin::WndLogin(HINSTANCE hInst) :
	Window<WndLogin>(TEXT("PipeMessaging - Login"),  WS_OVERLAPPED | WS_SYSMENU, 200, 200,
	615, 435, NULL, NULL, TEXT("JanelaLogin"), IDB_BITMAP2, NULL, hInst) {

		// Como estou a fazer a inicialização pela derivada e não pela base, o
		// evento WM_CREATE na derivada nunca era efetuado a tempo de ter o hWnd
		// bem preenchido. Por isso envio mensagem com WM_CREATE após inicialização
		// da base e contrução de ponteiro.
		SendMessage(hWnd, WM_CREATE, NULL, NULL); 
}

bool WndLogin::doQuit = true;

unsigned int WndLogin::backgroundImg = IDB_BITMAP1;

void WndLogin::onCreate() {
	// If registry doesnt have address yet
	HKEY hKey;
	DWORD result;
	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\PipeMessaging"),0, NULL, REG_OPTION_NON_VOLATILE,
				KEY_ALL_ACCESS, NULL, &hKey, &result) != ERROR_SUCCESS) {
				MessageBox(hWnd, TEXT("Error inserting/accessing address to registry. Try again later"), TEXT("PipeMessagin - Error"), MB_OK);
				exit(0);
	} else if (result == REG_CREATED_NEW_KEY) {
		// Start dialog
		DialogBox(gethInstance(), (LPCWSTR) IDD_DIALOG3, hWnd, (DLGPROC) DialogSetPipeToRegistry);
	}
	RegCloseKey(hKey);
	setPipeName();

	// User name static label and text box
	hStaticUser = CreateWindow(TEXT("STATIC"), TEXT("Username:"), WS_CHILD | WS_VISIBLE, 
		110, 250, 150, 20, hWnd, (HMENU) IDC_STATIC_USER, NULL, 0);
	hUser = CreateWindow(TEXT("EDIT"), TEXT("Insira username aqui..."), WS_CHILD | WS_VISIBLE, 
		110, 270, 150, 20, hWnd, (HMENU) IDC_EDIT_USER, NULL, 0);
	// Password static label and text box
	hStaticPass = CreateWindow(TEXT("STATIC"), TEXT("Password:"), WS_CHILD | WS_VISIBLE, 
		110, 300, 150, 20, hWnd, (HMENU) IDC_STATIC_PASS, NULL, 0);
	hPass = CreateWindow(TEXT("EDIT"), 0, WS_CHILD | WS_VISIBLE | ES_PASSWORD, 
		110, 320, 150, 20, hWnd, (HMENU) IDC_EDIT_PASS, NULL, 0);
	// Login button
	hLoginButton = CreateWindow(TEXT("BUTTON"), TEXT("Login"), WS_BORDER | WS_CHILD | WS_VISIBLE | BS_FLAT, 
		453, 306, 100, 40, hWnd, (HMENU) IDC_BUTTON_LOGIN, NULL, 0);

	// Pinta static labels, a mensagem tem que ser enviada para o pai do controlo, wParam: hdc lParam: hWndControl
	SendMessage(hWnd, WM_CTLCOLORSTATIC, (WPARAM) GetDC(hUser), (LPARAM) hStaticUser);
	SendMessage(hWnd, WM_CTLCOLOREDIT, (WPARAM) GetDC(hUser), (LPARAM) hStaticPass);

	// Envio mensagem de WM_SETTEXT para preencher editBox do username
	// SendMessage(hUser, WM_SETTEXT, NULL, (LPARAM) TEXT("Insira username aqui..."));
	// Ponho font default do windows nas edit controls
	SendMessage(hUser, WM_SETFONT, WPARAM ((HFONT)GetStockObject(DEFAULT_GUI_FONT)), TRUE);
	SendMessage(hPass, WM_SETFONT, WPARAM ((HFONT)GetStockObject(DEFAULT_GUI_FONT)), TRUE);
}

void WndLogin::onClick() {
	TCHAR bufferUser[TAMLOGIN];
	TCHAR bufferPass[TAMPASS];
	TCHAR msg[300];

	GetWindowText(hUser, bufferUser, sizeof(bufferUser)/sizeof(TCHAR));
	GetWindowText(hPass, bufferPass, sizeof(bufferPass)/sizeof(TCHAR));

	// Create pipe
	if (!WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER)) {
		//_tprintf(TEXT("[ERROR] Error connecting to server. Try again later...\n"), PIPE_NAME);
		exit(0);
	}
	if ((hPipePublic = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, 
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == NULL) {
			//_tprintf(TEXT("[ERROR] Error connecting to server. Try again later...\n"), PIPE_NAME);
			exit(0);
	}

	// Validate user with dll function (config.dll::Autenticar(...))
	if (_tcscmp(bufferUser, TEXT("")) != 0 && Autenticar(hPipePublic, bufferUser, bufferPass)) {
		_stprintf_s(msg, sizeof(msg)/sizeof(TCHAR), TEXT("Bem vindo, %s."), bufferUser);
		MessageBox(hWnd, msg, TEXT("Login - Success"), MB_ICONINFORMATION);
		
		Sleep(100); // Just so client thread on server has time to load

		// Instância janela principal
		if (_tcscmp(bufferUser, TEXT("admin")) == 0) {
			WndInicial w((LPCTSTR) IDR_MENU2, gethInstance());
			w.showWindow(SW_SHOWNORMAL);
		}
		else {
			WndInicial w((LPCTSTR) IDR_MENU1, gethInstance());
			w.showWindow(SW_SHOWNORMAL);
		}
		
		// Flag da janela base
		started = FALSE;
		// Fecha apenas janela do login
		doQuit = false;
		SendMessage(hWnd, WM_CLOSE, NULL, NULL);
	} else {
		_stprintf_s(msg, sizeof(msg)/sizeof(TCHAR), TEXT("User \"%s\" e/ou password são inválidos."), bufferUser);
		MessageBox(NULL, msg, TEXT("Login - Failed"), MB_ICONERROR);
		// Close Pipe Handle
		CloseHandle(hPipePublic);
	}
}

void WndLogin::onEditUserFocus() {
	SetWindowText(hUser, TEXT(""));
}

LRESULT WndLogin::setStaticsBk(WPARAM wParam, LPARAM lParam) {
	HDC hdc = (HDC)wParam;
	int controlID = GetDlgCtrlID((HWND) lParam);

	if (controlID == IDC_STATIC_USER || controlID == IDC_STATIC_PASS) {
		SetBkColor(hdc, RGB(80, 177, 62));
		SetTextColor(hdc, RGB(255, 255, 255));
		SetDCBrushColor(hdc, RGB(80, 177, 62));
		return (LRESULT) GetStockObject(DC_BRUSH);
	}
	return NULL;
}

LRESULT WndLogin::setEditsBk(WPARAM wParam, LPARAM lParam) {
	HDC hdc = (HDC)wParam;
	int controlID = GetDlgCtrlID((HWND) lParam);

	if (controlID == IDC_EDIT_USER || controlID == IDC_EDIT_PASS) {
		SetBkColor(hdc, RGB(58, 131, 46));
		SetTextColor(hdc, RGB(255, 255, 255));
		SetDCBrushColor(hdc, RGB(80, 177, 62));
		return (LRESULT) GetStockObject(DC_BRUSH);
	}
	return NULL;
}

int WndLogin::onOK_DialogSetPipeToRegistry(HWND hWnd) {
	// If time, move to config.dll
	TCHAR pipeAddress[TAMTEXTO];
	HKEY hKey;
	DWORD result, version;
	// Read value from editBox
	GetWindowText(GetDlgItem(hWnd, IDC_EDIT1), pipeAddress, _tcslen(pipeAddress)*sizeof(TCHAR));
	// Insert address into registry
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,TEXT("Software\\PipeMessaging"),0, NULL, REG_OPTION_NON_VOLATILE,
				KEY_ALL_ACCESS, NULL, &hKey, &result) != ERROR_SUCCESS) {
				MessageBox(hWnd, TEXT("Error inserting/accessing address to registry. Try again later"), TEXT("PipeMessagin - Error"), MB_OK);
				exit(0);
	} else if (result == REG_CREATED_NEW_KEY) {
		RegSetValueEx(hKey, TEXT("PipeAddress"), 0, REG_SZ, (LPBYTE) pipeAddress, _tcslen(pipeAddress)*sizeof(TCHAR));
		version = 1;
		RegSetValueEx(hKey, TEXT("Version"), 0, REG_DWORD, (LPBYTE)&version, sizeof(DWORD));
		RegCloseKey(hKey);
	} else {
		// Do nothing
		RegCloseKey(hKey);
	}
	setPipeName();

	return 0;
}

BOOL CALLBACK WndLogin::DialogSetPipeToRegistry(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	WndLogin * WindowPtr = (WndLogin *) GetWindowLongPtr(hWnd, 0);
	switch(msg) {
	case WM_INITDIALOG:
		SetWindowText(hWnd, TEXT("PipeMessaging - Set Pipe Server Address"));
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			WindowPtr->onOK_DialogSetPipeToRegistry(hWnd);
			EndDialog(hWnd, 0);
			break;
		}
		break;
	case WM_CLOSE:
		break; // Don't allow close
	default:
		return 0;
		break;
	}
	return 0;
}

LRESULT CALLBACK WndLogin::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	WndLogin * WindowPtr = (WndLogin *) GetWindowLongPtr(hWnd, 0);
	LRESULT aux;
	switch (msg) {
	case WM_CREATE:
		WindowPtr->onCreate();
		break;
	case WM_CTLCOLORSTATIC:
		aux = WindowPtr->setStaticsBk(wParam, lParam);
		if (aux == NULL)
			return DefWindowProc(hWnd, msg, wParam, lParam);
		else
			return aux; // Serve isto para retornar a brush.
		break;
	case WM_CTLCOLOREDIT:
		aux = WindowPtr->setEditsBk(wParam, lParam);
		if (aux == NULL)
			return DefWindowProc(hWnd, msg, wParam, lParam);
		else
			return aux; // Serve isto para retornar a brush.
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_BUTTON_LOGIN:
			WindowPtr->onClick();
			break;
		case IDC_EDIT_USER:
			switch(HIWORD(wParam)) {
			case EN_SETFOCUS:
				// Edit é seleccionada
				WindowPtr->onEditUserFocus();
				break;
			}
			break;
		}
		break;
	case WM_DESTROY:
		if (doQuit)
			PostQuitMessage(0);
		doQuit = false;
		break;
	default:
		return(DefWindowProc(hWnd,msg,wParam,lParam));
		break;
	}
	return(0);
}