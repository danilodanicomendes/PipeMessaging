#include <windows.h>
#include "WndInicial.h"
#include <process.h>
#include <errno.h>

WndInicial::WndInicial(LPCTSTR menu, HINSTANCE hInst) : 
	Window<WndInicial>(TEXT("PipeMessaging - Chat P�blico"), WS_OVERLAPPED | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT,
	615, 455, NULL, NULL, TEXT("ChatPublico"), IDB_BITMAP2, menu, hInst) {

		// Como estou a fazer a inicializa��o pela derivada e n�o pela base, o
		// evento WM_CREATE na derivada nunca era efetuado a tempo de ter o hWnd
		// bem preenchido. Por isso envio mensagem com WM_CREATE ap�s inicializa��o
		// da base e contru��o de ponteiro.
		SendMessage(hWnd, WM_CREATE, NULL, NULL);
}

DWORD WndInicial::Tindex = 0;
bool WndInicial::stopListaUsers = false;
bool WndInicial::stopMsgsChat = false;
bool WndInicial::doQuit = true;

void WndInicial::onCreate() {

	mutex = CreateMutex(NULL,FALSE,NULL);

	// LBS_STANDARD tem LBS_SORT | LBS_NOTIFY | WS_VSCROLL | WS_BORDER
	hUsersListBox = CreateWindow(TEXT("LISTBOX"), 0, WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL | WS_BORDER, 
		20, 20, 150, 355, hWnd, (HMENU) IDC_LISTA_UTILIZADORES, NULL, 0);

	hMsgEdit = CreateWindow(TEXT("EDIT"), 0, WS_CHILD | WS_VISIBLE | WS_BORDER, 
		180, 274, 235, 100, hWnd, 0, NULL, 0);

	hChatHistoricListBox = CreateWindow(TEXT("LISTBOX"), 0, WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_BORDER | WS_VSCROLL, 
		180, 20, 400, 254, hWnd, (HMENU) IDC_CHAT_HISTORY, NULL, 0);

	/*CreateWindow(TEXT("EDIT"), 0, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, 
	425, 274, 150, 100, hWnd, 0, NULL, 0);*/

	hSendButton = CreateWindow(TEXT("BUTTON"), TEXT("Enviar"), WS_BORDER | WS_CHILD | WS_VISIBLE | BS_FLAT, 
		453, 306, 100, 40, hWnd, (HMENU) IDC_ENVIAR_BUTTON, NULL, 0);

	// Foca a caixa de mensagens
	SetFocus(hMsgEdit);

	DWORD Tid = 0;
	// Lan�ar thread que atualiza lista de utilizadores
	HANDLE ThreadHwnd;

	ThreadHwnd = (HANDLE) _beginthreadex(NULL, 0, (unsigned (__stdcall*)(void*)) ThreadDistributor,
		this, 0, NULL);
	if (errno == EAGAIN || errno == EINVAL){
		MessageBox(hWnd, TEXT("Ocorreu um erro, tente mais tarde."), TEXT("Erro"), MB_ICONERROR);
		exit(0);
	}

	// Preenche dados da thread em quest�o
	ThreadListaUsers.setThreadId(Tid);
	ThreadListaUsers.setThreadHandle(ThreadHwnd);

	Sleep(200); // Para criar a segunda thread e evitar sobre
	Tindex = 1;
	Tid = 1;
	// (NULL, 0, (LPTHREAD_START_ROUTINE) ThreadDistributor, this, 0, &Tid)) == NULL)
	ThreadHwnd = (HANDLE) _beginthreadex(NULL, 0, (unsigned (__stdcall*)(void*)) ThreadDistributor,
		this, 0, NULL);
	if (errno == EAGAIN || errno == EINVAL){
		stopListaUsers = true;
		MessageBox(hWnd, TEXT("Ocorreu um erro, tente mais tarde."), TEXT("Erro"), MB_ICONERROR);
		WaitForSingleObject(ThreadListaUsers.getThreadHandle(), 10000);
		exit(0);
	}
	// Preenche dados da thread em quest�o
	ThreadMsgsChat.setThreadId(Tid);
	ThreadMsgsChat.setThreadHandle(ThreadHwnd);
}

DWORD WINAPI WndInicial::ThreadDistributor(LPVOID param) {
	if (Tindex == 0)
		return ((WndInicial*)param)->ThreadListaUsersFunction();
	else
		return ((WndInicial*)param)->ThreadMsgsChatFunction();
}

DWORD WndInicial::ThreadListaUsersFunction() {
	// Esta fun��o depois deve ser restruturada por causa que os pipes deve ser uma melhor forma de fazer isto
	// Talvez para esta primeira fazer at� nem precisava de fazer isto, pois a lista � est�tica
	int aux = 0, mutexFlag = 0;

	WaitForSingleObject(mutex,INFINITE);

	while(1) {
		// Obtem total de utilizadores
		if ((total = LerListaUtilizadoresRegistados(users)) != aux) {
			aux = total; // Desta forma s� escreve caso hajam altera��es nos users
			// Preenche Listbox com lista de utilizadores
			for (int i = 0; i < total; i++)
				SendMessage(hUsersListBox, LB_ADDSTRING, NULL, (LPARAM) users[i].login);
		}

		// Isto � para fazer uma �nica vez
		if (mutexFlag == 0) {
			mutexFlag++;
			// Liberta mutex
			ReleaseMutex(mutex);
		}

		if (stopListaUsers)
			break;
	}

	return 0;
}

inline bool operator!=(DATA aux1, DATA aux2) {
	if (aux1.ano == aux2.ano &&
		aux1.dia == aux2.dia &&
		aux1.hora == aux2.hora &&
		aux1.mes == aux2.mes &&
		aux1.minuto == aux2.minuto &&
		aux1.segundo == aux2.segundo)
		return false;
	else
		return true;
}

inline bool operator==(MENSAGEM aux1, MENSAGEM aux2) {
	if (!(aux1.instante != aux2.instante) && _tcscmp(aux1.texto, aux2.texto) == 0)
		return true;
	else
		return false;
}

DWORD WndInicial::ThreadMsgsChatFunction() {
	// Esta fun��o depois deve ser restruturada por causa que os pipes 
	// deve ser uma melhor forma de fazer isto pois j� s�o bloqueantes
	//
	// A funcao LerMensagemPublica retorna duas mensagens diferentes com a mesma hora
	// (uma do jo�o e outra do pedro) e vai retornando uma vez uma outra vez a outra
	// Por isso nesta fase vou comparar com a mem�ria toda, mas isto n�o � o melhor.
	// Para a segunda com os pipes corrijo isto.
	//
	// Update: Apesar de assim ser a forma mais segura e mais eficiente, continua a 
	// n�o conseguir apanhar a mensagem do pedro.  Para a conseguir apanhar tenho que
	// diminuir o sleep e isso j� interfere com a outra thread.
	//
	// Mesmo com o mutex n�o d�.
	// 
	// Outra situa��o:
	// Sempre que chega �s 28 msgs ele come�a a retornar mensagens vazias. E a flag
	// n�o passa para true.
	// Update: Resolvido.

	chatPublico = LerInformacaoInicial();

	MENSAGEM msg;
	TCHAR msgEnviar[TAMTEXTO];

	int linha = 0, i = 0;

	bool flag = false; // Para ver se h� alguma mensagem igual no hist�rico

	// Espera que a outra thread j� tenha corrido um bocado
	WaitForSingleObject(mutex,INFINITE);
	ReleaseMutex(mutex);

	while (1) {


		if (linha == 0) {
			for (; _tcscmp(chatPublico.publicas[linha].texto, TEXT("")) != 0; linha++) {
				_stprintf_s(msgEnviar, sizeof(msgEnviar)/sizeof(TCHAR), TEXT("(%d-%d-%d %d:%d) %s"), 
					chatPublico.publicas[linha].instante.dia, 
					chatPublico.publicas[linha].instante.mes, 
					chatPublico.publicas[linha].instante.ano, 
					chatPublico.publicas[linha].instante.hora, 
					chatPublico.publicas[linha].instante.minuto, chatPublico.publicas[linha].texto);
				SendMessage(hChatHistoricListBox, LB_ADDSTRING, NULL, (LPARAM) msgEnviar);
			}
		} else {
			WaitForSingleObject(mutex,INFINITE);
			msg = LerMensagensPublicas();
			ReleaseMutex(mutex);
			// Verifica se h� mensagem igual no hist�rico
			if (_tcscmp(msg.texto, TEXT("")) != 0) {
				for (i = 0; (_tcscmp(chatPublico.publicas[i].texto, TEXT("")) != 0); i++)
					// operator== "Global"
						if (msg == chatPublico.publicas[i])
							flag = true;
				// Se n�o houver mensagem igual
				if (!flag) {
					chatPublico.publicas[linha] = msg;
					_stprintf_s(msgEnviar, sizeof(msgEnviar)/sizeof(TCHAR), TEXT("(%d-%d-%d %d:%d) %s"), 
						chatPublico.publicas[linha].instante.dia, 
						chatPublico.publicas[linha].instante.mes, 
						chatPublico.publicas[linha].instante.ano, 
						chatPublico.publicas[linha].instante.hora, 
						chatPublico.publicas[linha].instante.minuto, chatPublico.publicas[linha].texto);
					SendMessage(hChatHistoricListBox, LB_ADDSTRING, NULL, (LPARAM) msgEnviar);
					linha++;
				} else
					_tcscpy_s(chatPublico.publicas[linha].texto, TEXT(""));
				// Reinicializar vari�veis
				i = 0;
				flag = false;

			}

		}

		if(stopMsgsChat)
			break;

	}
	return 0;
}

void WndInicial::onClick() {
	TCHAR aux[TAMTEXTO];
	GetWindowText(hMsgEdit, aux, sizeof(aux)/sizeof(TCHAR));
	SetWindowText(hMsgEdit, TEXT(""));
	SetFocus(hMsgEdit);

	WaitForSingleObject(mutex,INFINITE);
	EnviarMensagemP�blica(aux);
	ReleaseMutex(mutex);
	int i = SendMessage(hChatHistoricListBox, LB_GETCOUNT, 0, 0);
	SendDlgItemMessage(hWnd, IDC_CHAT_HISTORY, LB_SETCURSEL, i-1,0);
}

void WndInicial::onStartChatPrivado(LPARAM lParam) {
	// Verifica se o duplo click foi nua lista de utilizadores.
	// Se foi, inicia chat privado, sen�o n�o faz nada.

	if ((HWND) lParam == hUsersListBox) {
		// S� permite a conversa com o admin, isto estar� bem?
		if (IniciarConversa(users[SendDlgItemMessage(hWnd, IDC_LISTA_UTILIZADORES, LB_GETCURSEL, 0, 0)].login)) {
			//MessageBox(hWnd, TEXT("Aceite"), TEXT("SADDS"), MB_OK);
			WndPrivado w(((LPCTSTR) IDR_MENU1), gethInstance());
			w.showWindow(SW_SHOWNORMAL);
		} //else
		//MessageBox(hWnd, TEXT("Recusado, utilizador j� se encontra em conversa privada."), TEXT("PipeMessagin - Erro"), MB_OK);
	}

}

void WndInicial::onOpcao() {
	// Abre Caixa de di�logo modal.
	DialogBox(gethInstance(), (LPCWSTR) IDD_DIALOG_1, hWnd, (DLGPROC) DialogOpcao);
}

BOOL CALLBACK WndInicial::DialogOpcao(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	// Ainda n�o est� implementada, como n�o h� fun��es para isto na DLL.
	switch(msg) {
	case WM_INITDIALOG:
		SetWindowText(hWnd, TEXT("PipeMessaging - Op��es"));
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			return 0;
		case IDCANCEL:
			return 0;
		}
		//break; // est� comentado propositadamente, para sair. Pois n�o est� implementado
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		return 1;
	default:
		return 0;
		break;
	}
	return 0;
}


void WndInicial::onGerir() {
	DialogBox(gethInstance(), (LPCWSTR) IDD_DIALOG1, hWnd, (DLGPROC) DialogGerir);
}

BOOL CALLBACK WndInicial::DialogGerir(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	// Ainda n�o est� implementada, como n�o h� fun��es para isto na DLL.
	switch(msg) {
	case WM_INITDIALOG:
		SetWindowText(hWnd, TEXT("PipeMessaging - Gerir Utilizadores"));
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BUTTON2:
			Desligar();
			break;
		case IDOK:
			return 0;
		case IDC_BUTTON1:
			return 0;
		}
		//break; // est� comentado propositadamente, para sair. Pois n�o est� implementado
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		return 1;
	default:
		return 0;
		break;
	}
	return 0;
}


void WndInicial::onHelp() {
	MessageBox(hWnd, TEXT("\t\t\tAjuda\n\nPara enviar mensagem:\n\t1 - Escreva a mensagem na janela em baixo\n\t2 - Clique no bot�o \"Enviar\"\nCaso queira conversar em privado, basta clicar duas vezes no utilizador em quest�o."),
		TEXT("PipeMessaging - Ajuda"), MB_OK);

}

void WndInicial::onSobre() {
	MessageBox(hWnd, TEXT("\t\t\tSobre\n\nPipeMessagin Vers�o 1.0.0\n\nDesenvolvido no �mbito da cadeira Sistemas Operativos 2\nPor: Danilo Mendes 21180477\nDEIS-ISEC"),
		TEXT("PipeMessaging - Sobre"), MB_OK);
}

void WndInicial::onLogout() {
	// Suponho que o logout seja para terminar a aplica��o.
	// N�o sei se esta fun��o faz alguma coisa neste momento.
	Sair();
	// Acciona flags e espera threads terminarem
	stopListaUsers = true;
	WaitForSingleObject(ThreadListaUsers.getThreadHandle(), INFINITE);
	stopMsgsChat = true;
	WaitForSingleObject(ThreadMsgsChat.getThreadHandle(), INFINITE);
	// Envia pedido de WM_CLOSE
	doQuit = true;
	SendMessage(hWnd, WM_CLOSE, NULL, NULL);
}

LRESULT CALLBACK WndInicial::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	WndInicial * WindowPtr = (WndInicial *) GetWindowLongPtr(hWnd, 0);
	switch (msg) {
	case WM_CREATE:
		WindowPtr->onCreate();
		break;
	case WM_NOTIFY:
		break;
	case WM_COMMAND:
		switch(HIWORD(wParam)) {
		case LBN_DBLCLK:
			WindowPtr->onStartChatPrivado(lParam);
			break;
		default:
			switch(LOWORD(wParam)) {
			case IDC_ENVIAR_BUTTON:
				WindowPtr->onClick();
				break;
				// Eventos do menu
			case ID_ADMIN_OPC: // Separei caso fosse diferente
				WindowPtr->onOpcao();
				break;
			case ID_UTILIZADOR_OPC: // Separei caso fosse diferente
				WindowPtr->onOpcao();
				break;
			case ID_ADMIN_GERIR:
				WindowPtr->onGerir();
				break;
			case ID_AJUDA_HELP:
				WindowPtr->onHelp();
				break;
			case ID_AJUDA_SOBRE:
				WindowPtr->onSobre();
			case ID_ADMIN_LOGOUT:
				WindowPtr->onLogout();
				break;
			case ID_UTILIZADOR_LOGOUT:
				WindowPtr->onLogout();
				break;
			}
		}
		break;
	case WM_DESTROY:
		//if (doQuit)
		PostQuitMessage(0);
		break;
	default:
		return(DefWindowProc(hWnd,msg,wParam,lParam));
		break;
	}
	return(0);
}