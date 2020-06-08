#pragma once

#include "Matrix.h"

namespace Camera {
	extern float mMVPMat[16], mVPMat[16];
	extern bool ortho;

	enum Direction { forward, back, left, right };
	void moveDirection(float dmove, Direction direction);
	void move(float dx, float dy, float dz);
	void moveTo(float dx, float dy, float dz);
	void rotate(float dax, float day);
	void rotateTo(float dax, float day);
	void setScale(float cScale);
	void increaseScale(float delta);
	void changeProjection();
	void changeRatio(float ratio);
	void setCameraToMain();
	void setCameraToSensor();
	Vector3 getView();
	Vector3 getPos();
	float getFOV();
	float getRatio();
};