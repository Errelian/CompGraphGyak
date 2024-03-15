#include "CameraManipulator.h"


void CameraManipulator::SetCamera(Camera* _pCamera) 
{
	pCamera = _pCamera;

	center = pCamera->GetAt();

	glm::vec3 aim = center - pCamera->GetEye();

	distance = glm::length(aim);

	fi = atan2f(aim.z, aim.x);
	theta = acosf(aim.y / distance); //what if its zero
}

void CameraManipulator::Update(float deltaTime)
{
	if (!pCamera)
		return;

	fi += deltaTime * 0.5;

	glm::vec3 lookDir(
		cosf(fi) * sinf(theta),
		cosf(theta),
		sinf(fi) * sinf(theta)
	);

	eye = center - distance * lookDir;//scalar multiplication and vector substraction, so we get a vector that points to eye, basically just made the equation a bit simpler by multiplying AFTER

	pCamera->SetView(eye, center, pCamera->GetWorldUp()); //center changes, not eye for FPS
}

void CameraManipulator::MouseMove(const SDL_MouseMotionEvent& mouse) 
{
	if (mouse.state & SDL_BUTTON_LMASK)
	{
		fi += mouse.xrel / 100.f; //mouse sensitivity
		theta = glm::clamp<float>(theta + mouse.yrel / 100.f, 0.1, 3.1);
	}
}

