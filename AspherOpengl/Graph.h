#pragma once

#include <tchar.h>

namespace Graph {
	extern float w, h;
	extern bool line, point, grid, ticks, cursors;

	void generate();
	void draw();
	void addPoint(float x, float y1, float y2, float y3, float y4);
	void setXlabel(const TCHAR *label);
	void generateTicks();
	void drawTicks();
	void drawCursors(float x, float y);
	void getScreenShot(unsigned char *imageBuf);
	void clear();
	void release();
};