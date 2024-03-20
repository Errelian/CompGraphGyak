#pragma once

#include "Camera.h"
#include <SDL2/SDL_events.h>

class CameraManipulator
{
public:
	void SetCamera(Camera* _pCamera);

	void Update();

	void MouseMove(const SDL_MouseMotionEvent& key);


private:
	Camera* pCamera = nullptr;

	float fi = 0.0f;
	float theta = 0.0f;
	float distance = 5.0f;

	glm::vec3 eye = glm::vec3(0, 0, 5);
	glm::vec3 center = glm::vec3(0);
};

class FpsCameraManipulator
{
public:
	void SetCamera(Camera* _pCamera);

	void Update();

	void MouseMove(const SDL_MouseMotionEvent& key);


private:
	Camera* pCamera = nullptr;

	float fi = 0.0f;
	float theta = 0.0f;
	float distance = 5.0f;

	glm::vec3 eye = glm::vec3(0, 0, 5);
	glm::vec3 center = glm::vec3(0);
};