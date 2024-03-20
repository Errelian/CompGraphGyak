#include "FlyCameraManipulator.h"


void FlyCameraManipulator::SetCamera(Camera* _pCamera)
{
	pCamera = _pCamera;

	center = pCamera->GetAt();

	
}

void FlyCameraManipulator::Update()
{
	if (!pCamera)
		return;

	/*glm::vec3 lookDir(
		cosf(fi) * sinf(theta),
		cosf(theta),
		sinf(fi) * sinf(theta)
	);

	eye = center - distance * lookDir;*/

	//pCamera->SetView(eye, center, pCamera->GetWorldUp()); //center changes, not eye for FPS

	if (update) {
		pCamera->SetView(eye + translationVec, center + translationVec, pCamera->GetWorldUp());
	}

	translationVec = glm::vec3(1.0f);
	update = false;
}

void FlyCameraManipulator::KeyboardMove(const SDL_KeyboardEvent& key)
{
	/*if (mouse.state & SDL_BUTTON_LMASK)
	{
		fi += mouse.xrel / 100.f; //mouse sensitivity
		theta = glm::clamp<float>(theta + mouse.yrel / 100.f, 0.1, 3.1);
	}*/

	glm::vec3 aim = glm::normalize(center - pCamera->GetEye());

	if (key.keysym.sym == SDLK_w)
	{
		translationVec *= (aim);
		update = true;
	}
	if (key.keysym.sym == SDLK_s)
	{
		translationVec = -1.0f * (translationVec * aim);
		update = true;
	}
}
