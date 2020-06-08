#include "Ray.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <random>
#include <iostream>

#define GLEW_STATIC
#include <GL\glew.h>

#include "Shader.h"
#include "Matrix.h"
#include "Camera.h"
#include "Lens.h"
#include "Plane.h"

static const int MAX_IMAGE_H = 2048;
static const int MAX_IMAGE_W = 2048;
static const int MAX_IMAGE_BUF = MAX_IMAGE_H * MAX_IMAGE_W;
static const int MAX_RAYS = 500 * 500;
static const int NUM_DOTS = 8;

float Ray::pSize = 1.0f, Ray::pIntensity = 0.5f;

double Ray::FA, Ray::SA, Ray::width, Ray::Dist, 
Ray::Smile, Ray::SmileShift, Ray::SmilePlateau,
Ray::Rx, Ray::Ry, Ray::Rz;
int Ray::fStep, Ray::sStep, Ray::Num;
Ray::MODE Ray::Mode;

static GLfloat modelMat[16];
static GLuint vaoId, bufferId;
static GLuint feedbackVaoId, feedbackBufferId, feedbackQuery, feedbackPrimitives;
static GLuint fboId, textureId;
static GLuint vaoIdProfileI, bufferIdProfileI, vaoIdProfileJ, bufferIdProfileJ;
static int lines;
static int camPixX, camPixY;
static bool FWe2 = false, WidthAtMax = false, WidthAsFWHM = false;
static float FWlevel = 0.5;
static int textureBit = 3;
static float* imageBuf;
static float* profileI, * profileJ;

static void genFrameBuffer();

void Ray::generate(
	double cWidth, double cFA, double cSA, int cfStep, int csStep,
	MODE cMode, int cPx, int cPy,
	bool cFWe2, bool cWidthAtMax, bool cWidthAsFWHM, float cFWlevel) {
	camPixX = cPx; camPixY = cPy; FWe2 = cFWe2; WidthAtMax = cWidthAtMax; WidthAsFWHM = cWidthAsFWHM;
	FWlevel = cFWlevel;
	if (camPixX * camPixY > MAX_IMAGE_BUF) camPixX = MAX_IMAGE_BUF / camPixY;
	imageBuf = new float[MAX_IMAGE_BUF];
	profileI = new float[2 * MAX_IMAGE_W];
	profileJ = new float[2 * MAX_IMAGE_H];

	int attribFormat = 3;			//Buffer for draw rays
	Shader::createBuffer(GL_STATIC_DRAW, &vaoId, &bufferId, 0, MAX_RAYS * 3 * 2 * sizeof(float), &attribFormat, 1);
	int feedbackAttribFormat = 4;	//Feedback buffer for extract calculations from vertex shader
	Shader::createBuffer(GL_DYNAMIC_DRAW, &feedbackVaoId, &feedbackBufferId, 0,
		MAX_RAYS * 4 * (NUM_DOTS * 2 - 2) * sizeof(float), &feedbackAttribFormat, 1, 4 * ((NUM_DOTS * 2 - 2) - 1));
	glGenQueries(1, &feedbackQuery);//Query for count feedback's primitives
	genFrameBuffer();				//Framebuffer for intensity distibution

	Smile = 0.0;
	SmileShift = 0.0;
	SmilePlateau = 0.0;

	Rx = 0.0; Ry = 0.0; Rz = 0.0;
	FA = cFA; SA = cSA; fStep = cfStep, sStep = csStep, Mode = cMode, width = cWidth; Num = 1; Dist = 0.0;
	changeRays();
	Matrix::setIdentityM(modelMat);

	int attribFormatProfile = 2;	//Buffer for profiles
	Shader::createBuffer(GL_STREAM_DRAW, &vaoIdProfileI, &bufferIdProfileI, 0, MAX_IMAGE_W * attribFormatProfile * sizeof(float), &attribFormatProfile, 1);
	Shader::createBuffer(GL_STREAM_DRAW, &vaoIdProfileJ, &bufferIdProfileJ, 0, MAX_IMAGE_H * attribFormatProfile * sizeof(float), &attribFormatProfile, 1);
}

void Ray::drawOnScene() {
	glUseProgram(Shader::Line::programHandle);

	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, feedbackQuery);
	glBeginTransformFeedback(GL_LINES);

	glBindVertexArray(vaoId);
	glEnableVertexAttribArray(Shader::Line::Input::Position);

	Matrix::multiplyMM(Camera::mMVPMat, Camera::mVPMat, modelMat);
	glUniformMatrix4fv(Shader::Line::Uniform::uMatHandle, 1, GL_FALSE, Camera::mMVPMat);

	glLineWidth(1.0f);
	glDrawArrays(GL_LINES, 0, lines * 2);

	glDisableVertexAttribArray(Shader::Line::Input::Position);
	glBindVertexArray(0);

	glEndTransformFeedback();
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	glGetQueryObjectuiv(feedbackQuery, GL_QUERY_RESULT, &feedbackPrimitives);
}

void Ray::drawOnTexture() {
	glViewport(0, 0, camPixX, camPixY);
	glUseProgram(Shader::Point::programHandle);

	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(feedbackVaoId);
	glEnableVertexAttribArray(Shader::Point::Input::Position);

	Matrix::multiplyMM(Camera::mMVPMat, Camera::mVPMat, modelMat);
	glUniformMatrix4fv(Shader::Point::Uniform::uMatHandle, 1, GL_FALSE, Camera::mMVPMat);
	glUniform4f(Shader::Point::Uniform::uColorHandle, pIntensity, 0, 0, 1);

	glPointSize(pSize);
	glDrawArrays(GL_POINTS, 0, feedbackPrimitives / (NUM_DOTS - 1));

	glDisableVertexAttribArray(Shader::Point::Input::Position);
	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Ray::drawProfile() {
	glUseProgram(Shader::Graph::programHandle);
	//Matrix::multiplyMM(Camera::mMVPMat, Camera::mVPMat, modelMat);
	Matrix::setIdentityM(modelMat);
	glUniformMatrix4fv(Shader::Graph::Uniform::uMatHandle, 1, GL_FALSE, modelMat);
	glLineWidth(3.0f);

	glBindVertexArray(vaoIdProfileI);
	glEnableVertexAttribArray(Shader::Graph::Input::Position);
	glUniform4f(Shader::Graph::Uniform::uColor, 0.0f, 1.0f, 0.0f, 1.0f);
	glDrawArrays(GL_LINE_STRIP, 0, camPixX);
	glDisableVertexAttribArray(Shader::Graph::Input::Position);

	glBindVertexArray(vaoIdProfileJ);
	glEnableVertexAttribArray(Shader::Graph::Input::Position);
	glUniform4f(Shader::Graph::Uniform::uColor, 1.0f, 1.0f, 0.0f, 1.0f);
	glDrawArrays(GL_LINE_STRIP, 0, camPixY);
	glDisableVertexAttribArray(Shader::Graph::Input::Position);

	glBindVertexArray(0);
}

static inline float* fillAxis1D(float* axis, int size, float lim) {
	for (int i = 0; i < size / 2; ++i) {
		float val = (size / 2.0f - (float)i) * lim;
		axis[i] = val;
		axis[size - i - 1] = -val;
	}
	if (size % 2) axis[size / 2] = 0.0f;
	return axis;
}

float RotMat[16];
static inline void fillAxis2D(float* axis, int pos, float f, float s, float zOffset) {
	float len = sqrt(1.0f + f * f + s * s);

	//float p = rand()*2.0*M_PI / RAND_MAX;
	//float r = rand()*width / RAND_MAX - width / 2.0f;
	
	float z = rand() * Ray::width / RAND_MAX - Ray::width / 2.0f + zOffset;
	Vector3 vc = Vector3(0.0, std::max(0.0, Ray::Smile * abs(z + Ray::SmileShift) - Ray::SmilePlateau), z) * RotMat;
	axis[pos * 3 * 2 + 0] = vc.x;
	axis[pos * 3 * 2 + 1] = vc.y;//r * r * sin(p); //rand()*width / RAND_MAX - width / 2.0f;// 0.0;
	axis[pos * 3 * 2 + 2] = vc.z;//r * r * cos(p); 

	Vector3 vr = Vector3(1.0 / len, f / len, s / len) * RotMat;
	axis[pos * 3 * 2 + 3] = vr.x;
	axis[pos * 3 * 2 + 4] = vr.y;
	axis[pos * 3 * 2 + 5] = vr.z;

}
static inline void fillAxis2Dangle(float* axis, int pos, float f, float s, float zOffset) {
	float cf = cos(f);
	float cs = cos(s);

	//float p = rand()*2.0*M_PI / RAND_MAX;
	//float r = rand()*width / RAND_MAX - width / 2.0f;

	axis[pos * 3 * 2 + 0] = 0.0;
	axis[pos * 3 * 2 + 1] = 0.0;// r * r * sin(p); //rand()*width / RAND_MAX - width / 2.0f;// 0.0;
	axis[pos * 3 * 2 + 2] = rand() * Ray::width / RAND_MAX - Ray::width / 2.0f + zOffset;//r * r * cos(p); 
	axis[pos * 3 * 2 + 3] = sqrt(1.0f - cf * cf - cs * cs);
	axis[pos * 3 * 2 + 4] = cf;
	axis[pos * 3 * 2 + 5] = cs;
}


void Ray::changeRays() {
	float* cA = 0;
	if (FA <= 0 || SA <= 0 || fStep <= 0 || sStep <= 0 || Num <= 0)
		lines = 0;
	else {
		float x0 = -Dist * (Num / 2) - Dist / 2 * (Num % 2 - 1);
		srand(0);
		float fLim = abs(tan(FA / 2.0 * M_PI / 180.0));
		float sLim = abs(tan(SA / 2.0 * M_PI / 180.0));
		lines = fStep * sStep;
		if (lines > MAX_RAYS) {
			lines = MAX_RAYS;
			fStep = MAX_RAYS / sStep; //FIXME Can be 0 if sStep > MAX_RAYS
		}
		cA = new float[lines * 2 * 3];

		Matrix::setIdentityM(RotMat);
		Matrix::rotateM(RotMat, Rx, Axis::X);
		Matrix::rotateM(RotMat, Ry, Axis::Y);
		Matrix::rotateM(RotMat, Rz, Axis::Z);

		switch (Mode) {
		case Rect: {
			float* fm = fillAxis1D(new float[fStep], fStep, 2.0f * fLim / (float)fStep);
			float* sm = fillAxis1D(new float[sStep], sStep, 2.0f * sLim / (float)sStep);
			for (int i = 0; i < sStep; ++i)
				for (int j = 0; j < fStep; ++j)
					fillAxis2D(cA, i * fStep + j, fm[j], sm[i], x0 + Dist * ((i * fStep + j) % Num));
			delete[]  fm, sm;
		}
				 break;
		case Gauss: {
			float fGamma = fLim / (FWe2 ? 2 : sqrt(2.0 * log(2.0)));
			float sGamma = sLim / (FWe2 ? 2 : sqrt(2.0 * log(2.0)));
			std::default_random_engine generator;
			std::normal_distribution<float> distributionFast(0.0f, fGamma);
			std::normal_distribution<float> distributionSlow(0.0f, sGamma);
			for (int i = 0; i < lines; ++i)
				fillAxis2D(cA, i, distributionFast(generator), distributionSlow(generator), x0 + Dist * (i % Num));
		}
				  break;
		case Ellipse: {
			float* fm = fillAxis1D(new float[fStep], fStep, 2.0f * fLim / (float)fStep);
			float* sm = fillAxis1D(new float[sStep], sStep, M_PI / (float)sStep);
			for (int i = 0; i < sStep; ++i)
				for (int j = 0; j < fStep; ++j)
					fillAxis2D(cA, i * fStep + j, cos(sm[i]) * fm[j], sin(sm[i]) * fm[j] * SA / FA, x0 + Dist * ((i * fStep + j) % Num));
			delete[]  fm, sm;
		}
					break;
		case Rect_S: {
			float* fm = fillAxis1D(new float[fStep], fStep, (FA / 2.0 * M_PI / 180.0) / (float)fStep);
			float* sm = fillAxis1D(new float[sStep], sStep, (SA / 2.0 * M_PI / 180.0) / (float)sStep);
			for (int i = 0; i < sStep; ++i)
				for (int j = 0; j < fStep; ++j)
					fillAxis2Dangle(cA, i * fStep + j, fm[j], sm[i], x0 + Dist * ((i * fStep + j) % Num));
			delete[]  fm, sm;
		}
				   break;
		default:
			break;
		}
	}

	glBindVertexArray(vaoId);
	glBindBuffer(GL_ARRAY_BUFFER, bufferId);
	glBufferSubData(GL_ARRAY_BUFFER, 0, lines * 3 * 2 * sizeof(float), cA);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	if (cA != nullptr) delete[] cA;
}

void Ray::changeTextureFormat(int tFormat) {
	textureBit = tFormat;
	glDeleteTextures(1, &textureId);
	glDeleteFramebuffers(1, &fboId);

	genFrameBuffer();
}
void Ray::changeSensorResolutionW(int cPx) {
	camPixX = cPx;
	if (camPixX > MAX_IMAGE_W) camPixX = MAX_IMAGE_W;
	//if (camPixX*camPixY > MAX_IMAGE_BUF) camPixX = MAX_IMAGE_BUF / camPixY;

	glDeleteTextures(1, &textureId);
	glDeleteFramebuffers(1, &fboId);

	genFrameBuffer();
}
void Ray::changeSensorResolutionH(int cPy) {
	camPixY = cPy;
	if (camPixY > MAX_IMAGE_H) camPixY = MAX_IMAGE_H;
	//if (camPixX*camPixY > MAX_IMAGE_BUF) camPixY = MAX_IMAGE_BUF / camPixX;

	glDeleteTextures(1, &textureId);
	glDeleteFramebuffers(1, &fboId);

	genFrameBuffer();
}
void genFrameBuffer() {
	glGenTextures(1, &textureId);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureId);

	switch (textureBit) {
	case 0: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, camPixX, camPixY, 0, GL_RGBA, GL_FLOAT, 0); break; //FIXME check GL_RED instead  GL_RGBA
	case 1: glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, camPixX, camPixY, 0, GL_RED, GL_FLOAT, 0); break;
	case 2: glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, camPixX, camPixY, 0, GL_RED, GL_FLOAT, 0); break;
	case 3: glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, camPixX, camPixY, 0, GL_RED, GL_FLOAT, 0); break;
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenFramebuffers(1, &fboId);
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/*
float widthAtLevel(float* arr, int length, float level, float maxValue, float maxWidth) {
	int leftWx = 0, rightWx = maxWidth;
	for (int i1 = 0; i1 < length; ++i1)
		if (arr[i1] > maxValue * level) {
			leftWx = i1;
			break;
		}
	for (int i1 = length - 1; i1 > 0; --i1)
		if (arr[i1] > maxValue * level) {
			rightWx = i1;
			break;
		}
	return abs(leftWx - rightWx);
}*/

void Ray::getProfile(double* cWx, double* cWy) {
	memset(profileI, 0, sizeof(float) * 2 * MAX_IMAGE_W);
	memset(profileJ, 0, sizeof(float) * 2 * MAX_IMAGE_H);
	for (int i = 0; i < camPixX; ++i)
		for (int j = 0; j < camPixY; ++j) {
			profileI[2 * i + 1] += abs(imageBuf[i + camPixX * j]);
			profileJ[2 * j] += abs(imageBuf[i + camPixX * j]);
		}

	double mpi = -INFINITY;
	double mpj = -INFINITY;
	for (int i1 = 0; i1 < camPixX; ++i1)
		if (profileI[2 * i1 + 1] > mpi)
			mpi = profileI[2 * i1 + 1];
	for (int j1 = 0; j1 < camPixY; ++j1)
		if (profileJ[2 * j1] > mpj)
			mpj = profileJ[2 * j1];

	if (cWx != 0 && cWy != 0) {
		float wx = 0, wy = 0;
		{
			int leftWx = 0, rightWx = camPixX;
			for (int i1 = 0; i1 < camPixX; ++i1)
				if (profileI[2 * i1 + 1] > mpi* FWlevel) {
					leftWx = i1;
					break;
				}
			for (int i1 = camPixX - 1; i1 > 0; --i1)
				if (profileI[2 * i1 + 1] > mpi* FWlevel) {
					rightWx = i1;
					break;
				}
			wx = abs(leftWx - rightWx);
		}
		{
			int leftWy = 0, rightWy = camPixY;
			for (int j1 = 0; j1 < camPixY; ++j1)
				if (profileJ[2 * j1] > mpj* FWlevel) {
					leftWy = j1;
					break;
				}
			for (int j1 = camPixY - 1; j1 > 0; --j1)
				if (profileJ[2 * j1] > mpj* FWlevel) {
					rightWy = j1;
					break;
				}
			wy = abs(leftWy - rightWy);
		}
		*cWx = Plane::w * wx / camPixX;
		*cWy = Plane::h * wy / camPixY;
	}

	for (int i1 = 0; i1 < camPixX; ++i1) {
		profileI[2 * i1] = 2.0f * i1 / camPixX - 1.0f;
		profileI[2 * i1 + 1] /= 4.0f * mpi / 2.0f;
		profileI[2 * i1 + 1] -= 1.0f;
	}
	for (int j1 = 0; j1 < camPixY; ++j1) {
		profileJ[2 * j1 + 1] = 2.0f * j1 / camPixY - 1.0f;
		profileJ[2 * j1] /= 4.0f * mpj / 2.0f;
		profileJ[2 * j1] -= 1.0f;
	}

	glBindVertexArray(vaoIdProfileI);
	glBindBuffer(GL_ARRAY_BUFFER, bufferIdProfileI);
	glBufferSubData(GL_ARRAY_BUFFER, 0, camPixX * 2 * sizeof(float), profileI);
	glBindVertexArray(vaoIdProfileJ);
	glBindBuffer(GL_ARRAY_BUFFER, bufferIdProfileJ);
	glBufferSubData(GL_ARRAY_BUFFER, 0, camPixY * 2 * sizeof(float), profileJ);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Ray::getParams(double* cI, double* cCx, double* cCy, double* cWx, double* cWy) {
	if (WidthAsFWHM) getFWHM(cI, cCx, cCy, cWx, cWy);
	else getWxWy(cI, cCx, cCy, cWx, cWy);
}
void Ray::getWxWy(double* cI, double* cCx, double* cCy, double* cWx, double* cWy) {
	glActiveTexture(GL_TEXTURE0);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, imageBuf);
	double mnI = 0;
	for (int i = 0; i < camPixX * camPixY; ++i)
		mnI += imageBuf[i];

	double mi = 0, mj = 0;

	if (WidthAtMax) {	//Max Intensity
		double maxI = -INFINITY;
		for (int i1 = 0; i1 < camPixX; ++i1)
			for (int j1 = 0; j1 < camPixY; ++j1) {
				double curI = imageBuf[i1 + camPixX * j1];
				if (curI > maxI) {
					maxI = curI;
					mi = i1;
					mj = j1;
				}
			}
	}
	else {	//Moment
		for (int i1 = 0; i1 < camPixX; ++i1)
			for (int j1 = 0; j1 < camPixY; ++j1) {
				mi = mi + i1 * imageBuf[i1 + camPixX * j1];
				mj = mj + j1 * imageBuf[i1 + camPixX * j1];
			}
		mi = mi / mnI;
		mj = mj / mnI;
	}

	double wx = 0;
	double wy = 0;
	for (int i2 = 0; i2 < camPixX; ++i2)
		for (int j2 = 0; j2 < camPixY; ++j2) {
			wx = wx + (i2 - mi) * (i2 - mi) * imageBuf[i2 + camPixX * j2];
			wy = wy + (j2 - mj) * (j2 - mj) * imageBuf[i2 + camPixX * j2];
		}
	wx = sqrt(wx * 4 / mnI);
	wy = sqrt(wy * 4 / mnI);

	if (cI != 0) *cI = mnI;
	if (cCx != 0) *cCx = Plane::w * mi / camPixX;
	if (cCy != 0) *cCy = Plane::h * mj / camPixY;
	if (cWx != 0) *cWx = Plane::w * wx / camPixX;
	if (cWy != 0) *cWy = Plane::h * wy / camPixY;
}
void Ray::getFWHM(double* cI, double* cCx, double* cCy, double* cWx, double* cWy) {
	glActiveTexture(GL_TEXTURE0);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, imageBuf);
	double mnI = 0;
	for (int i = 0; i < camPixX * camPixY; ++i)
		mnI += imageBuf[i];

	double maxIntensity = -INFINITY;
	int mi = 0, mj = 0;
	for (int i1 = 0; i1 < camPixX; ++i1)
		for (int j1 = 0; j1 < camPixY; ++j1) {
			if (imageBuf[i1 + camPixX * j1] > maxIntensity) {
				maxIntensity = imageBuf[i1 + camPixX * j1];
				mi = i1;
				mj = j1;
			}
		}

	double wx = 0, wy = 0;
	{
		int leftWx = 0, rightWx = camPixX;
		for (int i1 = 0; i1 < camPixX; ++i1)
			if (imageBuf[i1 + camPixX * mj] > maxIntensity* FWlevel) {
				leftWx = i1;
				break;
			}
		for (int i1 = camPixX - 1; i1 > 0; --i1)
			if (imageBuf[i1 + camPixX * mj] > maxIntensity* FWlevel) {
				rightWx = i1;
				break;
			}
		wx = abs(leftWx - rightWx);
	}
	{
		int leftWy = 0, rightWy = camPixY;
		for (int j1 = 0; j1 < camPixY; ++j1)
			if (imageBuf[mi + camPixX * j1] > maxIntensity* FWlevel) {
				leftWy = j1;
				break;
			}
		for (int j1 = camPixY - 1; j1 > 0; --j1)
			if (imageBuf[mi + camPixX * j1] > maxIntensity* FWlevel) {
				rightWy = j1;
				break;
			}
		wy = abs(leftWy - rightWy);
	}

	if (cI != 0) *cI = mnI;
	if (cCx != 0) *cCx = Plane::w * mi / camPixX;
	if (cCy != 0) *cCy = Plane::h * mj / camPixY;
	if (cWx != 0) *cWx = Plane::w * wx / camPixX;
	if (cWy != 0) *cWy = Plane::h * wy / camPixY;
}

void Ray::release() {
	delete[] imageBuf;
	delete[] profileI, profileJ;
	glDeleteVertexArrays(1, &vaoId);
	glDeleteVertexArrays(1, &feedbackVaoId);
	glDeleteVertexArrays(1, &vaoIdProfileI);
	glDeleteVertexArrays(1, &vaoIdProfileJ);
	glDeleteBuffers(1, &bufferId);
	glDeleteBuffers(1, &bufferIdProfileI);
	glDeleteBuffers(1, &bufferIdProfileJ);
	glDeleteBuffers(1, &feedbackBufferId);
	glDeleteQueries(1, &feedbackQuery);
	glDeleteTextures(1, &textureId);
	glDeleteFramebuffers(1, &fboId);
}