#ifndef RAYTRACER_H
#define RAYTRACER_H

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

// GLEW
#include <GL/glew.h>

// SDL
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

// Utils
#include "../includes/Camera.h"
#include "../includes/ObjectManipulator.h"
#include "../includes/GLUtils.hpp"

// Mine
#include "LightSource.h"
#include "SceneObject.h"

struct SUpdateInfo
{
	float ElapsedTimeInSec = 0.0f;
	float DeltaTimeInSec = 0.0f;
};

struct Intersection
{
	glm::vec2 uv;
	float t;
};

class RayTracer
{
public:
	RayTracer();
	~RayTracer();

	bool Init();
	void Clean();

	void Update(const SUpdateInfo&);
	void Render();
	void RenderGUI();

	void RenderSceneObject(SceneObject object);

	void KeyboardDown(const SDL_KeyboardEvent&);
	void KeyboardUp(const SDL_KeyboardEvent&);
	void MouseMove(const SDL_MouseMotionEvent&);
	void MouseDown(const SDL_MouseButtonEvent&);
	void MouseUp(const SDL_MouseButtonEvent&);
	void MouseWheel(const SDL_MouseWheelEvent&);
	void Resize(int, int);

	void OtherEvent(const SDL_Event&);

protected:
	void SetupDebugCallback();

	// Other data

	float m_ElapsedTimeInSec = 0.0f;

	bool showSceneObjects = false;
	LightSource lightSource = LightSource(glm::vec2(0, 0), 0, 120, 200);

	// Picking

	glm::ivec2 m_PickedPixel = glm::ivec2(0, 0);
	bool m_IsPicking = false;
	bool m_IsCtrlDown = false;

	glm::uvec2 m_windowSize = glm::uvec2(0, 0);

	// Camera

	Camera m_camera;
	ObjectManipulator m_objectManipulator = ObjectManipulator(lightSource.origin, lightSource.direction);

	// Shader

	GLuint m_programID = 0;

	int state = 0;

	void InitShaders();
	void CleanShaders() const;

	// Geometry

	void SetCommonUniforms();
	void DrawObject(OGLObject& obj, const glm::mat4& world);

	ObjectContainer quad;
	ObjectContainer circle;

	std::vector<SceneObject> objects;

	void InitGeometry();
	void CleanGeometry();
	void InitObjects();

	// Texture

	GLuint m_SamplerID = 0;

	void InitTextures();
	void CleanTextures() const;
};

#endif // RAYTRACER_H