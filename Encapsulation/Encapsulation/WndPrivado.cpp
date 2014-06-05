#include "WndPrivado.h"
#include <process.h> // _beginthreadex()
#include <errno.h> // erro da thread
//#include "WndInicial.h"


/*
	Ok, continuo com erro. Isto em relação ao encapsulamento está tudo correcto, tinha problemas era com o stprintf_s
	em que não estava a por o argumento do tamanho que serve para caso exceda não tentar escrever mas sim cortar a 
	string. Depois erro por causa dos tamanhos em UNICODE têm que ser medidos sizeof(vv)/sizeof(TCHAR) pois unicode 
	tem tamanho 2 e não 1.

	Agora o problema que está a ocorrer será alguma função que estou a trocar, nadaquanto ao encapsulamento mas sim
	na thread de leitura/escrita no histórico do chat.
*/

WndPrivado::WndPrivado(LPCTSTR menu, HINSTANCE hInst) : 
	Window<WndPrivado>(TEXT("PipeMessaging - Chat Privado"), WS_OVERLAPPED | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT,
	615, 455, NULL, NULL, TEXT("ChatPrivado"), IDB_BITMAP1, menu, hInst) {
	// Como estou a fazer a inicialização pela derivada e não pela base, o
		// evento WM_CREATE na derivada nunca era efetuado a tempo de ter o hWnd
		// bem preenchido. Por isso envio mensagem com WM_CREATE após inicialização
		// da base e contrução de ponteiro.
		SendMessage(hWnd, WM_CREATE, NULL, NULL);
}

bool WndPrivado::stopMsgsChat = false;
bool WndPrivado::doQuit = false;

void WndPrivado::onCreate() {
	// LBS_STANDARD tem LBS_SORT | LBS_NOTIFY | WS_VSCROLL | WS_BORDER
	hUsersListBox2 = CreateWindow(TEXT("LISTBOX"), 0, WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL | WS_BORDER, 
		20, 20, 150, 355, hWnd, (HMENU) IDC_LISTA_UTILIZADORES2, NULL, 0);

	hMsgEdit2 = CreateWindow(TEXT("EDIT"), 0, WS_CHILD | WS_VISIBLE | WS_BORDER, 
		180, 274, 235, 100, hWnd, 0, NULL, 0);

	hChatHistoricListBox2 = CreateWindow(TEXT("LISTBOX"), 0, WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_BORDER | WS_VSCROLL, 
		180, 20, 400, 254, hWnd, (HMENU) IDC_CHAT_HISTORY2, NULL, 0);

	hSendButton2 = CreateWindow(TEXT("BUTTON"), TEXT("Enviar"), WS_BORDER | WS_CHILD | WS_VISIBLE | BS_FLAT, 
		453, 306, 100, 40, hWnd, (HMENU) IDC_ENVIAR_BUTTON2, NULL, 0);

	// Foca a caixa de mensagens
	SetFocus(hMsgEdit2);

	DWORD Tid = 0;
	// Lançar thread que atualiza mensagens entre utilizadores em privado
	HANDLE ThreadHwnd;
	ThreadHwnd = (HANDLE) _beginthreadex(NULL, 0, (unsigned (__stdcall*)(void*)) ThreadDistributor,
		this, 0, (unsigned int*) &Tid);
	if (errno == EAGAIN || errno == EINVAL){
		MessageBox(hWnd, TEXT("Ocorreu um erro, tente mais tarde."), TEXT("Erro"), MB_ICONERROR);
		exit(0);
	}
	// Preenche dados da thread em questão
	ThreadMsgsChat.setThreadId(Tid);
	ThreadMsgsChat.setThreadHandle(ThreadHwnd);
}

DWORD WINAPI WndPrivado::ThreadDistributor(LPVOID param) {
	return ((WndPrivado*)param)->ThreadMsgsChatFunction();
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

DWORD WndPrivado::ThreadMsgsChatFunction() {
	// Esta função depois deve ser restruturada por causa que os pipes 
	// deve ser uma melhor forma de fazer isto pois já são bloqueantes
	//
	// continuo a usar a variavel chatPrivado do tipo CHAT, para servir
	// apenas de registo em memória, a verificação é feita da mesma forma
	// como está descrita na janela pública.

	chatPrivado = LerInformacaoInicial();

	MENSAGEM msg;
	TCHAR msgEnviar[TAMTEXTO];

	int linha = 0, i = 0;

	bool flag = false; // Para ver se há alguma mensagem igual no histórico

	// Espera que a outra thread já tenha corrido um bocado
	WaitForSingleObject(mutex,INFINITE);
	ReleaseMutex(mutex);

	while (1) {
		if (linha == 0) {
			for (; _tcscmp(chatPrivado.privadas[0][linha].texto, TEXT("")) != 0; linha++) {
				_stprintf_s(msgEnviar, sizeof(msgEnviar)/sizeof(TCHAR), TEXT("(%d-%d-%d %d:%d) %s"), 
					chatPrivado.privadas[0][linha].instante.dia, 
					chatPrivado.privadas[0][linha].instante.mes, 
					chatPrivado.privadas[0][linha].instante.ano, 
					chatPrivado.privadas[0][linha].instante.hora, 
					chatPrivado.privadas[0][linha].instante.minuto, chatPrivado.privadas[0][linha].texto);
				SendMessage(hChatHistoricListBox2, LB_ADDSTRING, NULL, (LPARAM) msgEnviar);
			}
		} else {
			WaitForSingleObject(mutex,INFINITE);
			msg = LerMensagensPrivadas();
			ReleaseMutex(mutex);
				// Verifica se há mensagem igual no histórico
			if (_tcscmp(msg.texto, TEXT("")) != 0) {
				for (i = 0; (_tcscmp(chatPrivado.privadas[0][i].texto, TEXT("")) != 0); i++)
					// operator== "Global"
					if (msg == chatPrivado.privadas[0][i])
						flag = true;
					// Se não houver mensagem igual
					if (!flag) {
						chatPrivado.privadas[0][linha] = msg;
						_stprintf_s(msgEnviar, sizeof(msgEnviar)/sizeof(TCHAR), TEXT("(%d-%d-%d %d:%d) %s"), 
							chatPrivado.privadas[0][linha].instante.dia, 
							chatPrivado.privadas[0][linha].instante.mes, 
							chatPrivado.privadas[0][linha].instante.ano, 
							chatPrivado.privadas[0][linha].instante.hora, 
							chatPrivado.privadas[0][linha].instante.minuto, chatPrivado.privadas[0][linha].texto);
						SendMessage(hChatHistoricListBox2, LB_ADDSTRING, NULL, (LPARAM) msgEnviar);
						linha++;
					} else
						_tcscpy_s(chatPrivado.privadas[0][linha].texto, TEXT(""));
					// Reinicializar variáveis
					i = 0;
					flag = false;

				}
				
		}

		if(stopMsgsChat)
			break;

	}
	return 0;
}

void WndPrivado::onClick() {
	TCHAR aux[TAMTEXTO];
	GetWindowText(hMsgEdit2, aux, sizeof(aux)/sizeof(TCHAR));
	SetWindowText(hMsgEdit2, TEXT(""));
	SetFocus(hMsgEdit2);

	EnviarMensagemPrivada(aux);

	int i = SendMessage(hChatHistoricListBox2, LB_GETCOUNT, 0, 0);
	SendDlgItemMessage(hWnd, IDC_CHAT_HISTORY2, LB_SETCURSEL, i-1,0);
}

void WndPrivado::onOpcao() {
	// Abre Caixa de diálogo modal.
	DialogBox(gethInstance(), (LPCWSTR) IDD_DIALOG_1, hWnd, (DLGPROC) DialogOpcao);
}

BOOL CALLBACK WndPrivado::DialogOpcao(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	// Ainda não está implementada, como não há funções para isto na DLL.
	switch(msg) {
	case WM_INITDIALOG:
		SetWindowText(hWnd, TEXT("PipeMessaging - Opções"));
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			return 0;
		case IDCANCEL:
			return 0;
		}
		//break; // está comentado propositadamente, para sair. Pois não está implementado
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		return 1;
	default:
		return 0;
		break;
	}
	return 0;
}


void WndPrivado::onGerir() {
	DialogBox(gethInstance(), (LPCWSTR) IDD_DIALOG1, hWnd, (DLGPROC) DialogGerir);
}

BOOL CALLBACK WndPrivado::DialogGerir(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	// Ainda não está implementada, como não há funções para isto na DLL.
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
		//break; // está comentado propositadamente, para sair. Pois não está implementado
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		return 1;
	default:
		return 0;
		break;
	}
	return 0;
}


void WndPrivado::onHelp() {
	MessageBox(hWnd, TEXT("\t\t\tAjuda\n\nPara enviar mensagem:\n\t1 - Escreva a mensagem na janela em baixo\n\t2 - Clique no botão \"Enviar\"\nCaso queira conversar em privado, basta clicar duas vezes no utilizador em questão."),
		TEXT("PipeMessaging - Ajuda"), MB_OK);

}

void WndPrivado::onSobre() {
	MessageBox(hWnd, TEXT("\t\t\tSobre\n\nPipeMessagin Versão 1.0.0\n\nDesenvolvido no âmbito da cadeira Sistemas Operativos 2\nPor: Danilo Mendes 21180477\nDEIS-ISEC"),
		TEXT("PipeMessaging - Sobre"), MB_OK);
}

void WndPrivado::onLogout() {
	// Suponho que o logout seja para terminar a aplicação.
	// Não sei se esta função faz alguma coisa neste momento.
	Sair();
	// Acciona flags e espera threads terminarem
	stopMsgsChat = true;
	WaitForSingleObject(ThreadMsgsChat.getThreadHandle(), INFINITE);
	// Envia pedido de WM_CLOSE
	doQuit = true;
	SendMessage(hWnd, WM_CLOSE, NULL, NULL);
}

LRESULT CALLBACK WndPrivado::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	WndPrivado * WindowPtr = (WndPrivado *) GetWindowLongPtr(hWnd, 0);
	switch (msg) {
	case WM_CREATE:
		WindowPtr->onCreate();
		break;
	case WM_NOTIFY:
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_ENVIAR_BUTTON2:
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
		break;
	case WM_DESTROY:
		DesligarConversa(); // Desliga conversa ao fechar janela
		if (doQuit)
			PostQuitMessage(0);
		break;
	default:
		return(DefWindowProc(hWnd,msg,wParam,lParam));
		break;
	}
	return(0);
}