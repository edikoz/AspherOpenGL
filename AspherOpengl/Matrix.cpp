#include "Matrix.h"

#include <cmath>
#include <cstring>

void Matrix::multiplyMM(float *mat1, float *mat2) {
	float tmp[16];
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j) {
			tmp[j + i * 4] = mat2[0 + i * 4] * mat1[j + 0 * 4]
				+ mat2[1 + i * 4] * mat1[j + 1 * 4]
				+ mat2[2 + i * 4] * mat1[j + 2 * 4]
				+ mat2[3 + i * 4] * mat1[j + 3 * 4];
		}
	memcpy(mat1, tmp, sizeof(float) * 16);
}

void Matrix::multiplyMM(float *destMat, float *mat1, float *mat2) {
	float tmp[16];
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j) {
			tmp[j + i * 4] = mat2[0 + i * 4] * mat1[j + 0 * 4]
				+ mat2[1 + i * 4] * mat1[j + 1 * 4]
				+ mat2[2 + i * 4] * mat1[j + 2 * 4]
				+ mat2[3 + i * 4] * mat1[j + 3 * 4];
		}
	memcpy(destMat, tmp, sizeof(float) * 16);
}

void Matrix::setIdentityM(float *mat) {
	memset(mat, 0, sizeof(float) * 16);
	mat[0] = mat[5] = mat[10] = mat[15] = 1;
}

void Matrix::scaleM(float *mat, float x, float y, float z) {
	float tmp[16];
	memset(tmp, 0, sizeof(float) * 16);
	tmp[0] = x;
	tmp[5] = y;
	tmp[10] = z;
	tmp[15] = 1;
	multiplyMM(mat, tmp);
}

void Matrix::translateM(float *mat, float x, float y, float z) {
	float tmp[16];
	memset(tmp, 0, sizeof(float) * 16);
	tmp[0] = tmp[5] = tmp[10] = tmp[15] = 1;
	tmp[12] = x; tmp[13] = y; tmp[14] = z;
	multiplyMM(mat, tmp);
}

void Matrix::rotateM(float *mat, float rotateAngle, Axis charAxis) {
	float ca = cos(rotateAngle);
	float sa = sin(rotateAngle);

	switch (charAxis) {
	case X:
	{
		float tmp[] = {
			1,0,0,0,
			0,ca,-sa,0,
			0,sa,ca,0,
			0,0,0,1
		};
		multiplyMM(mat, tmp);
	}
	break;
	case Y:
	{
		float tmp[] = {
			ca,0,sa,0,
			0,1,0,0,
			-sa,0,ca,0,
			0,0,0,1
		};
		multiplyMM(mat, tmp);
	}
	break;
	case Z:
	{
		float tmp[] = {
			ca,-sa,0,0,
			sa,ca,0,0,
			0,0,1,0,
			0,0,0,1
		};
		multiplyMM(mat, tmp);
	}
	break;
	}
}

Vector3::Vector3(float a, float b, float c) {
	x = a; y = b; z = c;
}
Vector3 Vector3::cross(Vector3 up) {
	return Vector3(y*up.z - z*up.y, z*up.x - x*up.z, x*up.y - y*up.x);
}
void Vector3::normalize() {
	float len = sqrt(x*x + y*y + z*z);
	x = x / len;
	y = y / len;
	z = z / len;
}
Vector3 Vector3::operator-(const Vector3 & v) {
	return Vector3(x - v.x, y - v.y, z - v.z);
}
Vector3 Vector3::operator+(const Vector3 & v) {
	return Vector3(x + v.x, y + v.y, z + v.z);
}
Vector3 Vector3::operator*(const float & v) {
	return Vector3(x*v, y*v, z*v);
}
Vector3 Vector3::operator*(const float *m33) {
	return Vector3(
		x * m33[0] + y * m33[1] + z * m33[2],
		x * m33[4] + y * m33[5] + z * m33[6],
		x * m33[8] + y * m33[9] + z * m33[10]);
}

void Matrix::lookAt(float *mat, Vector3& eye, Vector3& target, Vector3& upDir) {
	Vector3 forward = eye - target;
	forward.normalize();
	Vector3 left = upDir.cross(forward);
	left.normalize();
	Vector3 up = forward.cross(left);
	float matrix[16] = {
		left.x, up.x, forward.x, 0,
		left.y, up.y, forward.y, 0,
		left.z, up.z, forward.z, 0,
		0, 0, 0, 1
	};
	translateM(matrix, -eye.x, -eye.y, -eye.z);
	memcpy(mat, matrix, sizeof(float) * 16);
}

void Matrix::perspective(float *mat, float fov, float aspect, float near, float far) {
	memset(mat, 0, sizeof(float) * 16);
	mat[0] = 1.0f / (tan(fov / 2.0f)*(aspect));
	mat[5] = 1.0f / (tan(fov / 2.0f));
	mat[10] = -(far + near) / (far - near);
	mat[14] = (-2.0f*far*near) / (far - near);
	mat[11] = -1;
}

void Matrix::ortho(float *mat, const float &b, const float &t, const float &l, const float &r, const float &n, const float &f) {
	memset(mat, 0, sizeof(float) * 16);
	mat[0] = 2.0f / (r - l);
	mat[5] = 2.0f / (t - b);
	mat[10] = -2.0f / (f - n);
	mat[12] = -(r + l) / (r - l);
	mat[13] = -(t + b) / (t - b);
	mat[14] = -(f + n) / (f - n);
	mat[15] = 1.0f;
}

void Matrix::orthoWOz(float *mat, const float &b, const float &t, const float &l, const float &r) {
	memset(mat, 0, sizeof(float) * 16);
	mat[0] = 2.0f / (r - l);
	mat[5] = 2.0f / (t - b);
	mat[3] = -(r + l) / (r - l);
	mat[7] = -(t + b) / (t - b);
	mat[11] = 0.5f;
	mat[15] = 1.0f;
}

void  Matrix::transponse(float *destMat, float *sourceMat) {
	float tmp[16];
	memcpy(tmp, sourceMat, sizeof(float) * 16);
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			destMat[i + 4 * j] = tmp[j + 4 * i];
}