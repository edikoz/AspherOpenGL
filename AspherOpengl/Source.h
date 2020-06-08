#pragma once

#define _USE_MATH_DEFINES
#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include <iostream>
#include <fstream>
#include <tchar.h>
#include <CommCtrl.h>
#include <shobjidl.h>

#define GLEW_STATIC
#include <GL\glew.h>
#include <GL\wglew.h>

#include "Shader.h"
#include "Matrix.h"
#include "Camera.h"
#include "Ray.h"
#include "Plane.h"
#include "Lens.h"
#include "Graph.h"
#include "Background.h"
#include "Helper.h"
#include "Optics.h"

#define FRAME_TIME 33

#define MY_KEY_DELETE 0x007F
#define MY_KEY_RETURN 0x000D
#define MY_KEY_BACKSPACE 0x0008
//#define MY_KEY_TAB 0x00000009
#define MY_WS_FLOAT (WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL)
#define MY_WS_INT (WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER)
#define MY_WS_BUTTON (WS_CHILD | WS_VISIBLE)
#define MY_WS_AXIS (WS_CHILD | WS_VISIBLE | ES_LEFT | SS_CENTER)

#define MAIN_BUTTONS 11
#define IS_MAIN_BUTTONS(x) LOWORD(x) / 1000 == MAIN_BUTTONS
#define MY_HMENU_BGEN (HMENU)11000
#define MY_HMENU_BMEAS (HMENU)11001
#define MY_HMENU_BSGEN (HMENU)11002
#define MY_HMENU_BCORRECT (HMENU)11003
#define MY_HMENU_BGRAPH (HMENU)11004
#define MY_HMENU_BFILE (HMENU)11005
#define MY_HMENU_BCONSOLE (HMENU)11006

#define MISC_BUTTONS 12
#define IS_MISC_BUTTONS(x) LOWORD(x) / 1000 == MISC_BUTTONS
#define MY_HMENU_BLIST (HMENU)12000
#define MY_HMENU_BRANDOM (HMENU)12001
#define MY_HMENU_BRS (HMENU)12002
#define MY_HMENU_BR8 (HMENU)12003
#define MY_HMENU_BR16 (HMENU)12004
#define MY_HMENU_BR32 (HMENU)12005
#define MY_HMENU_BIDEAL (HMENU)12006

#define GRAPH_BUTTONS 13
#define IS_GRAPH_BUTTONS(x) LOWORD(x) / 1000 == GRAPH_BUTTONS
#define MY_HMENU_BGGRID (HMENU)13000
#define MY_HMENU_BGAUTO (HMENU)13001
#define MY_HMENU_BGLINE (HMENU)13002
#define MY_HMENU_BGPOINT (HMENU)13003
#define MY_HMENU_BGTICK (HMENU)13004
#define MY_HMENU_BGSCREENSHOT (HMENU)13005
#define MY_HMENU_BGCURSOR (HMENU)13006
#define MY_HMENU_BGPROFILE (HMENU)13007

#define ROTATION_BUTTONS 14
#define MY_HMENU_BRAD (HMENU)14000
#define MY_HMENU_BDEG (HMENU)14001

#define OPTICS_BUTTONS 15
#define MY_HMENU_BMIRROR (HMENU)15000
#define MY_HMENU_BLENS (HMENU)15001

LRESULT CALLBACK SettingsWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK GraphWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK EditTextFloatProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EditTextIntProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EditTextAxisProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK StaticProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK  resize(HWND child, LPARAM lParam);
int createOpenGLWindow();
void switchHWNDfloat(HWND hWnd, float ControlText);
void switchHWNDint(HWND hWnd, int ControlText);
void stopMeasure();
void Render();
void graphRender();
//bool autoCorrect();
