#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "comctl32.lib")

#include "Source.h"
#include <map>

using namespace std;

const int bSize = 128;
TCHAR tmpBuffer[bSize] = TEXT("");
TCHAR screenBuffer[bSize] = TEXT("");

HINSTANCE hInst;
HDC hdc, ghdc;
HGLRC hglrc;
WNDPROC OriginalEditTextProc, OriginalStaticProc;
HWND mainW, settingsHWND, graphHWND;
HWND hwETlx, hwETly, hwETlz, hwETlrx, hwETlry, hwETlrz, hwETlax1, hwETlax2, hwETlax3,
hwCBtype, hwETfa, hwETsa, hwETnx, hwETny, hwETps, hwETpi, hwETrw, hwETmx, hwETmd, hwETma, hwETmr, hwETml, hwETmdx, hwETmdy, hwETrn, hwETrd,
hwETstart, hwETstep, hwETend, hwETcx, hwETcy, hwETwx, hwETwy, hwCBmeas, hwETincr, hwBgraph, hwETdif,
hwETsx, hwETsy, hwETswmm, hwETshmm, hwETswpix, hwETshpix,
hwETsmile, hwETsmileSft, hwETsmilePlt, hwETerx, hwETery, hwETerz,
hwBGgrid, hwBGauto, hwBGline, hwBGpoint, hwBGticks, hwBGcursors, hwBGprofile;

ULONGLONG gtt64All;
string dwText;
double cx, cy, wx, wy, cI;
float incr = 0.01f;
int graphDataSize;
bool measure = false, correct = false;
float measStart = 0.0f, measStep = 0.1f, measEnd = 1.0f;
int measType, measCurrent, phase;
float graphMx, graphMy;

bool appRun = false;
bool graph = false, file = false;
bool graphAuto = true;
bool drawProfile = true;
int clientWindowHeight, clientWindowWidth;
bool needRender = false, needGraphRender = false, needRecalc = true;

bool meas2D = false;
int measFirst = 0, measSecond = 0;
float measFstart, measFstep, measFend;
float measSstart, measSstep, measSend;

bool FWatMax = false, WidthAsFWHM = false, plotOnlyIntensity = false;

map<string, int> prefs = { { "SHADER_REPLASES", 1 }, { "FAC_LENS", 2 }, { "MEASURE2D", 3 }, { "BTS_LENS", 4 }, { "FAC_COEF", 5 }, { "RAYS", 6 } };
map<char, int> coords = { { 'x', 1 }, { 'y', 2 }, { 'z', 3 }, { 'X', 4 }, { 'Y', 5 }, { 'Z', 6 } };

int WINAPI WinMain(HINSTANCE chInst, HINSTANCE prev, LPSTR cmd, int mode) {
	/*for (int i = 0; cmd[i] != '\0'; ++i) {
		if (cmd[i] == 'c') {}
	}*/
	AllocConsole();
	toggleConsole();
	FILE* stream;
	_tfreopen_s(&stream, L"CONOUT$", L"w", stdout);
	DeleteMenu(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_BYCOMMAND);
	hInst = chInst;

	float lx, ld, lh, lw, ln;
	float br, bp, bw, bg;
	float fc[8];
	bool FWe2 = false, WidthAtMax = false;
	float FWlevel = 0.5;
	ifstream prefsFile("Res/prefs.txt", ios::in | ios_base::beg);
	if (prefsFile.is_open())
	{
		string menu;
		while (getline(prefsFile, menu)) {
			switch (prefs[menu]) {
			case 1:
			{
				string sFname, sRepl;
				do {
					prefsFile >> sFname >> sRepl;
					Shader::replaces.insert(pair<string, string>(sFname, sRepl));
				} while (sFname.compare("end")); //FIXIT: may cycle if not founded "end"
			}
			break;
			case 2: prefsFile >> lx >> ld >> lh >> lw >> ln; break;
			case 3:
				char mf, ms;
				prefsFile >> meas2D >> mf >> ms >> measFstart >> measFstep >> measFend >> measSstart >> measSstep >> measSend;
				if (meas2D) {
					measFirst = coords[mf];
					measSecond = coords[ms];
				}
				break;
			case 4: prefsFile >> br >> bp >> bw >> bg; break;
			case 5:
				for (int i = 0; i < 8; ++i)
					prefsFile >> fc[i];
				break;
			case 6: prefsFile >> FWe2 >> WidthAtMax >> FWatMax >> WidthAsFWHM >> FWlevel >> plotOnlyIntensity; break;
			}
		}
		prefsFile.close();
	}

	std::cout << "Creating window" << std::endl;
	if (!createOpenGLWindow()) return 0;

	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	std::cout << "/-----LIMITS-----/" << std::endl;
	std::cout << "Graph max points: " << "10000" << std::endl;
	std::cout << "Max rays: " << "500 x 500" << std::endl;
	std::cout << "Max sensor size: " << "2000 x 2000" << std::endl;

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	Shader::Init();
	Background::generate();
	Camera::move(-8.0f, 3.0f, 7.1f);
	Camera::rotateTo(-0.154f, 1.85f);
	Ray::generate(0.2, 25.0, 8.0, 100, 100, Ray::MODE::Gauss, 256, 256, FWe2, WidthAtMax, WidthAsFWHM, FWlevel);
	Plane::generate(10.0f, 10.0f);
	Lens::generate(lx, 0.0f, 0.0f, ld, lh, lw, ln, fc, br, bp, bw, bg);
	Optics::generate(400.0, 400.0, 80.0, 50.8, 70.0 * M_PI / 180.0);
	Plane::moveToX(538.72);
	Plane::moveToY(-51.423);
	Graph::generate();
	appRun = true;
	wglMakeCurrent(0, 0);

	ShowWindow(mainW, mode);
	UpdateWindow(settingsHWND);

	TCHAR thisPath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, thisPath);
	_tprintf_s(thisPath); std::cout << std::endl;

	ULONGLONG lastRenderTime = GetTickCount64();
	MSG msg = {};
	while (appRun) {
		while (correct) {
			wglMakeCurrent(hdc, hglrc);
			//correct = autoCorrect();
			wglMakeCurrent(0, 0);
			if (graphAuto) graphRender();

			if (PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE)) doMessage(msg);
		}

		while (measure) {
			if (false || meas2D) {
				wglMakeCurrent(hdc, hglrc);
				int xCount = 0, yCount = 0;
				for (float ddy = measFstart; ddy < measFend; ddy += measFstep) {
					switch (measFirst) {
					case 1: Lens::moveToX(ddy); setFloatText(hwETlx, Lens::getX()); break;
					case 2: Lens::moveToY(ddy); setFloatText(hwETly, Lens::getY()); break;
					case 3: Lens::moveToZ(ddy); setFloatText(hwETlz, Lens::getZ()); break;
					case 4: Lens::rotateTo(ddy, 0); setFloatText(hwETlrx, Lens::getRots(0)); break;
					case 5: Lens::rotateTo(ddy, 1); setFloatText(hwETlry, Lens::getRots(1)); break;
					case 6: Lens::rotateTo(ddy, 2); setFloatText(hwETlrz, Lens::getRots(2)); break;
					}
					++yCount;
					xCount = 0;
					for (float ddx = measSstart; ddx < measSend; ddx += measSstep) {
						switch (measSecond) {
						case 1: Lens::moveToX(ddx); setFloatText(hwETlx, Lens::getX()); break;
						case 2: Lens::moveToY(ddx); setFloatText(hwETly, Lens::getY()); break;
						case 3: Lens::moveToZ(ddx); setFloatText(hwETlz, Lens::getZ()); break;
						case 4: Lens::rotateTo(ddx, 0); setFloatText(hwETlrx, Lens::getRots(0)); break;
						case 5: Lens::rotateTo(ddx, 1); setFloatText(hwETlry, Lens::getRots(1)); break;
						case 6: Lens::rotateTo(ddx, 2); setFloatText(hwETlrz, Lens::getRots(2)); break;
						}
						++xCount;
						Render();
						if (isnan(cx)) cx = 0;
						if (isnan(cy)) cy = 0;
						if (isnan(wx)) wx = 0;
						if (isnan(wy)) wy = 0;
						if (isnan(cI)) cI = 0;
						dwText += to_string(ddx) + ' ' + to_string(ddy) + ' ' + to_string(cx) + ' ' + to_string(cy) + ' ' + to_string(wx) + ' ' + to_string(wy) + ' ' + to_string(cI) + '\n';
					}
				}
				dwText += to_string(xCount) + ' ' + to_string(yCount);
				wglMakeCurrent(0, 0);
				stopMeasure();
			}
			else {
				if (measCurrent < graphDataSize) {

					wglMakeCurrent(hdc, hglrc);
					float currentValue = (measStart < measEnd) ? measStart + measStep * measCurrent : measStart - measStep * measCurrent;
					switch (measType) {
					case 0: Lens::moveToX(currentValue); setFloatText(hwETlx, Lens::getX()); break;
					case 1: Lens::moveToY(currentValue); setFloatText(hwETly, Lens::getY()); break;
					case 2: Lens::moveToZ(currentValue); setFloatText(hwETlz, Lens::getZ()); break;
					case 3: Lens::rotateTo(currentValue, Axis::X); setFloatText(hwETlrx, Lens::getRots(0)); break;
					case 4: Lens::rotateTo(currentValue, Axis::Y); setFloatText(hwETlry, Lens::getRots(1)); break;
					case 5: Lens::rotateTo(currentValue, Axis::Z); setFloatText(hwETlrz, Lens::getRots(2)); break;
					case 6: Plane::moveToX(currentValue); setFloatText(hwETsx, Plane::getX()); break;
					}
					needRecalc = true;

					Render();
					if (plotOnlyIntensity)
						Graph::addPoint(currentValue, cI, cI, cI, cI);
					else
						Graph::addPoint(currentValue, cx, cy, wx, wy);
					wglMakeCurrent(0, 0);

					if (graphAuto) graphRender();

					dwText += to_string(currentValue) + ' ' + to_string(cx) + ' ' + to_string(cy) + ' ' + to_string(wx) + ' ' + to_string(wy) + ' ' + to_string(cI) + '\n';
					measCurrent++;
				}
				else stopMeasure();
			}
			if (PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE)) doMessage(msg);
		}

		doMessage(msg);

		if (needRender) {
			if (GetTickCount64() - lastRenderTime > FRAME_TIME&& appRun) {
				wglMakeCurrent(hdc, hglrc);
				Render();
				wglMakeCurrent(0, 0);
				lastRenderTime = GetTickCount64();
				needRender = false;
			}
		}
		if (needGraphRender) {
			if (appRun) {
				graphRender();
				needGraphRender = false;
			}
		}

	}
	return 0;
}

void Render() {
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, clientWindowHeight / 2, clientWindowWidth, clientWindowHeight / 2);
	glDisable(GL_DEPTH_TEST);
	Background::draw();
	glEnable(GL_DEPTH_TEST);
	Camera::setCameraToMain();
	Ray::drawOnScene();
	Optics::draw();

	Camera::setCameraToSensor();
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	Ray::drawOnTexture();

	glViewport(0, clientWindowHeight / 2.0f, clientWindowWidth, clientWindowHeight / 2.0f);
	Camera::setCameraToMain();
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	Plane::draw();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	Lens::draw();
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);

	glDisable(GL_DEPTH_TEST);
	Background::drawGUI();
	glEnable(GL_DEPTH_TEST);

	glViewport(0, 0, clientWindowWidth / 2, clientWindowHeight / 2);
	Camera::setCameraToSensor();
	Plane::draw();
	glDisable(GL_DEPTH_TEST);

	Ray::getParams(&cI, &cx, &cy, &wx, &wy);
	if (needRecalc || !FWatMax) {
		needRecalc = false;
		if (FWatMax || !WidthAsFWHM)
			Ray::getProfile(nullptr, nullptr);
		else
			Ray::getProfile(&wx, &wy);
	}

	if (drawProfile)
		Ray::drawProfile();

	setFloatText(hwETcx, cx);
	setFloatText(hwETcy, cy);
	setFloatText(hwETwx, wx);
	setFloatText(hwETwy, wy);

	Camera::setCameraToMain();
	glBindVertexArray(0);
	glUseProgram(0);
	SwapBuffers(hdc);
}

void graphRender() {
	wglMakeCurrent(ghdc, hglrc);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, Graph::w, Graph::h);

	if (!measure) Graph::drawTicks();
	Graph::draw();
	if (!measure) if (Graph::cursors) Graph::drawCursors(graphMx, graphMy);

	SwapBuffers(ghdc);
	wglMakeCurrent(NULL, NULL);
}

double interp(double* ary, int size, double x) {
	int idX = (int)x;
	if (idX < 0) return ary[0];
	if (idX >= size - 1) return ary[size - 1];
	return ary[idX] + (ary[idX + 1] - ary[idX]) / (idX + 1 - idX) * (x - idX);
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static float prevMx = 0, prevMy = 0;
	static bool mouseON = false;

	switch (message)
	{
	case WM_PAINT: {
		PAINTSTRUCT ps;
		BeginPaint(mainW, &ps);
		if (appRun) {
			wglMakeCurrent(hdc, hglrc);
			Render();
			wglMakeCurrent(0, 0);
		}
		EndPaint(mainW, &ps);
	}
				 break;
	case WM_LBUTTONDOWN: SetFocus(mainW); break;
	case WM_MOUSEWHEEL: {
		Camera::increaseScale(-GET_WHEEL_DELTA_WPARAM(wParam));
		needRender = true;
	}
					  break;
	case WM_CHAR:
		switch (wParam) {
		case 'q': mouseON = !mouseON; break;
		case ' ': if (measure) stopMeasure(); break;
		case 0x1B: if (measure) stopMeasure(); PostMessage(hWnd, WM_CLOSE, 0, 0); break;
		case 'o': Camera::changeProjection(); break;

		case 'w': Camera::moveDirection(0.1f, Camera::Direction::forward); break;
		case 's': Camera::moveDirection(0.1f, Camera::Direction::back); break;
		case 'a': Camera::moveDirection(0.1f, Camera::Direction::left); break;
		case 'd': Camera::moveDirection(0.1f, Camera::Direction::right); break;
		case 'r': Camera::move(0, 0.1f, 0); break;
		case 'f': Camera::move(0, -0.1f, 0); break;

		case 'W': Camera::moveDirection(1.0f, Camera::Direction::forward); break;
		case 'S': Camera::moveDirection(1.0f, Camera::Direction::back); break;
		case 'A': Camera::moveDirection(1.0f, Camera::Direction::left); break;
		case 'D': Camera::moveDirection(1.0f, Camera::Direction::right); break;
		case 'R': Camera::move(0, 1.0f, 0); break;
		case 'F': Camera::move(0, -1.0f, 0); break;

		case 'z': Camera::ortho = true; Camera::setScale(140.0f); Camera::moveTo((Lens::getX() + Plane::getX()) / 2.0, (Lens::getY() + Plane::getY()) / 2.0, 100.0f); Camera::rotateTo(0, -M_PI);	break;
		case 'x': Camera::moveTo(Lens::getX() + 2.0f, Lens::getY() + 2.0f, Lens::getZ() + 15.0f); Camera::rotateTo(-M_PI / 10.0f, -M_PI);	break;
		case 'c': Camera::moveTo(getFloatText(hwETmx), 2.0f, 100.0f); Camera::rotateTo(-M_PI / 10.0f, -M_PI); break;
		case 'v': Camera::moveTo(Plane::getX() + 2.0f, Plane::getY() + 2.0f, 100.0f); Camera::rotateTo(-M_PI / 10.0f, -M_PI); break;

		case 'P':
			/*wglMakeCurrent(hdc, hglrc);
			Shader::Destroy();
			Ray::release();
			Graph::release();
			Shader::Init();
			Ray::generate(0.2, 25.0, 8.0, 300, 300, 1, 256, 256);
			Graph::generate();
			wglMakeCurrent(0, 0);*/
			break;
		}
		needRender = true;
		break;
	case WM_MOUSEMOVE:
		if (mouseON || (wParam & MK_RBUTTON)) {
			int dx = prevMy - GET_Y_LPARAM(lParam);
			int dy = prevMx - GET_X_LPARAM(lParam);
			if (dx != 0 || dy != 0) {
				Camera::rotate(dx / 100.0f, dy / 100.0f);
				needRender = true;
			}
		}
		prevMx = GET_X_LPARAM(lParam);
		prevMy = GET_Y_LPARAM(lParam);
		break;
	case WM_ERASEBKGND: return 1;
	case WM_SIZE:
	{
		int windowWidth = LOWORD(lParam);
		int windowHeight = HIWORD(lParam);
		RECT r;
		GetClientRect(mainW, &r);
		clientWindowWidth = r.right - r.left;
		clientWindowHeight = r.bottom - r.top;
		Camera::changeRatio(2.0f * (float)clientWindowWidth / (float)clientWindowHeight);
		SetWindowPos(settingsHWND, NULL, windowWidth / 2, windowHeight / 2, windowWidth / 2, windowHeight / 2, NULL);
		needRender = true;
	}
	break;
	case WM_CLOSE:
		if (measure) stopMeasure();
		if (appRun) {
			correct = false;
			appRun = false;
			wglMakeCurrent(hdc, hglrc);
			Shader::Destroy();
			Background::release();
			Ray::release();
			Plane::release();
			Lens::release();
			Graph::release();
			Optics::release();
			wglMakeCurrent(0, 0);
			wglDeleteContext(hglrc);
			ReleaseDC(mainW, hdc);
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default: return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
/*
class windowDescription {
public:
	wstring text;
	windowDescription(wstring cText) : text(cText) {};
	virtual void create();
};
class windowEditDescription : public windowDescription {
public:
	HWND *window;
	windowEditDescription(HWND *cWindow, wstring cText) : windowDescription(cText), window(cWindow) {};
	virtual void create();
};
class floatW : public windowEditDescription {
public:
	floatW(HWND *cWindow, wstring cText = L"0.0") : windowEditDescription(cWindow, cText) {};
	void create() {

	}
};
class buttonW : public windowEditDescription {
public:
	buttonW(HWND *cWindow, wstring cText) : windowEditDescription(cWindow, cText) {};
	void create() {

	}
};*/

struct windowF {
	HWND* window;
	std::wstring text, label;
};
void createTableF(windowF* windowArray, int size, int sizeX, int sizeY, wstring* columnNames, int startX, int startY, int gapX, int gapY, HWND parent = 0) {
	static int hmenuIter = 10000;
	for (int i = 0; i < sizeX; ++i) {
		if (!columnNames[i].empty()) CreateWindow(TEXT("static"), columnNames[i].c_str(), WS_CHILD | WS_VISIBLE, startX + i * gapX, startY, 60, 20, parent, NULL, hInst, NULL);
		for (int j = 0; j < sizeY; ++j) {
			int ind = j + sizeY * i;
			if (ind < size) {
				HWND tmpHWND = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), windowArray[ind].text.c_str(), MY_WS_FLOAT, startX + i * gapX, startY + j * gapY + 20, 60, 20, parent, (HMENU)(hmenuIter++), hInst, NULL);
				if (windowArray[ind].window) *(windowArray[ind].window) = tmpHWND;
				if (!windowArray[ind].label.empty()) CreateWindow(TEXT("static"), windowArray[ind].label.c_str(), WS_CHILD | WS_VISIBLE | SS_CENTER, startX + i * gapX - 20, startY + j * gapY + 20, 20, 20, parent, NULL, hInst, NULL);
			}
		}
	}
}

LRESULT CALLBACK SettingsWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	constexpr size_t tabCount = 6;
	static HWND hwndTab, hTab[tabCount], hwUnderTab;
	static int w, h;
	switch (message) {
	case WM_CREATE:
	{
		RECT rcClient;
		INITCOMMONCONTROLSEX icex;
		icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icex.dwICC = ICC_TAB_CLASSES;
		InitCommonControlsEx(&icex);
		GetClientRect(hWnd, &rcClient);
		hwndTab = CreateWindow(WC_TABCONTROL, TEXT(""), WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, rcClient.left, rcClient.top, rcClient.right - rcClient.left, (rcClient.bottom - rcClient.top) / 2, hWnd, NULL, hInst, NULL);
		insertTabs(hInst, hwndTab, hTab, TEXT("Lens"), TEXT("Camera"), TEXT("Optics"), TEXT("Rays"), TEXT("Emitter"), TEXT("Graph"));
		hwUnderTab = CreateWindow(TEXT("static"), TEXT(""), WS_CHILD | WS_VISIBLE, rcClient.left, (rcClient.bottom - rcClient.top) / 2, rcClient.right - rcClient.left, (rcClient.bottom - rcClient.top) / 2, hWnd, NULL, hInst, NULL);

		//windowDescription wnds[] = { floatW(&hwETlx, L"0.09"), floatW(&hwETly), floatW(&hwETlz), floatW(&hwETlrx), floatW(&hwETlry), floatW(&hwETlrz) };
		//Lens
		HWND hLensTab = hTab[0];
		{
			windowF wndF[6] = { { &hwETlx, L"0.09", L"" },{ &hwETly, L"0.0", L"" },{ &hwETlz, L"0.0", L"" },{ &hwETlrx, L"0.0", L"x" },{ &hwETlry, L"0.0", L"y" },{ &hwETlrz, L"0.0", L"z" } };
			wstring columns[2] = { L"Translate", L"Rotate" };
			createTableF(wndF, sizeof(wndF) / sizeof(windowF), sizeof(columns) / sizeof(wstring), sizeof(wndF) * sizeof(wstring) / (sizeof(columns) * sizeof(windowF)), columns, 0, 5, 80, 25, hLensTab);
		}
		CreateWindow(TEXT("button"), TEXT("Random"), MY_WS_BUTTON, 180, 25, 80, 20, hLensTab, MY_HMENU_BRANDOM, hInst, NULL);
		CreateWindow(TEXT("button"), TEXT("Ideal"), MY_WS_BUTTON, 180, 75, 80, 20, hLensTab, MY_HMENU_BIDEAL, hInst, NULL);
		hwETlax1 = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("x"), MY_WS_AXIS, 140, 25, 20, 20, hLensTab, (HMENU)(10006), hInst, NULL);
		hwETlax2 = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("y"), MY_WS_AXIS, 140, 50, 20, 20, hLensTab, (HMENU)(10007), hInst, NULL);
		hwETlax3 = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("z"), MY_WS_AXIS, 140, 75, 20, 20, hLensTab, (HMENU)(10008), hInst, NULL);
		//Camera
		HWND hCamTab = hTab[1];
		CreateWindow(TEXT("static"), TEXT("Distance"), WS_CHILD | WS_VISIBLE, 0, 5, 60, 20, hCamTab, NULL, hInst, NULL);
		hwETsx = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("538.72"), MY_WS_FLOAT, 0, 25, 60, 20, hCamTab, (HMENU)(10100), hInst, NULL);
		hwETsy = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("-51.423"), MY_WS_FLOAT, 80, 25, 60, 20, hCamTab, (HMENU)(10105), hInst, NULL);
		CreateWindow(TEXT("static"), TEXT("Size"), WS_CHILD | WS_VISIBLE, 0, 55, 60, 20, hCamTab, NULL, hInst, NULL);
		hwETswmm = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("10.0"), MY_WS_FLOAT, 0, 75, 60, 20, hCamTab, (HMENU)(10101), hInst, NULL);
		hwETshmm = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("10.0"), MY_WS_FLOAT, 0, 100, 60, 20, hCamTab, (HMENU)(10102), hInst, NULL);
		CreateWindow(TEXT("static"), TEXT("Resolution"), WS_CHILD | WS_VISIBLE, 80, 55, 70, 20, hCamTab, NULL, hInst, NULL);
		hwETswpix = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("256"), MY_WS_INT, 80, 75, 60, 20, hCamTab, (HMENU)(10103), hInst, NULL);
		hwETshpix = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("256"), MY_WS_INT, 80, 100, 60, 20, hCamTab, (HMENU)(10104), hInst, NULL);
		CreateWindow(TEXT("static"), TEXT("w"), WS_CHILD | WS_VISIBLE | SS_CENTER, 60, 75, 20, 20, hCamTab, NULL, hInst, NULL);
		CreateWindow(TEXT("static"), TEXT("h"), WS_CHILD | WS_VISIBLE | SS_CENTER, 60, 100, 20, 20, hCamTab, NULL, hInst, NULL);

		CreateWindow(TEXT("button"), TEXT("standart"), WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP, 180, 25, 80, 20, hCamTab, (HMENU)MY_HMENU_BRS, hInst, NULL);
		CreateWindow(TEXT("button"), TEXT("8bit"), WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 180, 50, 80, 20, hCamTab, (HMENU)MY_HMENU_BR8, hInst, NULL);
		CreateWindow(TEXT("button"), TEXT("16bit"), WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 180, 75, 80, 20, hCamTab, (HMENU)MY_HMENU_BR16, hInst, NULL);
		HWND tmpHWND1 = CreateWindow(TEXT("button"), TEXT("32bit"), WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 180, 100, 80, 20, hCamTab, (HMENU)MY_HMENU_BR32, hInst, NULL);
		SendMessage(tmpHWND1, BM_SETCHECK, BST_CHECKED, 0);
		//Optics
		HWND hOpticsTab = hTab[2];
		CreateWindow(TEXT("static"), TEXT("Distance"), WS_CHILD | WS_VISIBLE, 0, 5, 60, 20, hOpticsTab, NULL, hInst, NULL);
		hwETmx = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("400.0"), MY_WS_FLOAT, 0, 25, 60, 20, hOpticsTab, (HMENU)(10400), hInst, NULL);
		hwETmd = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("80.0"), MY_WS_FLOAT, 0, 50, 60, 20, hOpticsTab, (HMENU)(10401), hInst, NULL);
		CreateWindow(TEXT("static"), TEXT("Angle"), WS_CHILD | WS_VISIBLE, 80, 5, 60, 20, hOpticsTab, NULL, hInst, NULL);
		hwETma = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("70.0"), MY_WS_FLOAT, 80, 25, 60, 20, hOpticsTab, (HMENU)(10402), hInst, NULL);
		CreateWindow(TEXT("static"), TEXT("Radius"), WS_CHILD | WS_VISIBLE, 160, 5, 60, 20, hOpticsTab, NULL, hInst, NULL);
		hwETmr = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("400.0"), MY_WS_FLOAT, 160, 25, 60, 20, hOpticsTab, (HMENU)(10403), hInst, NULL);
		CreateWindow(TEXT("static"), TEXT("Size"), WS_CHILD | WS_VISIBLE, 240, 5, 60, 20, hOpticsTab, NULL, hInst, NULL);
		hwETml = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("50.8"), MY_WS_FLOAT, 240, 25, 60, 20, hOpticsTab, (HMENU)(10404), hInst, NULL);
		CreateWindow(TEXT("static"), TEXT("dx"), WS_CHILD | WS_VISIBLE, 0, 75, 60, 20, hOpticsTab, NULL, hInst, NULL);
		hwETmdx = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("-61.284"), MY_WS_FLOAT | ES_READONLY, 0, 100, 60, 20, hOpticsTab, (HMENU)(10405), hInst, NULL);
		CreateWindow(TEXT("static"), TEXT("dy"), WS_CHILD | WS_VISIBLE, 80, 75, 60, 20, hOpticsTab, NULL, hInst, NULL);
		hwETmdy = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("-51.423"), MY_WS_FLOAT | ES_READONLY, 80, 100, 60, 20, hOpticsTab, (HMENU)(10406), hInst, NULL);
		//Rays
		HWND hRaysTab = hTab[3];
		CreateWindow(TEXT("static"), TEXT("Grid type"), WS_CHILD | WS_VISIBLE, 160, 5, 70, 20, hRaysTab, NULL, hInst, NULL);
		hwCBtype = CreateWindow(TEXT("COMBOBOX"), TEXT("CBtype"), WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS, 160, 25, 140, 200, hRaysTab, MY_HMENU_BLIST, hInst, NULL);
		addStrings(hwCBtype, TEXT("Rect"), TEXT("Gauss"), TEXT("Ellipse"), TEXT("Rect S"), TEXT("Gauss S"), TEXT("Ellipse S"));
		SendMessage(hwCBtype, CB_SETCURSEL, (LPARAM)1, 0);
		CreateWindow(TEXT("static"), TEXT("Divergence"), WS_CHILD | WS_VISIBLE, 0, 5, 75, 20, hRaysTab, NULL, hInst, NULL);
		hwETfa = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("25.0"), MY_WS_FLOAT, 0, 25, 60, 20, hRaysTab, (HMENU)(10200), hInst, NULL);
		hwETsa = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("8.0"), MY_WS_FLOAT, 0, 50, 60, 20, hRaysTab, (HMENU)(10201), hInst, NULL);
		CreateWindow(TEXT("static"), TEXT("Count"), WS_CHILD | WS_VISIBLE, 80, 5, 70, 20, hRaysTab, NULL, hInst, NULL);
		hwETnx = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("100"), MY_WS_INT, 80, 25, 60, 20, hRaysTab, (HMENU)(10202), hInst, NULL);
		hwETny = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("100"), MY_WS_INT, 80, 50, 60, 20, hRaysTab, (HMENU)(10203), hInst, NULL);
		CreateWindow(TEXT("static"), TEXT("f"), WS_CHILD | WS_VISIBLE | SS_CENTER, 60, 25, 20, 20, hRaysTab, NULL, hInst, NULL);
		CreateWindow(TEXT("static"), TEXT("s"), WS_CHILD | WS_VISIBLE | SS_CENTER, 60, 50, 20, 20, hRaysTab, NULL, hInst, NULL);
		CreateWindow(TEXT("static"), TEXT("Size"), WS_CHILD | WS_VISIBLE, 0, 80, 70, 20, hRaysTab, NULL, hInst, NULL);
		hwETps = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("1"), MY_WS_INT, 0, 100, 60, 20, hRaysTab, (HMENU)(10204), hInst, NULL);
		CreateWindow(TEXT("static"), TEXT("Intensity"), WS_CHILD | WS_VISIBLE, 80, 80, 70, 20, hRaysTab, NULL, hInst, NULL);
		hwETpi = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("0.5"), MY_WS_FLOAT, 80, 100, 60, 20, hRaysTab, (HMENU)(10205), hInst, NULL);
		CreateWindow(TEXT("static"), TEXT("Width"), WS_CHILD | WS_VISIBLE, 160, 80, 70, 20, hRaysTab, NULL, hInst, NULL);
		hwETrw = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("0.2"), MY_WS_FLOAT, 160, 100, 60, 20, hRaysTab, (HMENU)(10206), hInst, NULL);
		hwETrn = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("1"), MY_WS_INT, 160, 50, 60, 20, hRaysTab, (HMENU)(10207), hInst, NULL);
		hwETrd = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("0.0"), MY_WS_FLOAT, 240, 50, 60, 20, hRaysTab, (HMENU)(10208), hInst, NULL);
		//Emitter
		HWND hEmitterTab = hTab[4];
		{
			windowF wndF[6] = { { &hwETsmile, L"0.0", L"Scl" },{ &hwETsmileSft, L"0.0", L"Sft" },{ &hwETsmilePlt, L"0.0", L"Plt" },
			{ &hwETerx, L"0.0", L"x" },{ &hwETery, L"0.0", L"y" },{ &hwETerz, L"0.0", L"z" } };
			wstring columns[2] = { L"Smile", L"Rotate" };
			createTableF(wndF, sizeof(wndF) / sizeof(windowF), sizeof(columns) / sizeof(wstring), sizeof(wndF) * sizeof(wstring) / (sizeof(columns) * sizeof(windowF)), columns, 25, 5, 80, 25, hEmitterTab);
		}
		//Graph
		HWND hGraphTab = hTab[5];
		hwBGgrid = CreateWindow(TEXT("button"), TEXT("grid"), MY_WS_BUTTON | BS_AUTOCHECKBOX | BS_LEFTTEXT, 0, 0, 80, 20, hGraphTab, MY_HMENU_BGGRID, hInst, NULL);
		hwBGauto = CreateWindow(TEXT("button"), TEXT("autoplot"), MY_WS_BUTTON | BS_AUTOCHECKBOX | BS_LEFTTEXT, 0, 50, 80, 20, hGraphTab, MY_HMENU_BGAUTO, hInst, NULL);
		hwBGline = CreateWindow(TEXT("button"), TEXT("lines"), MY_WS_BUTTON | BS_AUTOCHECKBOX | BS_LEFTTEXT, 100, 0, 80, 20, hGraphTab, MY_HMENU_BGLINE, hInst, NULL);
		hwBGpoint = CreateWindow(TEXT("button"), TEXT("points"), MY_WS_BUTTON | BS_AUTOCHECKBOX | BS_LEFTTEXT, 100, 25, 80, 20, hGraphTab, MY_HMENU_BGPOINT, hInst, NULL);
		hwBGticks = CreateWindow(TEXT("button"), TEXT("ticks"), MY_WS_BUTTON | BS_AUTOCHECKBOX | BS_LEFTTEXT, 0, 25, 80, 20, hGraphTab, MY_HMENU_BGTICK, hInst, NULL);
		hwBGcursors = CreateWindow(TEXT("button"), TEXT("cursor"), MY_WS_BUTTON | BS_AUTOCHECKBOX | BS_LEFTTEXT, 0, 75, 80, 20, hGraphTab, MY_HMENU_BGCURSOR, hInst, NULL);
		CreateWindow(TEXT("button"), TEXT("Save"), MY_WS_BUTTON, 100, 50, 80, 20, hGraphTab, MY_HMENU_BGSCREENSHOT, hInst, NULL);
		hwBGprofile = CreateWindow(TEXT("button"), TEXT("profile"), MY_WS_BUTTON | BS_AUTOCHECKBOX | BS_LEFTTEXT, 100, 75, 80, 20, hGraphTab, MY_HMENU_BGPROFILE, hInst, NULL);
		SendMessage(hwBGgrid, BM_SETCHECK, BST_UNCHECKED, 0);
		SendMessage(hwBGauto, BM_SETCHECK, BST_CHECKED, 0);
		SendMessage(hwBGline, BM_SETCHECK, BST_CHECKED, 0);
		SendMessage(hwBGpoint, BM_SETCHECK, BST_CHECKED, 0);
		SendMessage(hwBGticks, BM_SETCHECK, BST_CHECKED, 0);
		SendMessage(hwBGcursors, BM_SETCHECK, BST_CHECKED, 0);
		SendMessage(hwBGprofile, BM_SETCHECK, BST_CHECKED, 0);
		//Under tab
		hwETstart = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), fTos(measStart), MY_WS_FLOAT, 3, 30, 60, 20, hwUnderTab, (HMENU)(10300), hInst, NULL);
		hwETstep = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), fTos(measStep), MY_WS_FLOAT, 83, 30, 60, 20, hwUnderTab, (HMENU)(10301), hInst, NULL);
		hwETend = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), fTos(measEnd), MY_WS_FLOAT, 163, 30, 60, 20, hwUnderTab, (HMENU)(10302), hInst, NULL);
		hwETcx = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("0.0"), MY_WS_FLOAT | ES_READONLY, 3, 55, 60, 20, hwUnderTab, (HMENU)(10303), hInst, NULL);
		hwETcy = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("0.0"), MY_WS_FLOAT | ES_READONLY, 83, 55, 60, 20, hwUnderTab, (HMENU)(10304), hInst, NULL);
		hwETwx = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("0.0"), MY_WS_FLOAT | ES_READONLY, 163, 55, 60, 20, hwUnderTab, (HMENU)(10305), hInst, NULL);
		hwETwy = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("0.0"), MY_WS_FLOAT | ES_READONLY, 243, 55, 60, 20, hwUnderTab, (HMENU)(10306), hInst, NULL);
		CreateWindow(TEXT("button"), TEXT("Correct"), MY_WS_BUTTON, 163, 5, 60, 20, hwUnderTab, MY_HMENU_BCORRECT, hInst, NULL);
		CreateWindow(TEXT("button"), TEXT("Start"), MY_WS_BUTTON, 243, 5, 60, 20, hwUnderTab, MY_HMENU_BGEN, hInst, NULL);
		CreateWindow(TEXT("button"), TEXT("Stop"), MY_WS_BUTTON, 243, 30, 60, 20, hwUnderTab, MY_HMENU_BSGEN, hInst, NULL);
		hwBgraph = CreateWindow(TEXT("button"), TEXT("plot"), MY_WS_BUTTON | BS_AUTOCHECKBOX | BS_LEFTTEXT, 243, 80, 60, 20, hwUnderTab, MY_HMENU_BGRAPH, hInst, NULL);
		CreateWindow(TEXT("button"), TEXT("file"), MY_WS_BUTTON | BS_AUTOCHECKBOX | BS_LEFTTEXT, 3, 80, 60, 20, hwUnderTab, MY_HMENU_BFILE, hInst, NULL);
		hwCBmeas = CreateWindow(TEXT("COMBOBOX"), TEXT("CBmeas"), WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS, 3, 5, 60, 300, hwUnderTab, MY_HMENU_BMEAS, hInst, NULL);
		addStrings(hwCBmeas, TEXT("x"), TEXT("y"), TEXT("z"), TEXT("Rx"), TEXT("Ry"), TEXT("Rz"), TEXT("Cam x"));
		SendMessage(hwCBmeas, CB_SETCURSEL, 0, (LPARAM)0);
		CreateWindow(TEXT("static"), TEXT("Incr"), WS_CHILD | WS_VISIBLE, 163, 105, 70, 20, hwUnderTab, NULL, hInst, NULL);
		hwETincr = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), fTos(incr), MY_WS_FLOAT, 163, 125, 60, 20, hwUnderTab, (HMENU)(10307), hInst, NULL);
		CreateWindow(TEXT("static"), TEXT("Param"), WS_CHILD | WS_VISIBLE, 3, 105, 70, 20, hwUnderTab, NULL, hInst, NULL);
		hwETdif = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("0.0"), MY_WS_FLOAT | ES_READONLY, 3, 125, 140, 20, hwUnderTab, (HMENU)(10308), hInst, NULL);
		CreateWindow(TEXT("button"), TEXT("cons"), MY_WS_BUTTON | BS_AUTOCHECKBOX | BS_LEFTTEXT, 243, 125, 60, 20, hwUnderTab, MY_HMENU_BCONSOLE, hInst, NULL);

		OriginalStaticProc = setProcedure(StaticProc, hTab[0], hTab[1], hTab[2], hTab[3], hTab[4], hTab[5], hwUnderTab);
		OriginalEditTextProc = setProcedure(EditTextFloatProc,
			hwETlx, hwETly, hwETlz, hwETlrx, hwETlry, hwETlrz, hwETfa, hwETsa, hwETpi, hwETrw, hwETsx, hwETsy, hwETrd,
			hwETstart, hwETstep, hwETend, hwETincr, hwETswmm, hwETshmm, hwETdif, hwETmx, hwETmd, hwETma, hwETmr, hwETml,
			hwETsmile, hwETsmileSft, hwETsmilePlt, hwETerx, hwETery, hwETerz);
		setProcedure(EditTextIntProc, hwETrn, hwETnx, hwETny, hwETps, hwETswpix, hwETshpix);
		setProcedure(EditTextAxisProc, hwETlax1, hwETlax2, hwETlax3);
	}
	break;
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == TCN_SELCHANGE) {
			int wmId = SendMessage(hwndTab, TCM_GETCURSEL, 0, 0);
			for (int i = 0; i < tabCount; i++) ShowWindow(hTab[i], SW_HIDE);
			ShowWindow(hTab[wmId], SW_SHOW);
			UpdateWindow(hTab[wmId]);
		}
		break;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdct = BeginPaint(hWnd, &ps);
		HDC hCdc = CreateCompatibleDC(hdct);
		HBITMAP hScreen = CreateCompatibleBitmap(hdct, w, h);
		HBITMAP oldBmp = (HBITMAP)SelectObject(hCdc, hScreen);
		PatBlt(hCdc, 0, 0, w, h, WHITENESS);
		BitBlt(hdct, 0, 0, w, h, hCdc, 0, 0, SRCCOPY);
		SelectObject(hCdc, oldBmp);
		DeleteObject(hScreen);
		DeleteDC(hCdc);
		EndPaint(hWnd, &ps);
	}
				 break;
	case WM_LBUTTONDOWN: SetFocus(settingsHWND); break;
	case WM_MOVE:
	case WM_SIZE:
		w = LOWORD(lParam);
		h = HIWORD(lParam);
		{
			RECT rcClient;
			GetClientRect(hWnd, &rcClient);
			SetWindowPos(hwndTab, NULL, rcClient.left, rcClient.top, rcClient.right - rcClient.left, (rcClient.bottom - rcClient.top) / 2, NULL);
			SetWindowPos(hwUnderTab, NULL, rcClient.left, (rcClient.bottom - rcClient.top) / 2, rcClient.right - rcClient.left, (rcClient.bottom - rcClient.top) / 2, NULL);
			GetClientRect(hwndTab, &rcClient);
			TabCtrl_AdjustRect(hwndTab, FALSE, &rcClient);
			for (int i = 0; i < tabCount; i++)
				SetWindowPos(hTab[i], NULL, rcClient.left, rcClient.top, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, NULL);
		}
		InvalidateRect(hWnd, NULL, false);
		UpdateWindow(hWnd);
		break;
	case WM_ERASEBKGND: return 1;
	default: return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK GraphWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static int screenshotLocalCounter = 0;
	switch (message) {
	case WM_PAINT: {
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		graphRender();
		EndPaint(hWnd, &ps);
	}
				 break;
	case WM_MOUSEMOVE:
		graphMx = GET_X_LPARAM(lParam);
		graphMy = GET_Y_LPARAM(lParam);
		needGraphRender = true;
		break;
	case WM_SIZE:
	{
		RECT r;
		GetClientRect(graphHWND, &r);
		Graph::w = r.right - r.left;
		Graph::h = r.bottom - r.top;

		wglMakeCurrent(ghdc, hglrc);
		Graph::generateTicks();
		wglMakeCurrent(0, 0);
	}
	InvalidateRect(hWnd, NULL, false);
	UpdateWindow(hWnd);
	break;
	case WM_ERASEBKGND: return 1;
	case WM_USER:
		if (graph) {
			WORD wWidth = Graph::w;
			WORD wHeight = Graph::h;
			PBITMAPINFO pBmInf = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFO));
			if (pBmInf != NULL) {
				pBmInf->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				pBmInf->bmiHeader.biWidth = wWidth;
				pBmInf->bmiHeader.biHeight = wHeight;
				pBmInf->bmiHeader.biCompression = BI_RGB;
				pBmInf->bmiHeader.biBitCount = 24;
				pBmInf->bmiHeader.biClrUsed = 16777216;
				pBmInf->bmiHeader.biPlanes = 1;
				pBmInf->bmiHeader.biSizeImage = (wWidth + 7) / 8 * wHeight * 24;
				PBYTE pBmData = (PBYTE)LocalAlloc(LPTR, pBmInf->bmiHeader.biSizeImage);
				if (pBmData != NULL) {
					wglMakeCurrent(ghdc, hglrc);
					Graph::getScreenShot(pBmData);
					wglMakeCurrent(0, 0);
					BITMAPFILEHEADER BmFileHdr;
					BmFileHdr.bfType = 0x4D42;
					BmFileHdr.bfReserved1 = BmFileHdr.bfReserved2 = 0;
					BmFileHdr.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
					BmFileHdr.bfSize = BmFileHdr.bfOffBits + pBmInf->bmiHeader.biSizeImage;
					HANDLE hFile = CreateFile(screenBuffer, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
					if (hFile != INVALID_HANDLE_VALUE) {
						DWORD dwBytes = 0;
						WriteFile(hFile, &BmFileHdr, sizeof(BITMAPFILEHEADER), &dwBytes, NULL);
						WriteFile(hFile, pBmInf, sizeof(BITMAPINFOHEADER), &dwBytes, NULL);
						WriteFile(hFile, pBmData, pBmInf->bmiHeader.biSizeImage, &dwBytes, NULL);
						CloseHandle(hFile);
					}
					LocalFree(pBmData);
				}
			}
			if (pBmInf != NULL) LocalFree(pBmInf);
		}
		break;
	case WM_CLOSE:
		graph = false;
		SendMessage(hwBgraph, BM_SETCHECK, BST_UNCHECKED, 0);
		ShowWindow(hWnd, SW_HIDE);
		break;
	default: return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK StaticProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static HBRUSH hbcx = NULL, hbcy = NULL, hbwx = NULL, hbwy = NULL;
	static int screenLocalCounter = 0;
	static int lastType = 1;
	switch (message) {
	case WM_CTLCOLORSTATIC:
	{
		HWND control = (HWND)lParam;
		if (control == hwETcx || control == hwETcy || control == hwETwx || control == hwETwy) {
			HDC hdcStatic = (HDC)wParam;
			SetTextColor(hdcStatic, RGB(0, 0, 0));
			HBRUSH returned;
			if (control == hwETcx) {
				SetBkColor(hdcStatic, RGB(255, 200, 200));
				if (hbcx == NULL) hbcx = CreateSolidBrush(RGB(255, 200, 200));
				returned = hbcx;
			}
			else if (control == hwETcy) {
				SetBkColor(hdcStatic, RGB(200, 255, 200));
				if (hbcy == NULL) hbcy = CreateSolidBrush(RGB(200, 255, 200));
				returned = hbcy;
			}
			else if (control == hwETwx) {
				SetBkColor(hdcStatic, RGB(200, 200, 255));
				if (hbwx == NULL) hbwx = CreateSolidBrush(RGB(200, 200, 255));
				returned = hbwx;
			}
			else if (control == hwETwy) {
				SetBkColor(hdcStatic, RGB(200, 255, 255));
				if (hbwy == NULL) hbwy = CreateSolidBrush(RGB(200, 255, 255));
				returned = hbwy;
			}
			return (INT_PTR)returned;
		}
		break;
	}
	case WM_COMMAND:
		if (IS_GRAPH_BUTTONS(wParam)) {
			switch (LOWORD(wParam)) {
			case (WORD)MY_HMENU_BGGRID: Graph::grid = Button_GetCheck(hwBGgrid); break;
			case (WORD)MY_HMENU_BGAUTO: graphAuto = Button_GetCheck(hwBGauto); break;
			case (WORD)MY_HMENU_BGLINE: Graph::line = Button_GetCheck(hwBGline); break;
			case (WORD)MY_HMENU_BGPOINT: Graph::point = Button_GetCheck(hwBGpoint); break;
			case (WORD)MY_HMENU_BGTICK: Graph::ticks = Button_GetCheck(hwBGticks); break;
			case (WORD)MY_HMENU_BGCURSOR: Graph::cursors = Button_GetCheck(hwBGcursors); break;
			case (WORD)MY_HMENU_BGPROFILE: drawProfile = Button_GetCheck(hwBGprofile); needRender = true; break;
			case (WORD)MY_HMENU_BGSCREENSHOT:
				if (graph) {
					_stprintf_s(screenBuffer, TEXT("screen_%d.bmp"), screenLocalCounter++);
					PostMessage(graphHWND, WM_USER, 0, 0);
				}
				break;
			}
			needGraphRender = true;
		}
		else if (IS_MISC_BUTTONS(wParam)) {
			wglMakeCurrent(hdc, hglrc);
			switch (LOWORD(wParam)) {
			case (WORD)MY_HMENU_BRS: Ray::changeTextureFormat(0); break;
			case (WORD)MY_HMENU_BR8: Ray::changeTextureFormat(1); break;
			case (WORD)MY_HMENU_BR16: Ray::changeTextureFormat(2); break;
			case (WORD)MY_HMENU_BR32: Ray::changeTextureFormat(3); break;
			case (WORD)MY_HMENU_BLIST:
				if (HIWORD(wParam) == CBN_SELCHANGE) {
					if (!measure) {
						int type = (int)SendMessage(hwCBtype, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
						if (type == CB_ERR) return 0;
						if (lastType != type) {
							lastType = type;
							Ray::Mode = (Ray::MODE)type;
							Ray::changeRays();
						}
					}
					else PostMessage(hwCBtype, CB_SETCURSEL, (WPARAM)lastType, (LPARAM)0);
				}
				break;
			case (WORD)MY_HMENU_BRANDOM:
				srand(rotl32(GetTickCount64() % 100000ULL, 4));
				Lens::moveToX(GET_RAND(0.02, 0.08)); setFloatText(hwETlx, Lens::getX());
				Lens::moveToY(GET_RAND(0.04, -0.02)); setFloatText(hwETly, Lens::getY());
				Lens::moveToZ(GET_RAND(0.04, -0.02)); setFloatText(hwETlz, Lens::getZ());
				Lens::rotateTo(GET_RAND(0.01, -0.005), Axis::X); setFloatText(hwETlrx, Lens::getRots(0));
				Lens::rotateTo(GET_RAND(0.01, -0.005), Axis::Y); setFloatText(hwETlry, Lens::getRots(1));
				Lens::rotateTo(GET_RAND(0.026, -0.013), Axis::Z); setFloatText(hwETlrz, Lens::getRots(2));
				break;
			case (WORD)MY_HMENU_BIDEAL:
				Lens::moveToX(0.09); setFloatText(hwETlx, Lens::getX());
				Lens::moveToY(0); setFloatText(hwETly, Lens::getY());
				Lens::moveToZ(0); setFloatText(hwETlz, Lens::getZ());
				Lens::rotateTo(0, Axis::X); setFloatText(hwETlrx, Lens::getRots(0));
				Lens::rotateTo(0, Axis::Y); setFloatText(hwETlry, Lens::getRots(1));
				Lens::rotateTo(0, Axis::Z); setFloatText(hwETlrz, Lens::getRots(2));
				break;
			}
			wglMakeCurrent(0, 0);
			needRecalc = needRender = true;
		}
		else if (IS_MAIN_BUTTONS(wParam)) {
			switch (LOWORD(wParam)) {
			case (WORD)MY_HMENU_BSGEN: correct = false;	if (measure) stopMeasure(); break;
			case (WORD)MY_HMENU_BGRAPH: ShowWindow(graphHWND, (graph = !graph) ? SW_SHOW : SW_HIDE); break;
			case (WORD)MY_HMENU_BFILE: file = !file; break;
			case (WORD)MY_HMENU_BCORRECT: correct = true; phase = 0; break;
			case (WORD)MY_HMENU_BCONSOLE: toggleConsole(); break;
			case (WORD)MY_HMENU_BGEN:
				if (HIWORD(wParam) == BN_CLICKED) {
					if (!measure) {
						graphDataSize = std::abs((measStart - measEnd) / measStep);
						std::cout << graphDataSize << std::endl;
						if (graphDataSize > 0) {
							measCurrent = 0;
							Graph::clear();
							graphDataSize++;
							measure = true;
							dwText = "";
							gtt64All = GetTickCount64();
						}
					}
				}
				break;
			case (WORD)MY_HMENU_BMEAS:
				if (HIWORD(wParam) == CBN_SELCHANGE) {
					if (!measure) {
						measType = (int)SendMessage(hwCBmeas, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
						switch (measType) {
						case 0: measStart = Lens::getX(); break;
						case 1: measStart = Lens::getY(); break;
						case 2: measStart = Lens::getZ(); break;
						case 3: measStart = Lens::getRots(0); break;
						case 4: measStart = Lens::getRots(1); break;
						case 5: measStart = Lens::getRots(2); break;
						case 6: measStart = Plane::getX();  break;
						}
						setFloatText(hwETstart, measStart);
					}
					else PostMessage(hwCBmeas, CB_SETCURSEL, (WPARAM)measType, (LPARAM)0);
				}
				break;
			}
		}
		break;
	}
	return CallWindowProc(OriginalStaticProc, hWnd, message, wParam, lParam);
}

void switchHWNDfloat(HWND hWnd, float ControlText) {
	if (!measure) {
		wglMakeCurrent(hdc, hglrc);
		if (hWnd == hwETlx) Lens::moveToX(ControlText);
		else if (hWnd == hwETly) Lens::moveToY(ControlText);
		else if (hWnd == hwETlz) Lens::moveToZ(ControlText);
		else if (hWnd == hwETlrx) Lens::rotateTo(ControlText, Axis::X);
		else if (hWnd == hwETlry) Lens::rotateTo(ControlText, Axis::Y);
		else if (hWnd == hwETlrz) Lens::rotateTo(ControlText, Axis::Z);
		else if (hWnd == hwETsx) Plane::moveToX(ControlText);
		else if (hWnd == hwETsy) Plane::moveToY(ControlText);
		else if (hWnd == hwETswmm) Plane::setW(ControlText);
		else if (hWnd == hwETshmm) Plane::setH(ControlText);
		else if (hWnd == hwETmx || hWnd == hwETmd || hWnd == hwETma || hWnd == hwETmr || hWnd == hwETml) {
			float mirX = getFloatText(hwETmx);
			float mirRad = getFloatText(hwETmr);
			float dx, dy;
			Optics::setParams(mirX, mirRad, getFloatText(hwETmd), getFloatText(hwETml), getFloatText(hwETma) * 3.14159265358979323 / 180.0, &dx, &dy);
			//
			Plane::moveToX(mirX + mirRad / 2 - dx);
			setFloatText(hwETsx, mirX + mirRad / 2 - dx);
			setFloatText(hwETmdx, -dx);
			Plane::moveToY(-dy);
			setFloatText(hwETsy, -dy);
			setFloatText(hwETmdy, -dy);
			//

			/*float a1 = getFloatText(hwETmx);
			float a2 = getFloatText(hwETmr);
			float a3 = getFloatText(hwETmd);
			float a4 = getFloatText(hwETml);
			glUseProgram(Shader::Line::programHandle);
			glUniform4f(Shader::Line::Uniform::uOpticsParam, a1, a2, a3, a4);
			glUseProgram(0);*/

		}
		else if (hWnd == hwETfa) { Ray::FA = ControlText; Ray::changeRays(); }
		else if (hWnd == hwETsa) { Ray::SA = ControlText; Ray::changeRays(); }
		else if (hWnd == hwETrd) { Ray::Dist = ControlText; Ray::changeRays(); }
		else if (hWnd == hwETrw) { Ray::width = ControlText; Ray::changeRays(); }
		else if (hWnd == hwETsmile) { Ray::Smile = ControlText; Ray::changeRays(); }
		else if (hWnd == hwETsmileSft) { Ray::SmileShift = ControlText; Ray::changeRays(); }
		else if (hWnd == hwETsmilePlt) { Ray::SmilePlateau = ControlText; Ray::changeRays(); }
		else if (hWnd == hwETerx) { Ray::Rx = ControlText; Ray::changeRays(); }
		else if (hWnd == hwETery) { Ray::Ry = ControlText; Ray::changeRays(); }
		else if (hWnd == hwETerz) { Ray::Rz = ControlText; Ray::changeRays(); }
		else if (hWnd == hwETpi) Ray::pIntensity = ControlText;
		else if (hWnd == hwETstart) measStart = ControlText;
		else if (hWnd == hwETstep) measStep = ControlText;
		else if (hWnd == hwETend) measEnd = ControlText;
		else if (hWnd == hwETincr) incr = ControlText;
		wglMakeCurrent(0, 0);
		needRecalc = needRender = true;
	}
}
LRESULT CALLBACK EditTextFloatProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
	case WM_KEYDOWN:
		if (wParam == VK_UP || wParam == VK_DOWN || wParam == VK_DELETE) {
			if (!measure) {
				const float ControlText = getFloatText(hWnd);
				switch (wParam) {
				case VK_UP: setFloatText(hWnd, ControlText + incr); break;
				case VK_DOWN: setFloatText(hWnd, ControlText - incr); break;
				}
				PostMessage(hWnd, WM_USER, 0, 0);
			}
			else return 0;
		}
		break;
	case  WM_CHAR:
		if (measure) return 0;
		if (wParam == TEXT('.')) {
			int count = SendMessage(hWnd, WM_GETTEXT, (WPARAM)(bSize - 1), (LPARAM)tmpBuffer);
			for (int i = 0; i < count; ++i) if (tmpBuffer[i] == TEXT('.')) return 0;
			for (int i = 0; i < count; ++i)
				if (tmpBuffer[i] == TEXT('-'))
					if (LOWORD(SendMessage(hWnd, EM_GETSEL, (WPARAM)0, (LPARAM)0)) == 0) return 0;
		}
		else if (wParam == TEXT('-')) {
			int count = SendMessage(hWnd, WM_GETTEXT, (WPARAM)(bSize - 1), (LPARAM)tmpBuffer);
			for (int i = 0; i < count; ++i) if (tmpBuffer[i] == TEXT('-')) return 0;
			if (LOWORD(SendMessage(hWnd, EM_GETSEL, (WPARAM)0, (LPARAM)0)) != 0) return 0;
		}
		if (!((wParam >= TEXT('0') && wParam <= TEXT('9'))
			|| wParam == TEXT('.')
			|| wParam == TEXT('-')
			|| wParam == MY_KEY_DELETE
			|| wParam == MY_KEY_BACKSPACE))
			return 0;
		{
			int count = SendMessage(hWnd, WM_GETTEXT, (WPARAM)(bSize - 1), (LPARAM)tmpBuffer);
			for (int i = 0; i < count; ++i)
				if (tmpBuffer[i] == TEXT('-')) {
					LRESULT sm = SendMessage(hWnd, EM_GETSEL, (WPARAM)0, (LPARAM)0);
					if (LOWORD(sm) == 0 && HIWORD(sm) == LOWORD(sm)) return 0;
				}
		}
		PostMessage(hWnd, WM_USER, 0, 0);
		break;
	case WM_USER:
		switchHWNDfloat(hWnd, getFloatText(hWnd));
		break;
	}
	return CallWindowProc(OriginalEditTextProc, hWnd, message, wParam, lParam);
}

void switchHWNDint(HWND hWnd, int ControlText) {
	if (!measure) {
		wglMakeCurrent(hdc, hglrc);
		if (hWnd == hwETswpix) Ray::changeSensorResolutionW(ControlText);
		else if (hWnd == hwETshpix) Ray::changeSensorResolutionH(ControlText);
		else if (hWnd == hwETnx) { Ray::fStep = ControlText; Ray::changeRays(); }
		else if (hWnd == hwETny) { Ray::sStep = ControlText; Ray::changeRays(); }
		else if (hWnd == hwETrn) { Ray::Num = ControlText; Ray::changeRays(); }
		else if (hWnd == hwETps) Ray::pSize = ControlText;
		wglMakeCurrent(0, 0);
		needRecalc = needRender = true;
	}
}
LRESULT CALLBACK EditTextIntProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (message == WM_KEYDOWN) {
		if (wParam == VK_UP || wParam == VK_DOWN) {
			if (!measure) {
				const int ControlText = getIntText(hWnd);
				switch (wParam) {
				case VK_UP:
					setIntText(hWnd, ControlText + 1);
					switchHWNDint(hWnd, ControlText + 1); //FIXME do like EditTextFloatProc
					break;
				case VK_DOWN:
					setIntText(hWnd, ControlText - 1);
					switchHWNDint(hWnd, ControlText - 1);
					break;
				}
			}
		}
	}
	else if (message == WM_CHAR) {
		if (measure) return 0;
		PostMessage(hWnd, WM_USER, 0, 0);
	}
	else if (message == WM_USER) switchHWNDint(hWnd, getIntText(hWnd));
	return CallWindowProc(OriginalEditTextProc, hWnd, message, wParam, lParam);
}

LRESULT CALLBACK EditTextAxisProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (message == WM_CHAR) {
		if (measure) return 0;
		Axis axis = Axis::X;
		switch (wParam) {
		case TEXT('X'): case TEXT('x'): axis = Axis::X; break;
		case TEXT('Y'): case TEXT('y'): axis = Axis::Y; break;
		case TEXT('Z'): case TEXT('z'): axis = Axis::Z; break;
		default: return 0;
		}
		wglMakeCurrent(hdc, hglrc);
		if (hWnd == hwETlax1) Lens::setAxis(0, axis);
		else if (hWnd == hwETlax2) Lens::setAxis(1, axis);
		else if (hWnd == hwETlax3) Lens::setAxis(2, axis);
		wglMakeCurrent(0, 0);
		needRecalc = needRender = true;
	}
	return CallWindowProc(OriginalEditTextProc, hWnd, message, wParam, lParam);
}

void stopMeasure() {
	measure = false;
	std::cout << "All time: " << GetTickCount64() - gtt64All << " ms" << std::endl;

	wglMakeCurrent(ghdc, hglrc);
	ComboBox_GetText(hwCBmeas, tmpBuffer, bSize);
	Graph::setXlabel(tmpBuffer);
	Graph::generateTicks();
	wglMakeCurrent(0, 0);
	graphRender();

	static int fileLocalCounter = 0;
	if (file) {
		DWORD dwTemp;
		_stprintf_s(tmpBuffer, TEXT("data_%d.txt"), fileLocalCounter);
		HANDLE hFile = CreateFile(tmpBuffer, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		WriteFile(hFile, dwText.c_str(), dwText.size(), &dwTemp, NULL);
		CloseHandle(hFile);

		if (graph) {
			_stprintf_s(screenBuffer, TEXT("data_%d.bmp"), fileLocalCounter);
			PostMessage(graphHWND, WM_USER, 0, 0);
		}

		++fileLocalCounter;
	}
}

int LOG(const char* message, int num = 0) { cout << message << ' ' << num << endl; return 0; }
#define CHECK(chk) if (chk) LOG("OK "#chk); else { MessageBox(NULL, TEXT("ERROR "#chk), NULL, NULL); return LOG("ERROR "#chk);}

int createOpenGLWindow() {
	{	//Fake window for loading functions
		TCHAR szFakeClass[] = TEXT("EdFakeClass");
		WNDCLASSEX wcexFake;
		memset(&wcexFake, 0, sizeof(WNDCLASSEX));
		wcexFake.cbSize = sizeof(WNDCLASSEX);
		wcexFake.lpfnWndProc = FakeWndProc;
		wcexFake.hInstance = hInst;
		wcexFake.lpszClassName = szFakeClass;
		CHECK(RegisterClassEx(&wcexFake));

		HWND hwndFake = CreateWindow(szFakeClass, TEXT("FAKE"), WS_OVERLAPPEDWINDOW | WS_MAXIMIZE | WS_CLIPCHILDREN,
			0, 0, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInst, NULL);
		CHECK(hwndFake);

		HDC hdcFake = GetDC(hwndFake);
		CHECK(hdcFake);

		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
		LOG("Fake created ms");
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 24;
		pfd.cStencilBits = 8;
		int iPixelFormat = ChoosePixelFormat(hdcFake, &pfd);
		CHECK(iPixelFormat);
		CHECK(SetPixelFormat(hdcFake, iPixelFormat, &pfd));

		HGLRC hRCFake = wglCreateContext(hdcFake);
		CHECK(hRCFake);
		CHECK(wglMakeCurrent(hdcFake, hRCFake));

		glewExperimental = GL_TRUE;
		CHECK(glewInit() == GLEW_OK);

		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hRCFake);
		ReleaseDC(hwndFake, hdcFake);
		DestroyWindow(hwndFake);
		UnregisterClass(szFakeClass, hInst);
	}
	{
		TCHAR szMainClass[] = TEXT("EdMainClass");
		initClass(hInst, MainWndProc, szMainClass, true);
		mainW = CreateWindow(szMainClass, TEXT("Lens"),
			WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_CLIPCHILDREN,
			200, 200, 630, 630, NULL, NULL, hInst, NULL);
		CHECK(mainW);
		hdc = GetDC(mainW);
		CHECK(hdc);

		PIXELFORMATDESCRIPTOR pfdNULL;
		memset(&pfdNULL, 0, sizeof(PIXELFORMATDESCRIPTOR));
		const int attribPFList[] = {
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			0
		};
		const int attribCntxList[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
			WGL_CONTEXT_MINOR_VERSION_ARB, 3,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0
		};
		int pixelFormat;
		UINT numFormats;
		wglChoosePixelFormatARB(hdc, attribPFList, 0, 1, &pixelFormat, &numFormats);
		LOG("pf: ", pixelFormat);
		LOG("nf: ", numFormats);
		CHECK(SetPixelFormat(hdc, pixelFormat, &pfdNULL));
		hglrc = wglCreateContextAttribsARB(hdc, 0, attribCntxList);
		CHECK(hglrc);
		CHECK(wglMakeCurrent(hdc, hglrc));

		TCHAR szGraphClass[] = TEXT("EdGraphClass");
		initClass(hInst, GraphWndProc, szGraphClass, true);
		graphHWND = CreateWindow(szGraphClass, TEXT("Graph"),
			WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_CLIPCHILDREN,
			0, 0, 200, 200,
			NULL, NULL, hInst, NULL);
		ghdc = GetDC(graphHWND);
		CHECK(SetPixelFormat(ghdc, pixelFormat, &pfdNULL));
	}
	{
		TCHAR szSettingsClass[] = TEXT("EdSettingsClass");
		initClass(hInst, SettingsWndProc, szSettingsClass, false);
		settingsHWND = CreateWindow(szSettingsClass, TEXT("Settings"),
			WS_CHILD | WS_VISIBLE,
			630 / 2, 630 / 2, 630 / 2, 630 / 2,
			mainW, NULL, hInst, NULL);
	}
	return TRUE;
}