#pragma once

#include "Matrix.h"

namespace Lens {
	void generate(float x, float  y, float z, float d, float  h, float w, float n, float *fc,
		float br, float bp, float bw, float bg);
	void draw();
	void rotate(float cRot, int axisID);
	void rotateTo(float cRot, int axisID);
	void move(float dx, float dy, float dz);
	void moveToX(float dx);
	void moveToY(float dy);
	void moveToZ(float dz);
	void setAxis(int axisID, Axis cAxis);
	float getRots(int index);
	float getX();
	float getY();
	float getZ();
	float getBoundX();
	void release();
};