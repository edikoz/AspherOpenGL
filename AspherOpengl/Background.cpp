#include "Background.h"

#include <cmath>
#define GLEW_STATIC
#include <GL\glew.h>

#include "Shader.h"
#include "Matrix.h"
#include "Camera.h"

static GLfloat modelMat[16], mVPMat[16], mViewMat[16], mProjMat[16];
static GLuint vaoId, bufferId, guiVaoId, guiBufferId;

void Background::generate() {
	{
		float cB[] = {
		-1.0f,1.0f,
		-1.0f,-1.0f,
		1.0f,-1.0f,
		1.0f,1.0f,
		};
		int cBattribFormat = 2;
		Shader::createBuffer(GL_STATIC_DRAW, &vaoId, &bufferId, cB, sizeof(cB), &cBattribFormat, 1);
	}
	{
		float cG[] = {
			0.0f,0.0f,0.0, 1.0f,0.0f,0.0,
			0.0f,0.0f,0.0, 0.0f,1.0f,0.0,
			0.0f,0.0f,0.0, 0.0f,0.0f,1.0,
		};
		int cGattribFormat = 3;
		Shader::createBuffer(GL_STATIC_DRAW, &guiVaoId, &guiBufferId, cG, sizeof(cG), &cGattribFormat, 1);
	}
}

void Background::draw() {
	glUseProgram(Shader::Background::programHandle);
	glBindVertexArray(vaoId);
	glEnableVertexAttribArray(Shader::Background::Input::Position);
	//glUniform1f(Shader::Background::Uniform::uSinViewAngle, (Camera::getView() - Camera::getPos()).y);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableVertexAttribArray(Shader::Background::Input::Position);
	glBindVertexArray(0);
}

void Background::drawGUI() {
	glUseProgram(Shader::Graph::programHandle);
	glBindVertexArray(guiVaoId);
	glEnableVertexAttribArray(Shader::Graph::Input::Position);

	Matrix::setIdentityM(modelMat);
	Matrix::translateM(modelMat, -0.9f, 0.8f, 0);
	Matrix::ortho(mProjMat, -8, 8, -8 * Camera::getRatio(), 8 * Camera::getRatio(), 0.01f, 1000.0f);
	Matrix::lookAt(mViewMat, (Camera::getView() - Camera::getPos()) * -15.0, Vector3(0, 0, 0), Vector3(0, 1, 0));
	Matrix::multiplyMM(mVPMat, mProjMat, mViewMat);
	Matrix::multiplyMM(modelMat, mVPMat);

	glUniformMatrix4fv(Shader::Graph::Uniform::uMatHandle, 1, GL_FALSE, modelMat);
	glUniform4f(Shader::Graph::Uniform::uColor, 1.0f, 0.0f, 0.0f, 1.0f);
	glDrawArrays(GL_LINES, 0, 2);
	glUniform4f(Shader::Graph::Uniform::uColor, 0.0f, 1.0f, 0.0f, 1.0f);
	glDrawArrays(GL_LINES, 2, 2);
	glUniform4f(Shader::Graph::Uniform::uColor, 0.0f, 0.0f, 1.0f, 1.0f);
	glDrawArrays(GL_LINES, 4, 2);
	glDisableVertexAttribArray(Shader::Graph::Input::Position);
	glBindVertexArray(0);
}

void Background::release() {
	glDeleteVertexArrays(1, &vaoId);
	glDeleteVertexArrays(1, &guiVaoId);
	glDeleteBuffers(1, &bufferId);
	glDeleteBuffers(1, &guiBufferId);
}