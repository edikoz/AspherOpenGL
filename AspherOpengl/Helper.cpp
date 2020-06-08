#include "Helper.h"

#include <windows.h>
#include <CommCtrl.h>
#include <tchar.h>

static const int bSize = 128;
static TCHAR charBuffer[bSize] = TEXT("");

void setIntText(HWND hWnd, int i) {
	_itot_s(i, charBuffer, 10);
	SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)charBuffer);
	UpdateWindow(hWnd);
};

void setFloatText(HWND hWnd, float f) {
	_stprintf_s(charBuffer, TEXT("%.5g"), f);
	SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)charBuffer);
	UpdateWindow(hWnd);
};

int getIntText(HWND hWnd) {
	SendMessage(hWnd, WM_GETTEXT, (WPARAM)(bSize - 1), (LPARAM)charBuffer);
	return _ttoi(charBuffer);
}

float getFloatText(HWND hWnd) {
	SendMessage(hWnd, WM_GETTEXT, (WPARAM)(bSize - 1), (LPARAM)charBuffer);
	return _ttof(charBuffer);
}

TCHAR* fTos(float f) {
	_stprintf_s(charBuffer, TEXT("%.5g"), f);
	return charBuffer;
}

#undef insertTabs
void insertTabs(HINSTANCE hInst, HWND tabCtrl, HWND *childHW, TCHAR *name, ...) {
	int index = 0;
	TCITEM tie;
	memset(&tie, 0, sizeof(TCITEM));
	tie.mask = TCIF_TEXT;
	tie.iImage = -1;

	va_list argPtr;
	va_start(argPtr, name);
	TCHAR *tmp = name;
	do {
		tie.pszText = tmp;
		TabCtrl_InsertItem(tabCtrl, index++, &tie);
	} while (tmp = va_arg(argPtr, TCHAR*));
	RECT rcClient;
	GetClientRect(tabCtrl, &rcClient);
	TabCtrl_AdjustRect(tabCtrl, FALSE, &rcClient);
	HMENU tabHMENU = (HMENU)20000;
	for (int i = 0; i < index; ++i)
		childHW[i] = CreateWindow(TEXT("static"), TEXT(""), WS_CHILD, rcClient.left, rcClient.top, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, tabCtrl, tabHMENU++, hInst, NULL);
	ShowWindow(*childHW, SW_SHOW);
	va_end(argPtr);
}

#undef setProcedure
WNDPROC setProcedure(WNDPROC wndProc, HWND hwnd, ...) {
	WNDPROC retProc;
	va_list argPtr;
	va_start(argPtr, hwnd);
	HWND tmp = hwnd;
	do {
		retProc = (WNDPROC)SetWindowLongPtr(tmp, GWLP_WNDPROC, (LONG_PTR)wndProc);
	} while (tmp = va_arg(argPtr, HWND));
	va_end(argPtr);
	return retProc;
}

#undef addStrings
void addStrings(HWND hwnd, TCHAR *name, ...) {
	va_list argPtr;
	va_start(argPtr, name);
	TCHAR *tmp = name;
	do {
		SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)tmp);
	} while (tmp = va_arg(argPtr, TCHAR*));
	va_end(argPtr);
}

LRESULT CALLBACK FakeWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	return DefWindowProc(hWnd, message, wParam, lParam);
}