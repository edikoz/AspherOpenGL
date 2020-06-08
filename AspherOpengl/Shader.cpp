#include "Shader.h"

#include <iostream>
#include <fstream>
#include <map>

using namespace std;

GLuint Shader::Point::programHandle;
GLuint Shader::Line::programHandle;
GLuint Shader::Triag::programHandle;
GLuint Shader::textureTriag::programHandle;
GLuint Shader::Graph::programHandle;
GLuint Shader::Text::programHandle;
GLuint Shader::Ticks::programHandle;
GLuint Shader::Background::programHandle;

GLuint
Shader::Point::Uniform::uMatHandle,
Shader::Point::Uniform::uColorHandle,
Shader::Line::Uniform::uMatHandle,
Shader::Line::Uniform::uLensMat1Handle,
Shader::Line::Uniform::uLensMat2Handle,
Shader::Line::Uniform::uLensMat3Handle,
Shader::Line::Uniform::uLensMat4Handle,
Shader::Line::Uniform::uLensPosHandle,
Shader::Line::Uniform::uLensParamHandle,
Shader::Line::Uniform::uPlanePosHandle,
Shader::Line::Uniform::uOpticsParam,
Shader::Line::Uniform::uOpticsPosition1,
Shader::Line::Uniform::uOpticsPosition2,
Shader::Line::Uniform::uOpticsPosition3,
Shader::Triag::Uniform::uMatHandle,
Shader::textureTriag::Uniform::uMatHandle,
Shader::textureTriag::Uniform::uTexture0,
Shader::Graph::Uniform::uMatHandle,
Shader::Graph::Uniform::uColor,
Shader::Text::Uniform::uScale,
Shader::Text::Uniform::uTranslate,
Shader::Text::Uniform::uFontShift,
Shader::Text::Uniform::uColor,
Shader::Text::Uniform::uFont,
Shader::Ticks::Uniform::uScale,
Shader::Ticks::Uniform::uTranslate,
Shader::Ticks::Uniform::uGap,
Shader::Ticks::Uniform::uColor;
//Shader::Background::Uniform::uSinViewAngle;

std::map <std::string, std::string> Shader::replaces;

void Shader::Init() {
	const GLchar *vargs[] = { "feedbackOut" };
	Point::programHandle = Load("Res/Shaders/point");
	Line::programHandle = Load("Res/Shaders/line", vargs, sizeof(vargs) / sizeof(GLchar *));
	Triag::programHandle = Load("Res/Shaders/triag");
	textureTriag::programHandle = Load("Res/Shaders/textureTriag");
	Graph::programHandle = Load("Res/Shaders/graph");
	Text::programHandle = Load("Res/Shaders/text");
	Ticks::programHandle = Load("Res/Shaders/ticks");
	Background::programHandle = Load("Res/Shaders/background");

	Point::Uniform::uMatHandle = glGetUniformLocation(Point::programHandle, "u_viewMat");
	Point::Uniform::uColorHandle = glGetUniformLocation(Point::programHandle, "u_color");
	Line::Uniform::uMatHandle = glGetUniformLocation(Line::programHandle, "u_viewMat");
	Line::Uniform::uLensMat1Handle = glGetUniformLocation(Line::programHandle, "u_lensMat1");
	Line::Uniform::uLensMat2Handle = glGetUniformLocation(Line::programHandle, "u_lensMat2");
	Line::Uniform::uLensMat3Handle = glGetUniformLocation(Line::programHandle, "u_lensMat3");
	Line::Uniform::uLensMat4Handle = glGetUniformLocation(Line::programHandle, "u_lensMat4");
	Line::Uniform::uLensPosHandle = glGetUniformLocation(Line::programHandle, "u_lensPos");
	Line::Uniform::uLensParamHandle = glGetUniformLocation(Line::programHandle, "u_lensParam");
	Line::Uniform::uOpticsParam = glGetUniformLocation(Line::programHandle, "u_opticsParam");
	Line::Uniform::uOpticsPosition1 = glGetUniformLocation(Line::programHandle, "u_opticsPosition1");
	Line::Uniform::uOpticsPosition2 = glGetUniformLocation(Line::programHandle, "u_opticsPosition2");
	Line::Uniform::uOpticsPosition3 = glGetUniformLocation(Line::programHandle, "u_opticsPosition3");
	Line::Uniform::uPlanePosHandle = glGetUniformLocation(Line::programHandle, "u_planePos");
	Triag::Uniform::uMatHandle = glGetUniformLocation(Triag::programHandle, "u_viewMat");
	textureTriag::Uniform::uMatHandle = glGetUniformLocation(textureTriag::programHandle, "u_viewMat");
	textureTriag::Uniform::uTexture0 = glGetUniformLocation(textureTriag::programHandle, "s2D_Texture0");
	Graph::Uniform::uMatHandle = glGetUniformLocation(Graph::programHandle, "u_viewMat");
	Graph::Uniform::uColor = glGetUniformLocation(Graph::programHandle, "u_color");
	Text::Uniform::uColor = glGetUniformLocation(Text::programHandle, "u_color");
	Text::Uniform::uFont = glGetUniformLocation(Text::programHandle, "s2D_font");
	Text::Uniform::uTranslate = glGetUniformLocation(Text::programHandle, "u_translate");
	Text::Uniform::uScale = glGetUniformLocation(Text::programHandle, "u_scale");
	Text::Uniform::uFontShift = glGetUniformLocation(Text::programHandle, "u_fontShift");
	Ticks::Uniform::uTranslate = glGetUniformLocation(Ticks::programHandle, "u_translate");
	Ticks::Uniform::uScale = glGetUniformLocation(Ticks::programHandle, "u_scale");
	Ticks::Uniform::uGap = glGetUniformLocation(Ticks::programHandle, "u_gap");
	Ticks::Uniform::uColor = glGetUniformLocation(Ticks::programHandle, "u_color");
	//Background::Uniform::uSinViewAngle = glGetUniformLocation(Background::programHandle, "u_sinViewAngle");
}
void Shader::Destroy() {
	glUseProgram(0);
	glDeleteProgram(Point::programHandle);
	glDeleteProgram(Line::programHandle);
	glDeleteProgram(Triag::programHandle);
	glDeleteProgram(textureTriag::programHandle);
	glDeleteProgram(Graph::programHandle);
	glDeleteProgram(Text::programHandle);
	glDeleteProgram(Ticks::programHandle);
	glDeleteProgram(Background::programHandle);
}

GLuint Shader::Load(string fname, const GLchar ** vargs, GLsizei size) {
	GLuint vertexShaderHandle = subLoad(fname + ".vsr", GL_VERTEX_SHADER);
	GLuint geometryShaderHandle = subLoad(fname + ".gsr", GL_GEOMETRY_SHADER);
	GLuint fragmentShaderHandle = subLoad(fname + ".fsr", GL_FRAGMENT_SHADER);
	GLuint programHandle = glCreateProgram();
	if (programHandle != 0)
	{
		glAttachShader(programHandle, vertexShaderHandle);
		if (geometryShaderHandle) glAttachShader(programHandle, geometryShaderHandle);
		glAttachShader(programHandle, fragmentShaderHandle);
		if (vargs != 0 && size != 0) glTransformFeedbackVaryings(programHandle, size, vargs, GL_INTERLEAVED_ATTRIBS);
		glLinkProgram(programHandle);

		GLint linkStatus;
		glGetProgramiv(programHandle, GL_LINK_STATUS, &linkStatus);
		if (linkStatus == 0)
		{
			cout << fname << " Link ERROR\n";
			glDeleteProgram(programHandle);
			programHandle = 0;
		}
		else cout << fname << " Link OK\n";
	}

	glDetachShader(programHandle, vertexShaderHandle);
	if (geometryShaderHandle) glDetachShader(programHandle, geometryShaderHandle);
	glDetachShader(programHandle, fragmentShaderHandle);
	glDeleteShader(vertexShaderHandle);
	if (geometryShaderHandle) glDeleteShader(geometryShaderHandle);
	glDeleteShader(fragmentShaderHandle);
	glReleaseShaderCompiler();

	return programHandle;
}

GLuint Shader::subLoad(string fname, GLenum type) {
	string shaderString = "";

	//Replace the filename if founded it in list
	if (replaces.count(fname)) fname = replaces[fname];

	ifstream shaderFile(fname, ios::in | ios_base::beg);
	if (shaderFile.is_open())
	{
		string tmp;
		while (getline(shaderFile, tmp))
			shaderString += tmp + '\n';
		shaderFile.close();
	}
	else return 0;
	GLuint shaderHandle = glCreateShader(type);
	if (shaderHandle != 0)
	{
		const GLchar *c = shaderString.c_str();
		glShaderSource(shaderHandle, 1, &c, 0);
		glCompileShader(shaderHandle);

		GLint compileStatus;
		glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileStatus);
		if (compileStatus == 0)
		{
			GLsizei len;
			GLchar inf[512];
			glGetShaderInfoLog(shaderHandle, 512, &len, inf);
			cout << fname << " Compiler ERROR:\n" << inf << endl;
			glDeleteShader(shaderHandle);
			shaderHandle = 0;
		}
		else cout << fname << " Compile OK\n";
	}
	return shaderHandle;
}

void Shader::createBuffer(GLenum usage, GLuint *vaoId, GLuint *bufferId, float* buffer, GLsizeiptr bufferSize, int *attributeSizes, GLuint attribSize, int feedbackOffset) {
	glGenVertexArrays(1, vaoId);
	glGenBuffers(1, bufferId);
	glBindVertexArray(*vaoId);
	glBindBuffer(GL_ARRAY_BUFFER, *bufferId);
	glBufferData(GL_ARRAY_BUFFER, bufferSize, buffer, usage);
	GLsizei stride = feedbackOffset;
	for (GLuint i = 0; i < attribSize; ++i)
		stride += attributeSizes[i];
	for (GLuint i = 0, localOffset = 0; i < attribSize; ++i) {
		glVertexAttribPointer(i, attributeSizes[i], GL_FLOAT, GL_FALSE, sizeof(float) * stride, (GLchar*)0 + sizeof(float) * (localOffset + feedbackOffset));
		localOffset += attributeSizes[i];
	}
	if (feedbackOffset > 0) glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, *bufferId);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}