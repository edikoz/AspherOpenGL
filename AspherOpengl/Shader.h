#pragma once

#include <string>
#define GLEW_STATIC
#include <GL\glew.h>
#include <map>

class Shader {
public:
	static std::map <std::string, std::string> replaces;

	static void Init();
	static void createBuffer(GLenum usage, GLuint *vaoId, GLuint *bufferId, float* buffer, GLsizeiptr bufferSize, int *attributeSizes, GLuint attribSize, int offset = 0);
	static void Destroy();

	struct Point {
		static GLuint programHandle;
		struct Uniform {
			static GLuint uMatHandle, uColorHandle;
		};
		enum Input {
			Position
		};
	};

	struct Line {
		static GLuint programHandle;
		struct Uniform {
			static GLuint uMatHandle,
				uLensMat1Handle, uLensMat2Handle, uLensMat3Handle, uLensMat4Handle,
				uLensPosHandle, uLensParamHandle, uOpticsParam, uOpticsPosition1, uOpticsPosition2, uOpticsPosition3,
				uPlanePosHandle;
		};
		enum Input {
			Position
		};
	};

	struct Triag {
		static GLuint programHandle;
		struct Uniform {
			static GLuint uMatHandle;
		};
		enum Input {
			Position, Normal
		};
	};

	struct textureTriag {
		static GLuint programHandle;
		struct Uniform {
			static GLuint uMatHandle, uTexture0;
		};
		enum Input {
			Position, TextureCoord
		};
	};
	
	struct Graph {
		static GLuint programHandle;
		struct Uniform {
			static GLuint uMatHandle, uColor;
		};
		enum Input {
			Position
		};
	};

	struct Text {
		static GLuint programHandle;
		struct Uniform {
			static GLuint uColor, uFont, uScale, uTranslate, uFontShift;
		};
		enum Input {
			Position, TextureCoord
		};
	};

	struct Ticks {
		static GLuint programHandle;
		struct Uniform {
			static GLuint uScale, uTranslate, uGap, uColor;
		};
		enum Input {
			Position
		};
	};

	struct Background {
		static GLuint programHandle;
		/*struct Uniform {
			static GLuint uSinViewAngle;
		};*/
		enum Input {
			Position
		};
	};
private:
	static GLuint Load(std::string fname, const GLchar ** vargs = 0, GLsizei size = 0);
	static GLuint subLoad(std::string fname, GLenum type);
};