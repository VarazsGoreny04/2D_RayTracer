#ifndef OBJECTMANIPULATOR_H
#define OBJECTMANIPULATOR_H

#include <glm/glm.hpp>
#include "../LightSource.h"

struct SDL_KeyboardEvent;
struct SDL_MouseMotionEvent;
struct SDL_MouseWheelEvent;

class ObjectManipulator
{
public:
	glm::vec2& position;
	float& direction;

	float goForward = 0.0f;
	float goRight = 0.0f;
	float rotate = 0.0f;

	float speed = 10.0f;

	ObjectManipulator(glm::vec2& position, float& direction);

	~ObjectManipulator();

	void Update(float deltaTime) const;

	void KeyboardDown(const SDL_KeyboardEvent& key);
	void KeyboardUp(const SDL_KeyboardEvent& key);
	void MouseMove(const SDL_MouseMotionEvent& mouse) const;
};

#endif // OBJECTMANIPULATOR_H