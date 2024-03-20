#pragma once

#include "Camera.h"
#include <SDL2/SDL_events.h>

class FlyCameraManipulator
{
public:
	void SetCamera(Camera* _pCamera);

	void Update();

	void KeyboardMove(const SDL_KeyboardEvent& key);


private:
	Camera* pCamera = nullptr;

	glm::vec3 eye = glm::vec3(0, 0, 5);
	glm::vec3 center = glm::vec3(0);

	glm::vec3 translationVec = glm::vec3(1.0f);

	bool update = false;
};

