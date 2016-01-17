#include "base.h"
#include "ui\interface.h"
#include "ui\interface_screen.h"

CInterfaceScreen::CInterfaceScreen() {
	m_type = INTERFACE_TYPE_SCREEN;
}
CInterfaceScreen::~CInterfaceScreen() {
}

bool CInterfaceScreen::onCreate() {
	return true;
}
void CInterfaceScreen::onDestroy() {
}