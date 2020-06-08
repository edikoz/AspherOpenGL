#include "Lens.h"

#include <cmath>
#include <iostream>

#define GLEW_STATIC
#include <GL\glew.h>

#include "Shader.h"
#include "Matrix.h"
#include "Camera.h"
#include "Ray.h"
#include "Plane.h"

static float rots[] = { 0.0f, 0.0f, 0.0f };
static Axis axis[] = { Axis::X, Axis::Y, Axis::Z };

static GLfloat modelMat[16], lens1Mat[16], lens2Mat[16], lens3Mat[16], lens4Mat[16];
static GLuint bufferId, vaoId;
static GLuint bufferIdBTSback, vaoIdBTSback;
static GLuint bufferIdBTSfront, vaoIdBTSfront;
static float x, y, z;
static float d, h, w, n;
static int verts, vertsBTSback, vertsBTSfront;

static float surface(float y, float *fc);
float dPolyAspher(float y, float *fc);
static void preCalc();

float mod(float x, float y) {
	return x - y * floor(x / y);
}
float BTSsurface(float y0, float z0, float BTSinOut, float br, float bp) {
	float b = mod(y0 - z0 - bp / 2, bp) - bp / 2;
	return BTSinOut * sqrt(br*br - b * b);
}
float dBTSequation(float y0, float z0, float BTSinOut, float br, float bp) {
	float b = mod(y0 - z0 - bp / 2, bp) - bp / 2;
	return BTSinOut * b / sqrt(br*br - b * b);
}

void Lens::generate(float cx, float  cy, float cz, float cd, float  ch, float cw, float cn, float *fc,
	float br, float bp, float bw, float bg) {
	x = cx; y = cy; z = cz;
	d = cd; h = ch; w = cw; n = cn;
	float d2 = d / 2, w2 = w / 2, h2 = h / 2;

	{
		const int points = 500;
		//const int surfSize = (2 * (points - 1) + 2 + 6) * 3;
		const int surfSize = 2 * points * 6;
		float *surf = new float[surfSize];
		for (int i = 0; i < points; ++i) {
			float ySurf = i / (float)(points - 1) * h - h2;
			float xSurf = surface(ySurf, fc);
			int i6 = i * 6 * 2;
			surf[i6 + 0] = surf[i6 + 6] = xSurf + d2;
			surf[i6 + 1] = surf[i6 + 7] = ySurf;
			surf[i6 + 2] = w2;
			surf[i6 + 8] = -w2;
			float n = dPolyAspher(ySurf, fc);
			surf[i6 + 3] = surf[i6 + 9] =  -1.0f;
			surf[i6 + 4] = surf[i6 + 10] = -n;
			surf[i6 + 5] = surf[i6 + 11] = 0;
		}
		/*surf[surfSize - 6 * 3 + 0] = -d2;
		surf[surfSize - 6 * 3 + 1] = h2;
		surf[surfSize - 6 * 3 + 2] = w2;

		surf[surfSize - 5 * 3 + 0] = -d2;
		surf[surfSize - 5 * 3 + 1] = h2;
		surf[surfSize - 5 * 3 + 2] = -w2;

		surf[surfSize - 4 * 3 + 0] = -d2;
		surf[surfSize - 4 * 3 + 1] = -h2;
		surf[surfSize - 4 * 3 + 2] = w2;

		surf[surfSize - 3 * 3 + 0] = -d2;
		surf[surfSize - 3 * 3 + 1] = -h2;
		surf[surfSize - 3 * 3 + 2] = -w2;

		surf[surfSize - 2 * 3 + 0] = surf[0];
		surf[surfSize - 2 * 3 + 1] = surf[1];
		surf[surfSize - 2 * 3 + 2] = surf[2];

		surf[surfSize - 1 * 3 + 0] = surf[3];
		surf[surfSize - 1 * 3 + 1] = surf[4];
		surf[surfSize - 1 * 3 + 2] = surf[5];*/

		verts = surfSize / 6;

		int attribFormat[] = { 3, 3 };
		Shader::createBuffer(GL_STATIC_DRAW, &vaoId, &bufferId, surf, surfSize * sizeof(float), attribFormat, 2);
		delete[] surf;
	}
	{
		const int points = 5000;
		const int surfBTSSize = 2 * points * 6;
		float *surfBTS = new float[surfBTSSize];
		for (int i = 0; i < points; ++i) {
			float zSurf = i / (float)(points - 1) * w - w2;
			float xSurf = BTSsurface(0, zSurf, -1.0f, br, bp);
			int i6 = i * 6 * 2;
			surfBTS[i6 + 0] = surfBTS[i6 + 6] = d2 + xSurf + br + bg;
			surfBTS[i6 + 1] = -h2;
			surfBTS[i6 + 7] = h2;
			surfBTS[i6 + 2] = zSurf - h2;
			surfBTS[i6 + 8] = zSurf + h2;
			float n = dBTSequation(0, zSurf, -1.0f, br, bp);
			surfBTS[i6 + 3] = surfBTS[i6 + 9] = -1.0f;
			surfBTS[i6 + 4] = surfBTS[i6 + 10] = n;
			surfBTS[i6 + 5] = surfBTS[i6 + 11] = -n;
		}

		vertsBTSback = surfBTSSize / 6;

		int attribFormatBTS[] = { 3, 3 };
		Shader::createBuffer(GL_STATIC_DRAW, &vaoIdBTSback, &bufferIdBTSback, surfBTS, surfBTSSize * sizeof(float), attribFormatBTS, 2);
		delete[] surfBTS;
	}
	{
		const int points = 5000;
		const int surfBTSSize = 2 * points * 6;
		float *surfBTS = new float[surfBTSSize];
		for (int i = 0; i < points; ++i) {
			float zSurf = i / (float)(points - 1) * w - w2;
			float xSurf = BTSsurface(0, zSurf, 1.0f, br, bp);
			int i6 = i * 6 * 2;
			surfBTS[i6 + 0] = surfBTS[i6 + 6] = d2 + xSurf - br + bw + bg;
			surfBTS[i6 + 1] = -h2;
			surfBTS[i6 + 7] = h2;
			surfBTS[i6 + 2] = zSurf - h2;
			surfBTS[i6 + 8] = zSurf + h2;
			float n = dBTSequation(0, zSurf, 1.0f, br, bp);
			surfBTS[i6 + 3] = surfBTS[i6 + 9] = -1.0f;
			surfBTS[i6 + 4] = surfBTS[i6 + 10] = -n;
			surfBTS[i6 + 5] = surfBTS[i6 + 11] = n;
		}

		vertsBTSfront = surfBTSSize / 6;

		int attribFormatBTS[] = { 3 , 3 };
		Shader::createBuffer(GL_STATIC_DRAW, &vaoIdBTSfront, &bufferIdBTSfront, surfBTS, surfBTSSize * sizeof(float), attribFormatBTS, 2);
		delete[] surfBTS;
	}
	preCalc();
}

void Lens::draw() {
	glUseProgram(Shader::Triag::programHandle);
	Matrix::multiplyMM(Camera::mMVPMat, Camera::mVPMat, modelMat);
	glUniformMatrix4fv(Shader::Triag::Uniform::uMatHandle, 1, GL_FALSE, Camera::mMVPMat);

	glBindVertexArray(vaoId);
	glEnableVertexAttribArray(Shader::Triag::Input::Position);
	glEnableVertexAttribArray(Shader::Triag::Input::Normal);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, verts);
	glDisableVertexAttribArray(Shader::Triag::Input::Normal);
	glDisableVertexAttribArray(Shader::Triag::Input::Position);

	glBindVertexArray(vaoIdBTSback);
	glEnableVertexAttribArray(Shader::Triag::Input::Position);
	glEnableVertexAttribArray(Shader::Triag::Input::Normal);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, vertsBTSback);
	glDisableVertexAttribArray(Shader::Triag::Input::Normal);
	glDisableVertexAttribArray(Shader::Triag::Input::Position);

	glBindVertexArray(vaoIdBTSfront);
	glEnableVertexAttribArray(Shader::Triag::Input::Position);
	glEnableVertexAttribArray(Shader::Triag::Input::Normal);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, vertsBTSfront);
	glDisableVertexAttribArray(Shader::Triag::Input::Normal);
	glDisableVertexAttribArray(Shader::Triag::Input::Position);

	glBindVertexArray(0);
}

float surface(float y0, float *p) {
	return
		(y0*y0 / p[0]) /
		(1.0 + sqrt(abs(1.0 - (1.0 + p[1])*y0*y0 / (p[0] * p[0])))) +
		p[2] * y0*y0*y0*y0 +
		p[3] * y0*y0*y0*y0*y0*y0 +
		p[4] * y0*y0*y0*y0*y0*y0*y0*y0 +
		p[5] * y0*y0*y0*y0*y0*y0*y0*y0*y0*y0 +
		p[6] * y0*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0 +
		p[7] * y0*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0;
}
float dPolyAspher(float y0, float *p) {
	float sq = sqrt(1.0 - (1.0 + p[1])*y0*y0 / (p[0] * p[0]));
	float b = p[0] * (1.0 + sq);
	float db = -(1.0 + p[1])*y0 / (p[0] * sq);
	return ((2.0 * y0) * b - (y0*y0) * db) / (b * b) +
		4.0*p[2] * y0*y0*y0 +
		6.0*p[3] * y0*y0*y0*y0*y0 +
		8.0*p[4] * y0*y0*y0*y0*y0*y0*y0 +
		10.0*p[5] * y0*y0*y0*y0*y0*y0*y0*y0*y0 +
		12.0*p[6] * y0*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0 +
		14.0*p[7] * y0*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0*y0;
}

void Lens::rotate(float cRot, int axisID) {
	rots[axisID] += cRot;
	preCalc();
}

void Lens::rotateTo(float cRot, int axisID) {
	rots[axisID] = cRot;
	preCalc();
}

void Lens::move(float dx, float dy, float dz) {
	x += dx;
	float pX = Plane::getX();
	if (x + d > pX) x = pX - d;
	if (x < 0) x = 0;
	y += dy;
	z += dz;
	preCalc();
}

void Lens::moveToX(float dx) {
	x = dx;
	float pX = Plane::getX();
	if (x + d > pX) x = pX - d;
	if (x < 0) x = 0;
	preCalc();
}
void Lens::moveToY(float dy) {
	y = dy;
	preCalc();
}
void Lens::moveToZ(float dz) {
	z = dz;
	preCalc();
}

float Lens::getRots(int index) {
	return rots[index];
}

float Lens::getX() {
	return x;
}
float Lens::getY() {
	return y;
}
float Lens::getZ() {
	return z;
}

void preCalc() {
	float rotX = x + d / 2.0f, rotY = y, rotZ = z;

	Matrix::setIdentityM(lens1Mat);
	Matrix::translateM(lens1Mat, rotX, rotY, rotZ);
	for (int i = sizeof(rots) / sizeof(float) - 1; i >= 0; --i)
		Matrix::rotateM(lens1Mat, rots[i], axis[i]);
	Matrix::translateM(lens1Mat, -rotX, -rotY, -rotZ);

	Matrix::setIdentityM(lens2Mat);
	for (int i = sizeof(rots) / sizeof(float) - 1; i >= 0; --i)
		Matrix::rotateM(lens2Mat, rots[i], axis[i]);

	Matrix::setIdentityM(lens3Mat);
	Matrix::translateM(lens3Mat, rotX, rotY, rotZ);
	for (int i = sizeof(rots) / sizeof(float) - 1; i >= 0; --i)
		Matrix::rotateM(lens3Mat, -rots[i], axis[i]);
	Matrix::translateM(lens3Mat, -rotX, -rotY, -rotZ);

	Matrix::setIdentityM(lens4Mat);
	for (int i = sizeof(rots) / sizeof(float) - 1; i >= 0; --i)
		Matrix::rotateM(lens4Mat, -rots[i], axis[i]);

	Matrix::setIdentityM(modelMat);
	Matrix::translateM(modelMat, rotX, rotY, rotZ);
	for (int i = sizeof(rots) / sizeof(float) - 1; i >= 0; --i)
		Matrix::rotateM(modelMat, -rots[i], axis[i]);

	glUseProgram(Shader::Line::programHandle);
	glUniformMatrix4fv(Shader::Line::Uniform::uLensMat1Handle, 1, GL_FALSE, lens1Mat);
	glUniformMatrix4fv(Shader::Line::Uniform::uLensMat2Handle, 1, GL_FALSE, lens2Mat);
	glUniformMatrix4fv(Shader::Line::Uniform::uLensMat3Handle, 1, GL_FALSE, lens3Mat);
	glUniformMatrix4fv(Shader::Line::Uniform::uLensMat4Handle, 1, GL_FALSE, lens4Mat);
	glUniform4f(Shader::Line::Uniform::uLensParamHandle, d, h, w, n);
	glUniform3f(Shader::Line::Uniform::uLensPosHandle, x, y, z);
	glUseProgram(0);
}

void Lens::setAxis(int axisID, Axis cAxis) {
	axis[axisID] = cAxis;
	preCalc();
}

float Lens::getBoundX() {
	return x + d;
}

void Lens::release() {
	glDeleteVertexArrays(1, &vaoId);
	glDeleteBuffers(1, &bufferId);
	glDeleteVertexArrays(1, &vaoIdBTSback);
	glDeleteBuffers(1, &bufferIdBTSback);
	glDeleteVertexArrays(1, &vaoIdBTSfront);
	glDeleteBuffers(1, &bufferIdBTSfront);
}