#include "ObjectManipulator.h"

#include <SDL2/SDL.h>

ObjectManipulator::ObjectManipulator(glm::vec2& position, float& direction) : position(position), direction(direction) {}

ObjectManipulator::~ObjectManipulator() {}

void ObjectManipulator::Update(float deltaTime) const
{
	if (!(goForward + goRight))
		return;

	float directionInRadian = glm::radians(direction) + atan2(goRight, goForward);

	position += glm::vec2(glm::cos(directionInRadian), glm::sin(directionInRadian)) * speed * deltaTime;
}

void ObjectManipulator::KeyboardDown(const SDL_KeyboardEvent& key)
{
	switch (key.keysym.sym)
	{
	case SDLK_LSHIFT:
	case SDLK_RSHIFT:
		if (key.repeat == 0) speed /= 4.0f;
		break;
	case SDLK_w:
		goForward = 1;
		break;
	case SDLK_s:
		goForward = -1;
		break;
	case SDLK_a:
		goRight = 1;
		break;
	case SDLK_d:
		goRight = -1;
		break;
	}
}

void ObjectManipulator::KeyboardUp(const SDL_KeyboardEvent& key)
{
	switch (key.keysym.sym)
	{
	case SDLK_LSHIFT:
	case SDLK_RSHIFT:
		speed *= 4.0f;
		break;
	case SDLK_w:
	case SDLK_s:
		goForward = 0;
		break;
	case SDLK_a:
	case SDLK_d:
		goRight = 0;
		break;
	}
}

void ObjectManipulator::MouseMove(const SDL_MouseMotionEvent& mouse) const
{
	if (mouse.state & SDL_BUTTON_LMASK)
	{
		float du = mouse.xrel / 10.0f;
		// float dv = mouse.yrel / 100.0f;

		direction += du;
	}
}