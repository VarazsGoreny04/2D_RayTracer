#pragma once

#include <glm/glm.hpp>
#include <../LightSource.h>

class Camera;

struct SDL_KeyboardEvent;
struct SDL_MouseMotionEvent;
struct SDL_MouseWheelEvent;

class CameraManipulator
{
public:
	CameraManipulator(LightSource& lightSource);

	~CameraManipulator();

	void Update(float _deltaTime);

	inline void  SetSpeed(float _speed) { m_speed = _speed; }
	inline float GetSpeed() const noexcept { return m_speed; }

	void KeyboardDown(const SDL_KeyboardEvent& key);
	void KeyboardUp(const SDL_KeyboardEvent& key);
	void MouseMove(const SDL_MouseMotionEvent& mouse);
	void MouseWheel(const SDL_MouseWheelEvent& wheel);

private:
	LightSource& lightSource;

	float m_u = 0.0f;
	float m_v = 0.0f;

	float m_distance = 0.0f;
	glm::vec3 m_center = glm::vec3(0.0f);
	glm::vec3 m_worldUp = glm::vec3(0.0f, 0.0f, 1.0f);
	float m_speed = 16.0f;

	// Traveling indicator to different directions.
	float m_goForward = 0.0f;
	float m_goRight = 0.0f;
	float m_goUp = 0.0f;
};