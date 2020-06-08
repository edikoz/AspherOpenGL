#pragma once

#include <windows.h>
#include <stdint.h>
#include <iostream>
#include <tchar.h>

#define GET_RAND(x,y) ((float)rand() / RAND_MAX * (x) + (y))

void setIntText(HWND hWnd, int i);
void setFloatText(HWND hWnd, float f);
int getIntText(HWND hWnd);
float getFloatText(HWND hWnd);
TCHAR* fTos(float f);

static inline uint32_t rotl32(uint32_t n, unsigned int c) {
	const unsigned int mask = (CHAR_BIT*sizeof(n) - 1);
	c &= mask;
	return (n << c) | (n >> ((-c)&mask));
}

static inline uint32_t rotr32(uint32_t n, unsigned int c) {
	const unsigned int mask = (CHAR_BIT*sizeof(n) - 1);
	c &= mask;
	return (n >> c) | (n << ((-c)&mask));
}

static inline void doMessage(MSG msg) {
	if (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

static inline void initClass(HINSTANCE hinst, WNDPROC wndproc, TCHAR *className, bool ownDC) {
	WNDCLASSEX wcex;
	memset(&wcex, 0, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	if (ownDC) wcex.style = CS_OWNDC;
	wcex.lpfnWndProc = wndproc;
	wcex.hInstance = hinst;
	wcex.hIcon = LoadIcon(hinst, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszClassName = className;
	wcex.hIconSm = LoadIcon(hinst, MAKEINTRESOURCE(IDI_APPLICATION));
	if (!RegisterClassEx(&wcex)) std::cout << "RegisterClass FAIL" << std::endl;
}

static inline void toggleConsole() {
	static bool consHide = true;
	HWND consoleHWND;
	if (consHide = !consHide) {
		consoleHWND = GetConsoleWindow();
		ShowWindow(consoleHWND, SW_SHOW);
	}
	else {
		consoleHWND = GetConsoleWindow();
		ShowWindow(consoleHWND, SW_HIDE);
	}
}

void insertTabs(HINSTANCE hInst, HWND tabCtrl, HWND *childHW, TCHAR *name, ...);
WNDPROC setProcedure(WNDPROC wndProc, HWND hwnd, ...);
void addStrings(HWND hwnd, TCHAR *name, ...);
#define insertTabs(...) insertTabs(__VA_ARGS__, NULL)
#define setProcedure(...) setProcedure(__VA_ARGS__, NULL)
#define addStrings(...) addStrings(__VA_ARGS__, NULL)

LRESULT CALLBACK FakeWndProc(HWND, UINT, WPARAM, LPARAM);