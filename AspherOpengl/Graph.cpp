#include "Graph.h"

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <tchar.h>
#include <string>

#define GLEW_STATIC
#include <GL\glew.h>

#include "Shader.h"
#include "Matrix.h"

float Graph::w, Graph::h;
bool Graph::line = true, Graph::point = true, Graph::grid = false, Graph::ticks = true, Graph::cursors = true;

static const int maxPoints = 10000;
static const int numOfGraphs = 4;
static const float color[numOfGraphs][4] = { {1.0f,0.0f,0.0f,1.0f},{ 0.0f,1.0f,0.0f,1.0f },{ 0.0f,0.0f,1.0f,1.0f },{ 0.0f,1.0f,1.0f,1.0f } };
static const float colorGrid[4] = { 0.8f, 0.8f, 0.8f, 1.0f }, colorBlack[4] = { 0.0f, 0.0f, 0.0f, 1.0f }, colorWhite[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
static const TCHAR labels[numOfGraphs][3] = { L"cx", L"cy", L"wx", L"wy" };

static int curPoint = 0;
static GLfloat modelMat[16];

static const int bSize = 128;
static TCHAR xLabel[bSize] = TEXT("x");
static float lastY[numOfGraphs];
static float minX = INFINITY, maxX = -INFINITY, minY = INFINITY, maxY = -INFINITY;

static GLuint textureId;
static GLuint vaoId, bufferId, fVaoId, fBufferId, tVaoId, tBufferId;
static const int uboCount = 100;
static GLuint uboId[uboCount];
static const int charCount = 6;
static const int uboBindingPoint = 1;
static const float charWidthPx = 8.0f, charHeightPx = 8.0f;
static float paddingW, paddingH;
static int ticksCount, vt, ht;

static void fontLoad(std::string fname);
static void fillNumber(float num, float x, float y, int offset);
static void fillText(const TCHAR *text, float x, float y, int offset);

void Graph::generate() {
	fontLoad("Res/Font/font.bmp");

	int attribFormat = 2;
	Shader::createBuffer(GL_STREAM_DRAW, &vaoId, &bufferId, 0, numOfGraphs * maxPoints * attribFormat * sizeof(float), &attribFormat, 1);

	float tmp1[] = {
		0.0f, 0.0f,
		0.0f, -1.0f,
		0.0f, 0.0f,
		-1.0f, 0.0f
	};
	int tmp1AttribFormat = 2;
	Shader::createBuffer(GL_STATIC_DRAW, &tVaoId, &tBufferId, tmp1, sizeof(tmp1), &tmp1AttribFormat, 1);

	float tmp2[] = {
		0.0f,0.0f, 0.0f, 0.0f,
		2.0f,0.0f, 1.0f / 16.0f, 0.0f,
		0.0f,2.0f, 0.0f, 1.0f / 16.0f,
		2.0f,2.0f, 1.0f / 16.0f, 1.0f / 16.0f
	};
	int tmp2AttribFormat[] = { 2, 2 };
	Shader::createBuffer(GL_STATIC_DRAW, &fVaoId, &fBufferId, tmp2, sizeof(tmp2), tmp2AttribFormat, 2);

	GLuint indexBlock = glGetUniformBlockIndex(Shader::Text::programHandle, "u_BlockTranslateText");
	glUniformBlockBinding(Shader::Text::programHandle, indexBlock, uboBindingPoint);

	glGenBuffers(uboCount, uboId);
	for (int i = 0; i < uboCount; ++i) {
		glBindBuffer(GL_UNIFORM_BUFFER, uboId[i]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * (4 * charCount + 4), 0, GL_DYNAMIC_DRAW);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	Matrix::setIdentityM(modelMat);
}

void Graph::draw() {
	glUseProgram(Shader::Graph::programHandle);

	glLineWidth(1.0f);
	glPointSize(5.0f);
	glBindVertexArray(vaoId);
	glUniformMatrix4fv(Shader::Graph::Uniform::uMatHandle, 1, GL_FALSE, modelMat);
	glEnableVertexAttribArray(Shader::Graph::Input::Position);
	for (int i = 0; i < numOfGraphs; ++i) {
		glUniform4f(Shader::Graph::Uniform::uColor, color[i][0], color[i][1], color[i][2], color[i][3]);
		if (point)  glDrawArrays(GL_POINTS, maxPoints*i, curPoint);
		if (curPoint > 1 && line) glDrawArrays(GL_LINE_STRIP, maxPoints*i, curPoint);
	}
	glDisableVertexAttribArray(Shader::Graph::Input::Position);
	glBindVertexArray(0);
}

void fillNumber(float num, float x, float y, int offset) {
	float ar[4 + charCount * 4];
	memset(ar, 0, sizeof(ar));
	ar[0] = x;
	ar[1] = y;
	TCHAR buf[200];//FIXME need 20, else error: buffer too small
	_stprintf_s(buf, TEXT("%.4f"), num);
	int i;
	for (i = 0; i < charCount && buf[i] != '\0'; ++i) {
		ar[i * 4 + 0 + 4] = 1.0f / 16.0f * (buf[i] % 16);
		ar[i * 4 + 1 + 4] = 1.0f / 16.0f * (15 - buf[i] / 16);
	}
	while (i < charCount) {
		ar[i * 4 + 0 + 4] = 1.0f / 16.0f * 0;
		ar[i * 4 + 1 + 4] = 1.0f / 16.0f * 15;
		++i;
	}
	glBindBuffer(GL_UNIFORM_BUFFER, uboId[offset]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ar), ar);
}
void fillText(const TCHAR *buf, float x, float y, int offset) {
	float ar[4 + charCount * 4];
	memset(ar, 0, sizeof(ar));
	ar[0] = x;
	ar[1] = y;
	int i;
	for (i = 0; i < charCount && buf[i] != '\0'; ++i) {
		ar[i * 4 + 0 + 4] = 1.0f / 16.0f * (buf[i] % 16);
		ar[i * 4 + 1 + 4] = 1.0f / 16.0f * (15 - buf[i] / 16);
	}
	while (i < charCount) {
		ar[i * 4 + 0 + 4] = 1.0f / 16.0f * 0;
		ar[i * 4 + 1 + 4] = 1.0f / 16.0f * 15;
		++i;
	}
	glBindBuffer(GL_UNIFORM_BUFFER, uboId[offset]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ar), ar);
}

void Graph::setXlabel(const TCHAR *label) {
	if (label != 0)
		for (int i = 0; i < bSize; ++i)
			if ((xLabel[i] = label[i]) == '\0') break;
}

void Graph::generateTicks() {
	paddingW = 1.0f - 2.0f*(charCount*charWidthPx + 10) / w;
	paddingH = 1.0f - 2.0f*(charHeightPx + 10) / h;
	glUseProgram(Shader::Text::programHandle);
	glUniform2f(Shader::Text::Uniform::uScale, charWidthPx / w, charHeightPx / h);

	ticksCount = 0;
	//Horizontal ticks
	{
		ht = (int)((w - 1.0f*charWidthPx*charCount) / (2.0f * charWidthPx*charCount));
		for (int i = 0; i < ht; ++i) {
			float x = -paddingW + 2.0f*paddingW / (ht - 1) * i;
			fillNumber((maxX - minX)*(x + paddingW) / (2.0f*paddingW) + minX, x, -1.0f, ticksCount++);
		}
	}
	//Vertical ticks
	{
		vt = (int)((h - 1.0f*charHeightPx) / (2.0f * charHeightPx*charCount));
		for (int i = 0; i < vt; ++i) {
			float y = -paddingH + 2.0f*paddingH / (vt - 1) * i;
			fillNumber((maxY - minY)*(y + paddingH) / (2.0f*paddingH) + minY, -1.0f, y, ticksCount++);
		}
	}

	float charHeight = 2.0f*paddingH*charHeightPx / h;
	float lY[numOfGraphs];
	int floatYid[numOfGraphs];
	for (int i = 0; i < numOfGraphs; ++i){
		floatYid[i] = i;
		lY[i] = (2.0f*(lastY[i] - minY) / (maxY - minY) - 1.0f)*paddingH;
	}
	for (int i = 0; i < numOfGraphs; ++i) {
		for (int k = i + 1; k < numOfGraphs; ++k) {
			if (lY[i] + charHeight > lY[k] && lY[i] < lY[k] + charHeight) {
				floatYid[floatYid[i]] = k;
			}
		}
		for (int k = 0; k < numOfGraphs; ++k) {
			if (floatYid[k] != k) lY[floatYid[k]] = lY[k] - charHeight;
		}
	}

	for (int i = 0; i < numOfGraphs; ++i)
		fillText(labels[i], 1.0f - 0.5f*(1.0f - paddingW), lY[i], uboCount - i - 1);
	fillText(xLabel, 0.0f, 2.0f*(1.0f - paddingH) - 1.0f, uboCount - 5);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	Matrix::setIdentityM(modelMat);
	if (curPoint > 1) {
		float gWidth = (maxX - minX), gHeight = (maxY - minY);
		Matrix::scaleM(modelMat, paddingW, paddingH, 1.0f);
		Matrix::scaleM(modelMat, 2.0f / gWidth, 2.0f / gHeight, 1.0f);
		Matrix::translateM(modelMat, -gWidth / 2.0f - minX, -gHeight / 2.0f - minY, 0.0f);
	}
}

void drawColoredTicksV(int count, float sw, float sh, float gw, float gh, const float *color = 0) {
	if (color) glUniform4fv(Shader::Ticks::Uniform::uColor, 1, color);
	glUniform2f(Shader::Ticks::Uniform::uScale, sw, sh);
	glUniform2f(Shader::Ticks::Uniform::uGap, gw, gh);
	glDrawArraysInstanced(GL_LINES, 2, 2, count);
}
void drawColoredTicksH(int count, float sw, float sh, float gw, float gh, const float *color = 0) {
	if (color) glUniform4fv(Shader::Ticks::Uniform::uColor, 1, color);
	glUniform2f(Shader::Ticks::Uniform::uScale, sw, sh);
	glUniform2f(Shader::Ticks::Uniform::uGap, gw, gh);
	glDrawArraysInstanced(GL_LINES, 0, 2, count);
}

void Graph::drawTicks() {
	glUseProgram(Shader::Text::programHandle);
	glBindVertexArray(fVaoId);
	glEnableVertexAttribArray(Shader::Text::Input::Position);
	glEnableVertexAttribArray(Shader::Text::Input::TextureCoord);
	glUniform1i(Shader::Text::Uniform::uFont, 1);
	glUniform4fv(Shader::Text::Uniform::uColor, 1, colorBlack);

	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	//ticks's text
	for (int i = 0; i < ticksCount; ++i) {
		glBindBufferBase(GL_UNIFORM_BUFFER, uboBindingPoint, uboId[i]);
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, charCount);
	}
	//plot's labels
	for (int i = 0; i < numOfGraphs; ++i) {
		glUniform4fv(Shader::Text::Uniform::uColor, 1, color[i]);
		glBindBufferBase(GL_UNIFORM_BUFFER, uboBindingPoint, uboId[uboCount - i - 1]);
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, charCount);
	}
	//plot's Ox label
	glUniform4f(Shader::Text::Uniform::uColor, 1.0f, 0.0f, 0.0f, 1.0f);
	glBindBufferBase(GL_UNIFORM_BUFFER, uboBindingPoint, uboId[uboCount - numOfGraphs - 1]);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, charCount);
	glDisable(GL_BLEND);

	glDisableVertexAttribArray(Shader::Text::Input::TextureCoord);
	glDisableVertexAttribArray(Shader::Text::Input::Position);

	glUseProgram(Shader::Ticks::programHandle);
	glLineWidth(1.0f);
	glBindVertexArray(tVaoId);
	glEnableVertexAttribArray(Shader::Ticks::Input::Position);

	if (ht > 0) {
		glUniform2f(Shader::Ticks::Uniform::uTranslate, -paddingW, -paddingH);
		if (grid)
			drawColoredTicksH(ht, 0.0f, -2.0f*paddingH, 2.0f*paddingW / (ht - 1), 0.0f, colorGrid);
		if (ticks) {
			drawColoredTicksH(ht, 0.0f, 2.0f*10.0f / h, 2.0f*paddingW / (ht - 1), 0.0f, colorBlack);
			drawColoredTicksH(ht * 10 - 9, 0.0f, 2.0f*5.0f / h, (2.0f*paddingW / (ht - 1)) / 10.0f, 0.0f);
			drawColoredTicksH(2, 0.0f, -2.0f*paddingH, 2.0f*paddingW, 0.0f);
		}
	}

	if (vt > 0) {
		glUniform2f(Shader::Ticks::Uniform::uTranslate, -paddingW, -paddingH);
		if (grid)
			drawColoredTicksV(vt, -2.0f*paddingW, 0.0f, 0.0f, 2.0f*paddingH / (vt - 1), colorGrid);
		if (ticks) {
			drawColoredTicksV(vt, 2.0f*10.0f / w, 0.0f, 0.0f, 2.0f*paddingH / (vt - 1), colorBlack);
			drawColoredTicksV(vt * 10 - 9, 2.0f*5.0f / w, 0.0f, 0.0f, (2.0f*paddingH / (vt - 1)) / 10.0f);
			drawColoredTicksV(2, -2.0f*paddingW, 0.0f, 0.0f, 2.0f*paddingH);
		}
	}
	glDisableVertexAttribArray(Shader::Ticks::Input::Position);

	glBindVertexArray(0);
}

void Graph::drawCursors(float mx, float my) {
	glBindBuffer(GL_ARRAY_BUFFER, bufferId);
	int nx = curPoint * ((mx * (w + 2*(charCount*charWidthPx + 10)) / w - charCount*charWidthPx - 10) / w);
	if (nx >= curPoint) nx = curPoint - 1;
	if (nx < 0) nx = 0;
	float bx[numOfGraphs], by[numOfGraphs];
	for (int i = 0; i < numOfGraphs; ++i) {
		float* xy = (float*)glMapBufferRange(GL_ARRAY_BUFFER, (maxPoints * i + nx) * 2 * sizeof(float), 2 * sizeof(float), GL_MAP_READ_BIT);
		if (xy) {
			bx[i] = xy[0]; by[i] = xy[1];
		}
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	float minDist = INFINITY;
	int minId = 0;
	float cy = (((h - my)*(h + 2 * (charHeightPx + 10)) / h - charHeightPx - 10) / h) * (maxY - minY) + minY;
	for (int i = 0; i < numOfGraphs; ++i) {
		float dist = abs(cy - by[i]);
		if (dist < minDist) {
			minId = i;
			minDist = dist;
		}
	}
	float underX = bx[minId], underY = by[minId];
	float tx = ((underX - minX) / (maxX - minX) * 2 - 1) * paddingW;
	float ty = ((underY - minY) / (maxY - minY) * 2 - 1) * paddingH;

	glUseProgram(Shader::Ticks::programHandle);
	glPointSize(5.0f);
	glBindVertexArray(tVaoId);
	glEnableVertexAttribArray(Shader::Ticks::Input::Position);
	glUniform2f(Shader::Ticks::Uniform::uTranslate, tx, ty);
	glUniform4fv(Shader::Ticks::Uniform::uColor, 1, colorBlack);
	glUniform2f(Shader::Ticks::Uniform::uScale, 1, 1);
	glUniform2f(Shader::Ticks::Uniform::uGap, 1, 1);
	glDrawArrays(GL_POINTS, 0, 1);
	glDisableVertexAttribArray(Shader::Ticks::Input::Position);

	glUseProgram(Shader::Text::programHandle);
	glBindVertexArray(fVaoId);
	glEnableVertexAttribArray(Shader::Text::Input::Position);
	glEnableVertexAttribArray(Shader::Text::Input::TextureCoord);
	glUniform1i(Shader::Text::Uniform::uFont, 1);
	glUniform4fv(Shader::Text::Uniform::uColor, 1, colorWhite);
	fillNumber(underX, tx, ty, uboCount - numOfGraphs - 2);
	glBindBufferBase(GL_UNIFORM_BUFFER, uboBindingPoint, uboId[uboCount - numOfGraphs - 2]);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, charCount);
	fillNumber(underY, tx, ty - charHeightPx / h * 2, uboCount - numOfGraphs - 3);
	glBindBufferBase(GL_UNIFORM_BUFFER, uboBindingPoint, uboId[uboCount - numOfGraphs - 3]);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, charCount);
	glDisableVertexAttribArray(Shader::Text::Input::TextureCoord);
	glDisableVertexAttribArray(Shader::Text::Input::Position);

	glBindVertexArray(0);
}

void Graph::addPoint(float x, float y1, float y2, float y3, float y4) {
	float subData[2];
	if (x < minX) minX = x;
	if (x > maxX) maxX = x;
	if (y1 < minY) minY = y1;
	if (y1 > maxY) maxY = y1;
	if (y2 < minY) minY = y2;
	if (y2 > maxY) maxY = y2;
	if (y3 < minY) minY = y3;
	if (y3 > maxY) maxY = y3;
	if (y4 < minY) minY = y4;
	if (y4 > maxY) maxY = y4;
	Matrix::setIdentityM(modelMat);
	if (curPoint > 1) {
		float gWidth = (maxX - minX), gHeight = (maxY - minY);
		Matrix::scaleM(modelMat, paddingW, paddingH, 1.0f);
		Matrix::scaleM(modelMat, 2.0f / gWidth, 2.0f / gHeight, 1.0f);
		Matrix::translateM(modelMat, -gWidth / 2.0f - minX, -gHeight / 2.0f - minY, 0.0f);
	}


	glBindVertexArray(vaoId);
	subData[0] = x;
	for (int i = 0; i < numOfGraphs; ++i) {
		switch (i) {
		case 0: lastY[i] = subData[1] = y1; break;
		case 1: lastY[i] = subData[1] = y2; break;
		case 2: lastY[i] = subData[1] = y3; break;
		case 3: lastY[i] = subData[1] = y4; break;
		}
		glBindBuffer(GL_ARRAY_BUFFER, bufferId);
		glBufferSubData(GL_ARRAY_BUFFER, 2 * sizeof(float)*(maxPoints*i + curPoint), 2 * sizeof(float), subData);
	}
	glBindVertexArray(0);

	++curPoint;
	if (curPoint >= maxPoints) curPoint = 0;
}

void Graph::getScreenShot(unsigned char *imageBuf) {
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, w, h, GL_BGR, GL_UNSIGNED_BYTE, imageBuf);
	glReadBuffer(GL_BACK);
}

void fontLoad(std::string fname) {
	std::ifstream fontFile(fname, std::ios::in | std::ios_base::beg | std::ifstream::binary);
	if (fontFile.is_open())
	{
		char header[54];
		fontFile.read(header, 54);

		unsigned int dataPos = *(int*)&(header[0x0A]);
		unsigned int imageSize = *(int*)&(header[0x22]);
		unsigned int width = *(int*)&(header[0x12]);
		unsigned int height = *(int*)&(header[0x16]);
		if (imageSize == 0) imageSize = width*height * 3;
		if (dataPos == 0) dataPos = 54;
		char *data = new char[imageSize];

		fontFile.read(data, imageSize);

		glActiveTexture(GL_TEXTURE1);
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		fontFile.close();
		std::cout << fname << " Texture OK" << std::endl;
	}
}

void Graph::clear() {
	curPoint = 0;
	minX = minY = INFINITY;
	maxX = maxY = -INFINITY;
}

void Graph::release() {
	glDeleteVertexArrays(1, &vaoId);
	glDeleteBuffers(1, &bufferId);
	glDeleteVertexArrays(1, &fVaoId);
	glDeleteBuffers(1, &fBufferId);
	glDeleteVertexArrays(1, &tVaoId);
	glDeleteBuffers(1, &tBufferId);
	glDeleteTextures(1, &textureId);
	glDeleteBuffers(uboCount, uboId);
}