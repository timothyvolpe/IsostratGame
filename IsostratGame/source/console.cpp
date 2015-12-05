#include <stdio.h>
#include <iostream>
#ifdef _WINDOWS
#include <io.h>
#include <fcntl.h>
#endif

#include "console.h"

#ifdef _WINDOWS
CConsole::CConsole() {
	m_hCout = NULL;
	m_hCerr = NULL;
}
CConsole::~CConsole() {
}

bool CConsole::initialize()
{
#ifdef EXTERNAL_DEBUG_CONSOLE
	// Allocate the console
	AllocConsole();
	// Redirect cout to the console
	freopen_s( &m_hCout, "CONOUT$", "w", stdout );
	freopen_s( &m_hCerr, "CONOUT$", "w", stderr );
#endif

	return true;
}
void CConsole::destroy()
{
#ifdef EXTERNAL_DEBUG_CONSOLE
	// Destroy the out handle
	if( m_hCout ) {
		fclose( m_hCout );
		m_hCout = NULL;
	}
	if( m_hCerr ) {
		fclose( m_hCerr );
		m_hCerr = NULL;
	}

	FreeConsole();
#endif
}

void CConsole::setColor( unsigned char color )
{
#ifdef EXTERNAL_DEBUG_CONSOLE
	switch( color )
	{
	case CONSOLE_COLOR_DEFAULT:
		SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), 0x0F );
		break;
	case CONSOLE_COLOR_WARNING:
		SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN );
		break;
	case CONSOLE_COLOR_ERROR:
		SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), FOREGROUND_INTENSITY | FOREGROUND_RED );
		break;
	default:
		break;
	}
#endif EXTERNAL_DEBUG_CONSOLE
}

#endif

void CConsole::printColor( std::wstring message, unsigned char color )
{
	this->setColor( color );
#ifdef EXTERNAL_DEBUG_CONSOLE
	std::wcout << message.c_str();
#endif
}