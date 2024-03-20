#include "CameraManipulator.h"


void CameraManipulator::SetCamera(Camera* _pCamera) 
{
	pCamera = _pCamera;

	center = pCamera->GetAt();

	glm::vec3 aim = center - pCamera->GetEye(); //eye == ahol vagyunk

	distance = glm::length(aim);

	fi = atan2f(aim.z, aim.x);
	theta = acosf(aim.y / distance); //what if its zero
}

void CameraManipulator::Update()
{
	if (!pCamera)
		return;

	glm::vec3 lookDir(
		cosf(fi) * sinf(theta),
		cosf(theta),
		sinf(fi) * sinf(theta)
	);

	eye = center - distance * lookDir;

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

void FpsCameraManipulator::SetCamera(Camera* _pCamera)
{
	pCamera = _pCamera;

	center = pCamera->GetAt();

	glm::vec3 aim = center - pCamera->GetEye();

	distance = glm::length(aim);

	fi = atan2f(aim.z, aim.x);
	theta = acosf(aim.y / distance); //what if its zero
}

void FpsCameraManipulator::Update()
{
	if (!pCamera)
		return;

	glm::vec3 lookDir(
		cosf(fi) * sinf(theta),
		cosf(theta),
		sinf(fi) * sinf(theta)
	);

	center = distance * lookDir - eye;

	pCamera->SetView(eye, center, pCamera->GetWorldUp()); //center changes, not eye for FPS
}

void FpsCameraManipulator::MouseMove(const SDL_MouseMotionEvent& mouse)
{
	if (mouse.state & SDL_BUTTON_LMASK)
	{
		fi += mouse.xrel / 100.f; //mouse sensitivity
		theta = glm::clamp<float>(theta + mouse.yrel / 100.f, 0.1, 3.1);
	}
}