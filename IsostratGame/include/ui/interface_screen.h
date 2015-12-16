#pragma once

#include "ui\interface.h"

class CInterfaceScreen : public CInterfaceRenderable
{
public:
	CInterfaceScreen();
	~CInterfaceScreen();

	bool onCreate();
	void onDestroy();
};