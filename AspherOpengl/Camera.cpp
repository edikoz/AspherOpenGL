#include "Camera.h"

#define _USE_MATH_DEFINES
#include <cmath>

#include "Matrix.h"
#include "Plane.h"

float Camera::mMVPMat[16], Camera::mVPMat[16];
bool Camera::ortho = false;

static float mMainViewMat[16], mPerspectiveProjMat[16], mOrthogonalProjMat[16];
static float mSensorViewMat[16], mSensorProjMat[16];
static float *mViewMat = mMainViewMat, *mProjMat = mPerspectiveProjMat;

static float FOV = 30.0f * M_PI / 180.0f, ZOOM = 5.0f, ratio = 2.0f;

static float ax = 0, ay = -M_PI;
static float camPx = 0, camPy = 0, camPz = 0;
static float camVx = 0, camVy = 0, camVz = -1;
static float camUx = 0, camUy = 1, camUz = 0;

static inline void preCalc() {
	double cax = cos(ax);
	double sax = sin(ax);
	double cay = cos(ay);
	double say = sin(ay);

	camVx = (float)(say * cax) + camPx;
	camVy = (float)(sax)+camPy;
	camVz = (float)(cay * cax) + camPz;

	camUx = 0;// (float)(say * -sax);
	camUy = 1;// (float)(cax);
	camUz = 0;// (float)(cay * -sax);

	Matrix::lookAt(mViewMat, Vector3(camPx, camPy, camPz), Vector3(camVx, camVy, camVz), Vector3(camUx, camUy, camUz));
	Matrix::multiplyMM(Camera::mVPMat, mProjMat, mViewMat);
}

void Camera::moveDirection(float dmove, Direction direction) {
	switch (direction) {
	case forward: move(dmove * sin(ay), 0, dmove * cos(ay)); break;
	case back: move(-dmove * sin(ay), 0, -dmove * cos(ay)); break;
	case left: move(dmove * cos(ay), 0, -dmove * sin(ay)); break;
	case right: move(-dmove * cos(ay), 0, dmove * sin(ay)); break;
	}
}

void Camera::move(float dx, float dy, float dz) {
	camPx += dx;
	camPy += dy;
	camPz += dz;

	preCalc();
}
void Camera::moveTo(float dx, float dy, float dz) {
	camPx = dx;
	camPy = dy;
	camPz = dz;

	preCalc();
}

void Camera::rotate(float dax, float day) {
	ax += dax;
	ay += day;

	if (ax >= M_PI / 2.0f) ax = M_PI / 2.0f;
	if (ax <= -M_PI / 2.0f) ax = -M_PI / 2.0f;

	preCalc();
}
void Camera::rotateTo(float dax, float day) {
	ax = dax;
	ay = day;

	if (ax >= M_PI / 2.0f) ax = M_PI / 2.0f;
	if (ax <= -M_PI / 2.0f) ax = -M_PI / 2.0f;

	preCalc();
}

Vector3 Camera::getView() {
	return Vector3(camVx, camVy, camVz);
}

Vector3 Camera::getPos() {
	return Vector3(camPx, camPy, camPz);
}

void Camera::increaseScale(float delta) {
	if (ortho) ZOOM += delta / 120.0f;
	else FOV += delta / 120.0f * M_PI / 180.0f;
	Matrix::ortho(mOrthogonalProjMat, -ZOOM, ZOOM, -ZOOM*ratio, ZOOM*ratio, 0.01f, 1000.0f);
	Matrix::perspective(mPerspectiveProjMat, FOV, ratio, 0.01f, 1000.0f);

	Matrix::multiplyMM(mVPMat, mProjMat, mViewMat);
}
void Camera::setScale(float cScale) {
	if (ortho) ZOOM = cScale;
	//else FOV = cScale / 120.0f * M_PI / 180.0f;
	Matrix::ortho(mOrthogonalProjMat, -ZOOM, ZOOM, -ZOOM*ratio, ZOOM*ratio, 0.01f, 1000.0f);
	Matrix::perspective(mPerspectiveProjMat, FOV, ratio, 0.01f, 1000.0f);

	Matrix::multiplyMM(mVPMat, mProjMat, mViewMat);
}

void Camera::changeProjection() {
	ortho = !ortho;
}

void Camera::changeRatio(float r) {
	ratio = r;
	Matrix::ortho(mOrthogonalProjMat, -ZOOM, ZOOM, -ZOOM*ratio, ZOOM*ratio, 0.01f, 1000.0f);
	Matrix::perspective(mPerspectiveProjMat, FOV, ratio, 0.01f, 1000.0f);

	Matrix::multiplyMM(mVPMat, mProjMat, mViewMat);
}

void Camera::setCameraToMain() {
	mViewMat = mMainViewMat;
	mProjMat = (ortho) ? mOrthogonalProjMat : mPerspectiveProjMat;

	Matrix::multiplyMM(mVPMat, mProjMat, mViewMat);
}

void Camera::setCameraToSensor() {
	mViewMat = mSensorViewMat;
	mProjMat = mSensorProjMat;
	Matrix::lookAt(mViewMat, Vector3(Plane::getX(), Plane::getY(), 0), Vector3(0, Plane::getY(), 0), Vector3(0, 1, 0));
	Matrix::orthoWOz(mProjMat, -Plane::h / 2.0f, Plane::h / 2.0f, -Plane::w / 2.0f, Plane::w / 2.0f);

	Matrix::multiplyMM(mVPMat, mProjMat, mViewMat);
}

float Camera::getFOV() {
	return FOV;
}
float Camera::getRatio() {
	return ratio;
}