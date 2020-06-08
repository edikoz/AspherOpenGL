#pragma once

namespace Ray {
	enum MODE {Rect, Gauss, Ellipse, Rect_S, Gauss_S, Ellipse_S};
	extern float pSize, pIntensity;
	extern double FA, SA, width, Dist, 
		Smile, SmileShift, SmilePlateau,
		Rx, Ry, Rz;
	extern int fStep, sStep, Num;
	extern MODE Mode;

	void generate(double cWidth, double FA, double SA, int fStep, int sStep, MODE Mode, int cPx, int cPy, bool FWe2, bool WidthAtMax, bool cWidthAsFWHM, float FWlevel);
	void drawOnScene();
	void drawOnTexture();
	void drawProfile();
	void getProfile(double* cWx, double* cWy);
	void getParams(double *cI, double *cCx, double *cCy, double *cWx, double *cWy);
	void getWxWy(double *cI, double *cCx, double *cCy, double *cWx, double *cWy);
	void getFWHM(double *cI, double *cCx, double *cCy, double *cWx, double *cWy);
	//float widthAtLevel(float *arr, int length, float level, float maxValue, float maxWidth);
	void changeTextureFormat(int tFormat);
	void changeSensorResolutionW(int w);
	void changeSensorResolutionH(int h);
	void changeRays();
	void release();
};