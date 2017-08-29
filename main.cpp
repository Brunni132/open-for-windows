// Open (mimique de la commande Mac).cpp : Defines the entry point for the application.

#include <Windows.h>
#include <stdio.h>
#include "Shellapi.h"
#include <string>

std::string exec(char* cmd) {
	FILE* pipe = _popen(cmd, "r");
	if (!pipe) return "ERROR";
	char buffer[128];
	std::string result = "";
	while(!feof(pipe)) {
		if(fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	}
	_pclose(pipe);
	return result;
}

static void printLastError() {
	if (GetLastError()) {
		// Retrieve the system error message for the last-error code
		LPVOID lpMsgBuf;
		DWORD dw = GetLastError(); 
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, NULL );
		MessageBox(NULL, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_OK); 
		LocalFree(lpMsgBuf);
		ExitProcess(dw); 
	}
}

int wmain(int argc, wchar_t *argv[]) {
	if (argc < 2) {
		printf("Usage: open [-t|-e] file\n\nOpens a file like from the shell.\n");
		exit(0);
	}

	bool editMode = false, textMode = false;
	wchar_t *file = 0, *verb;
	wchar_t fileBuffer[1024];

	for (int i = 0; i < argc; i++) {
		if (!wcscmp(argv[i], L"-e"))
			editMode = true;
		else if (!wcscmp(argv[i], L"-t"))
			textMode = true;
		else
			file = argv[i];
	}

	// /cygdrive/c/Users... => /c/Users
	if (wcsncmp(file, L"/", 1) == 0) {
		char buffer[1024], path[1024];
		wcstombs(path, file, 1024);
		sprintf_s(buffer, "cygpath -w \"%s\"", path);
		mbstowcs(fileBuffer, exec(buffer).c_str(), 1024);
		file = fileBuffer;
	}

	// C:/Users => C:\Users
	for (unsigned i = 0; i < wcslen(file); i++) {
		if (file[i] == '/')
			file[i] = '\\';
		else if (file[i] == '\n' || file[i] == '\r')
			file[i] = 0;
	}

	if (textMode) {
		char command[1024 + 32], path[1024];
		wcstombs(path, file, 1024);
		sprintf_s(command, "start notepad \"%s\"", path);
		system(command);
	}
	else {
		if (editMode)
			verb = L"edit";
		else
			verb = L"open";

		ShellExecute(NULL, verb, file, L"", NULL, SW_SHOW);
		if (GetLastError() && !editMode)
			ShellExecute(NULL, L"edit", file, L"", NULL, SW_SHOW);
	}
	printLastError();
	return 0;
}
