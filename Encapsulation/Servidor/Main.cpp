#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "Server.h"

void main() {
	#ifdef UNICODE 
		_setmode(_fileno(stdin), _O_WTEXT); 
		_setmode(_fileno(stdout), _O_WTEXT); 
	#endif
	//Innitiate server
	Server * s = new Server();
	s->run();
}