/*
	File: main.cpp
	Description: NEDitor main entry point
*/
#include "ned.h"

#ifdef PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

static void configureConsoleUtf8()
{
#ifdef PLATFORM_WINDOWS
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
	if (out != INVALID_HANDLE_VALUE)
	{
		DWORD mode = 0;
		if (GetConsoleMode(out, &mode))
			SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
	}
#endif
}

int main()
{
	configureConsoleUtf8();
	Ned ned;
	if (!ned.initialize())
	{
		return -1;
	}
	std::cout << "🙈Starting NED...🙈" << '\n';
	ned.run();
	return 0;
}
