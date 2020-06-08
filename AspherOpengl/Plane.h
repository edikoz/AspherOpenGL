#pragma once

namespace Plane {
	extern float w, h;

	void generate(float cw, float ch);
	void draw();
	void release();
	void moveX(float dx);
	void moveToX(float dx);
	void moveY(float dy);
	void moveToY(float dy);
	void setH(float h);
	void setW(float w);
	float getX();
	float getY();
};