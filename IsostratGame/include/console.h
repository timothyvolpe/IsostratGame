#pragma once

#include "def.h"
#include "platform.h"

enum : unsigned char
{
	CONSOLE_COLOR_DEFAULT,
	CONSOLE_COLOR_WARNING,
	CONSOLE_COLOR_ERROR
};

// Variadic console output macros
#if defined( _WINDOWS ) && defined( EXTERNAL_DEBUG_CONSOLE )
#define PrintInfo(msg, ...) CGame::instance().getConsole()->printColor(L"", CONSOLE_COLOR_DEFAULT); wprintf(msg, ##__VA_ARGS__ )
#define PrintWarn(msg, ...) CGame::instance().getConsole()->printColor(L"WARN: ", CONSOLE_COLOR_WARNING); wprintf(msg, ##__VA_ARGS__ )
#define PrintError(msg, ...) CGame::instance().getConsole()->printColor(L"ERROR: ", CONSOLE_COLOR_ERROR); wprintf(msg, ##__VA_ARGS__ )
#else
#define PrintInfo(msg, ...) 
#define PrintWarn(msg, ...)
#define PrintError(msg, ...)
#endif

class CConsole
{
private:
#ifdef _WINDOWS
	FILE *m_hCout, *m_hCerr;
#endif
public:
	CConsole();
	~CConsole();

	bool initialize();
	void destroy();

	void setColor( unsigned char color );

	void printColor( std::wstring message, unsigned char color );
};