#include "Optics.h"

#define _USE_MATH_DEFINES
#include <cmath>
#define GLEW_STATIC
#include <GL\glew.h>

#include "Shader.h"
#include "Matrix.h"
#include "Camera.h"

static GLfloat modelMatPlane[16], modelMatSphere[16];
static GLuint bufferId, vaoId;
static float x;
static float r, d, l;//радиус кривизны, расстояние между зеркалами, диаметр зеркал
static float angle;

static void preCalc(float *outDx, float *outDy);

static float *surf;
static const int hdiv = 20;
static const int rdiv = 60;
static const int surfSize = hdiv*rdiv * 3 * 2;
static const int planeSize = (rdiv + 1) * 3;
static const int bufSize = surfSize + planeSize;
static const int verts1 = surfSize / 3, verts2 = planeSize / 3;

void Optics::generate(float cx, float cr, float cd, float cl, float ang) {
	x = cx; r = cr; d = cd; l = cl; angle = ang;
	surf = new float[bufSize];
	float dx, dy;
	preCalc(&dx, &dy);
	int attribFormat = 3;
	Shader::createBuffer(GL_STATIC_DRAW, &vaoId, &bufferId, surf, bufSize*sizeof(float), &attribFormat, 1);
}

void preCalc(float *outDx, float *outDy) {
	glUseProgram(Shader::Line::programHandle);
	float dx = *outDx = d * cos(2.0*(M_PI / 2 - angle));
	float dy = *outDy = d * sin(2.0*(M_PI / 2 - angle));
	float rx = r * sin(angle);
	float ry = r * cos(angle);
	float dToS = sqrt(2 * r*r - 2 * r*sqrt(r*r - l*l / 4));
	glUniform4f(Shader::Line::Uniform::uOpticsParam, x, r, l / 2, dToS);
	glUniform3f(Shader::Line::Uniform::uOpticsPosition1, -sin(angle), -cos(angle), 0.0);
	glUniform3f(Shader::Line::Uniform::uOpticsPosition2, x + rx - dx, ry - dy, 0.0);
	glUniform3f(Shader::Line::Uniform::uOpticsPosition3, x - dx, -dy, 0.0);
	glUseProgram(0);

	for (int i = 0; i < hdiv; ++i) {
		float xi = r - (float)(hdiv - i) / hdiv * (r - sqrt(r*r - l*l / 4));
		float xi1 = r - (float)(hdiv - i - 1) / hdiv * (r - sqrt(r*r - l*l / 4));
		float ki = sqrt(r*r - xi*xi);
		float ki1 = sqrt(r*r - xi1*xi1);
		for (int j = 0; j < rdiv; ++j) {
			int ind = (i*rdiv + j) * 3 * 2;
			float ang = 2 * M_PI*(float)j / (rdiv - 1);
			surf[ind + 0] = ki*cos(ang);
			surf[ind + 1] = -xi;
			surf[ind + 2] = ki*sin(ang);

			surf[ind + 3] = ki1*cos(ang);
			surf[ind + 4] = -xi1;
			surf[ind + 5] = ki1*sin(ang);
		}
	}

	surf[surfSize + 0] = 0;
	surf[surfSize + 1] = 0;
	surf[surfSize + 2] = 0;
	for (int i = 0; i < rdiv; ++i) {
		int ind = surfSize + (i + 1) * 3;
		float ang = 2 * M_PI*(float)i / (rdiv - 1);
		surf[ind + 0] = 0.5*cos(ang);
		surf[ind + 1] = 0;
		surf[ind + 2] = 0.5*sin(ang);
	}

	Matrix::setIdentityM(modelMatPlane);
	Matrix::translateM(modelMatPlane, x, 0, 0);
	Matrix::rotateM(modelMatPlane, angle, Axis::Z);
	Matrix::scaleM(modelMatPlane, l, 1, l);

	Matrix::setIdentityM(modelMatSphere);
	Matrix::translateM(modelMatSphere, x + rx - dx, ry - dy, 0);
	Matrix::rotateM(modelMatSphere, angle, Axis::Z);
}

void Optics::draw() {
	glUseProgram(Shader::Triag::programHandle);

	glBindVertexArray(vaoId);
	glEnableVertexAttribArray(Shader::Triag::Input::Position);

	Matrix::multiplyMM(Camera::mMVPMat, Camera::mVPMat, modelMatSphere);
	glUniformMatrix4fv(Shader::Triag::Uniform::uMatHandle, 1, GL_FALSE, Camera::mMVPMat);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, verts1);

	Matrix::multiplyMM(Camera::mMVPMat, Camera::mVPMat, modelMatPlane);
	glUniformMatrix4fv(Shader::Triag::Uniform::uMatHandle, 1, GL_FALSE, Camera::mMVPMat);
	glDrawArrays(GL_TRIANGLE_FAN, verts1, verts2);

	glDisableVertexAttribArray(Shader::Triag::Input::Position);
}

void Optics::setParams(float cx, float cr, float cd, float cl, float ang, float *outDx, float *outDy) {
	x = cx; r = cr; d = cd; l = cl; angle = ang;
	preCalc(outDx, outDy);
	glBindBuffer(GL_ARRAY_BUFFER, bufferId);
	glBufferData(GL_ARRAY_BUFFER, bufSize*sizeof(float), surf, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Optics::release() {
	delete[] surf;
	glDeleteVertexArrays(1, &vaoId);
	glDeleteBuffers(1, &bufferId);
}