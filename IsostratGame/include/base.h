#pragma once

#include "game.h"
#include "console.h"

#define SAFE_DELETE(x) if(x) { delete x; x = 0; }
#define SAFE_DELETE_A(x) if(x) { delete[] x; x = 0; }
#define DESTROY_DELETE(x) if(x) { x->destroy(); delete x; x = 0; }

class CConsole;
class CGraphics;