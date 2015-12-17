#pragma once

#include "ui\interface.h"

class CInterfaceScreen : public CInterfaceContainer
{
public:
	CInterfaceScreen();
	~CInterfaceScreen();

	bool onCreate();
	void onDestroy();
};