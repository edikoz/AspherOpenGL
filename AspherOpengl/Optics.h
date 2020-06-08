#pragma once

namespace Optics {
	void generate(float x, float r, float d, float l, float ang);
	void draw();
	void setParams(float x, float r, float d, float l, float ang, float *outDx, float *outDy);
	void release();
};