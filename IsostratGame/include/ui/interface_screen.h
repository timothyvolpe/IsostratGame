#pragma once

#include "ui\interface.h"

class CInterfaceScreen : public CInterfaceBase
{
public:
	CInterfaceScreen();
	~CInterfaceScreen();

	bool onCreate();
	void onDestroy();
};