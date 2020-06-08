#include "Plane.h"

#define GLEW_STATIC
#include <GL\glew.h>

#include "Shader.h"
#include "Matrix.h"
#include "Camera.h"
#include "Lens.h"
#include "Ray.h"

float Plane::w, Plane::h;
static GLfloat x, y;
static GLfloat modelMat[16];
static GLuint vaoId, bufferId;

static inline void calcMat() {
	Matrix::setIdentityM(modelMat);
	Matrix::translateM(modelMat, x, y, 0);
	Matrix::scaleM(modelMat, 1.0f, Plane::h, Plane::w);
}

void Plane::generate(float cw, float ch) {
	w = cw; h = ch;
	float cA[] = {
		0,-0.5f,0.5f, 0.0f, 0.0f,
		0,-0.5f,-0.5f, 1.0f, 0.0f,
		0,0.5f,0.5f, 0.0f, 1.0f,
		0,0.5f,-0.5f, 1.0f, 1.0f
	};
	int attribFormat[] = { 3,2 };

	Shader::createBuffer(GL_STATIC_DRAW, &vaoId, &bufferId, cA, sizeof(cA), attribFormat, 2);

	calcMat();
}

void Plane::draw() {
	glUseProgram(Shader::textureTriag::programHandle);

	glBindVertexArray(vaoId);
	glEnableVertexAttribArray(Shader::textureTriag::Input::Position);
	glEnableVertexAttribArray(Shader::textureTriag::Input::TextureCoord);

	Matrix::multiplyMM(Camera::mMVPMat, Camera::mVPMat, modelMat);
	glUniform1i(Shader::textureTriag::Uniform::uTexture0, 0);
	glUniformMatrix4fv(Shader::textureTriag::Uniform::uMatHandle, 1, GL_FALSE, Camera::mMVPMat);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(Shader::textureTriag::Input::TextureCoord);
	glDisableVertexAttribArray(Shader::textureTriag::Input::Position);
}

void Plane::setH(float ch) {
	h = ch;
	calcMat();
	glUseProgram(Shader::Line::programHandle);
	glUniform1f(Shader::Line::Uniform::uPlanePosHandle, x);
	glUseProgram(0);
}
void Plane::setW(float cw) {
	w = cw;
	calcMat();
	glUseProgram(Shader::Line::programHandle);
	glUniform1f(Shader::Line::Uniform::uPlanePosHandle, x);
	glUseProgram(0);
}

void Plane::moveX(float dx) {
	x += dx;
	float bX = Lens::getBoundX();
	if (x < bX) x = bX;
	calcMat();
	glUseProgram(Shader::Line::programHandle);
	glUniform1f(Shader::Line::Uniform::uPlanePosHandle, x);
	glUseProgram(0);
}
void Plane::moveToX(float dx) {
	x = dx;
	float bX = Lens::getBoundX();
	if (x < bX) x = bX;
	calcMat();
	glUseProgram(Shader::Line::programHandle);
	glUniform1f(Shader::Line::Uniform::uPlanePosHandle, x);
	glUseProgram(0);
}
float Plane::getX() {
	return x;
}

void Plane::moveY(float dy) {
	y += dy;
	calcMat();
}
void Plane::moveToY(float dy) {
	y = dy;
	calcMat();
}
float Plane::getY() {
	return y;
}

void Plane::release() {
	glDeleteVertexArrays(1, &vaoId);
	glDeleteBuffers(1, &bufferId);
}