#include "RayTracer.h"
#include "../includes/ObjParser.h"
#include "../includes/SDL_GLDebugMessageCallback.h"

#include <imgui.h>

RayTracer::RayTracer() {}

RayTracer::~RayTracer() {}

void RayTracer::SetupDebugCallback()
{

	GLint context_flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
	if (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
		glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, GL_DONT_CARE, 0, nullptr, GL_FALSE);
		glDebugMessageCallback(SDL_GLDebugMessageCallback, nullptr);
	}
}

void RayTracer::InitShaders()
{
	m_programID = glCreateProgram();
	AttachShader(m_programID, GL_VERTEX_SHADER, "../Shaders/Vert_PosNormTex.vert");
	AttachShader(m_programID, GL_FRAGMENT_SHADER, "../Shaders/Frag_ZH.frag");
	LinkProgram(m_programID);
}

void RayTracer::CleanShaders() const
{
	glDeleteProgram(m_programID);
}

static MeshObject<Vertex> CreateQuad()
{
	MeshObject<Vertex> mesh;

	mesh.vertexArray = {
		{{ 0.5f,  0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{-0.5f,  0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
		{{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
		{{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}
	};

	mesh.indexArray = { 0, 1, 2, 0, 2, 3 };

	return mesh;
}

static MeshObject<Vertex> CreateCircle(int sides)
{
	MeshObject<Vertex> mesh;

	for (int i = 0; i < sides; ++i)
	{
		float radian = glm::radians(360.0f / sides * i);
		mesh.vertexArray.push_back({ {glm::cos(radian) / 2, glm::sin(radian) / 2, 0}, {1, 0, 0}, {0, 0} });
	}

	for (int i = 1; i < sides - 1; ++i)
	{
		mesh.indexArray.push_back(0);
		mesh.indexArray.push_back(i);
		mesh.indexArray.push_back(i + 1);
	}

	return mesh;
}

static std::vector<SceneObject> Genearate3DView(glm::vec2 origin, std::vector<glm::vec2> distances, ObjectContainer cube)
{
	const float width = 24.f;
	const float height = 8.f;

	const float halfWidht = width / 2;
	const int fov = distances.size();
	const float degWidth = width / fov;
	const float dislocation = halfWidht - degWidth / 2;

	std::vector<SceneObject> result = {};

	for (int i = 0; i < fov; ++i)
	{
		SceneObject sceneObject = SceneObject(
			cube,
			glm::translate(glm::vec3(degWidth * (fov - i - 1) - dislocation, 0, 0)) *
			glm::scale(glm::vec3(degWidth, height / glm::distance(origin, distances[i]), 1)),
			glm::vec3(1 / glm::distance(origin, distances[i]), 0, 0)
		);

		result.push_back(sceneObject);
	}

	return result;
}

static bool HitPlane(const Ray& ray, const glm::vec3& planeQ, const glm::vec3& planeI, const glm::vec3& planeJ, Intersection& result)
{

	glm::mat3 A(-ray.direction, planeI, planeJ);
	glm::vec3 B = ray.origin - planeQ;

	if (fabsf(glm::determinant(A)) < 1e-6)
		return false;

	glm::vec3 X = glm::inverse(A) * B;

	if (X.x < 0.0)
		return false;

	result.t = X.x;
	result.uv.x = X.y;
	result.uv.y = X.z;

	return true;
}

static Ray CalculatePixelRay(glm::vec2 pixel, glm::vec2 windowSize, Camera camera)
{
	glm::vec3 pickedNDC = glm::vec3(
		2.0f * (pixel.x + 0.5f) / windowSize.x - 1.0f,
		1.0f - 2.0f * (pixel.y + 0.5f) / windowSize.y, 0.0f);

	glm::vec4 pickedWorld = glm::inverse(camera.GetViewProj()) * glm::vec4(pickedNDC, 1.0f);
	pickedWorld /= pickedWorld.w;

	glm::vec3 origin = camera.GetEye();
	Ray ray(origin, glm::vec3(pickedWorld) - origin);

	return ray;
}

void RayTracer::InitGeometry()
{
	const std::initializer_list<VertexAttributeDescriptor> vertexAttribList =
	{
		{0, offsetof(Vertex, position), 3, GL_FLOAT},
		{1, offsetof(Vertex, normal), 3, GL_FLOAT},
		{2, offsetof(Vertex, texcoord), 2, GL_FLOAT},
	};

	quad.meshObject = CreateQuad();
	quad.oglObject = CreateGLObjectFromMesh(quad.meshObject, vertexAttribList);

	circle.meshObject = CreateCircle(30);
	circle.oglObject = CreateGLObjectFromMesh(circle.meshObject, vertexAttribList);
}

void RayTracer::CleanGeometry()
{
	CleanOGLObject(quad.oglObject);
	CleanOGLObject(circle.oglObject);
}

void RayTracer::InitTextures()
{
	glCreateSamplers(1, &m_SamplerID);
	glSamplerParameteri(m_SamplerID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(m_SamplerID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(m_SamplerID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(m_SamplerID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void RayTracer::CleanTextures() const
{
	glDeleteSamplers(1, &m_SamplerID);
}

void RayTracer::InitObjects()
{
	objects.push_back(SceneObject(quad,
		glm::translate(glm::vec3(5.5, 0, 0)) * glm::scale(glm::vec3(1, 10, 1)),
		glm::i8vec3(0, 1, 0)));
	objects.push_back(SceneObject(quad,
		glm::translate(glm::vec3(-5.5, 0, 0)) * glm::scale(glm::vec3(1, 10, 1)),
		glm::i8vec3(0, 1, 0)));
	objects.push_back(SceneObject(quad, glm::translate(glm::vec3(0, 5.5, 0)) * glm::scale(glm::vec3(10, 1, 1)),
		glm::i8vec3(0, 1, 0)));
	objects.push_back(SceneObject(quad, glm::translate(glm::vec3(0, -5.5, 0)) * glm::scale(glm::vec3(10, 1, 1)),
		glm::i8vec3(0, 1, 0)));

	objects.push_back(SceneObject(quad, glm::translate(glm::vec3(1, 1, 0)),
		glm::i8vec3(0, 1, 0)));
	objects.push_back(SceneObject(quad, glm::translate(glm::vec3(-1, -1, 0)),
		glm::i8vec3(0, 1, 0)));
	objects.push_back(SceneObject(quad, glm::translate(glm::vec3(-3, 3, 0)) * glm::scale(glm::vec3(4, 1, 1)),
		glm::i8vec3(0, 1, 0)));
}

bool RayTracer::Init()
{
	SetupDebugCallback();

	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

	InitShaders();
	InitGeometry();
	InitTextures();
	InitObjects();

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);

	m_camera.SetView(
		glm::vec3(0.0, 0.0, 26.0),
		glm::vec3(0.0, 0.0, 0.0),
		glm::vec3(0.0, 1.0, 0.0)
	);

	return true;
}

void RayTracer::Clean()
{
	CleanShaders();
	CleanGeometry();
	CleanTextures();
}


void RayTracer::Update(const SUpdateInfo& updateInfo)
{
	m_ElapsedTimeInSec = updateInfo.ElapsedTimeInSec;

	if (m_IsPicking)
	{

		Ray ray = CalculatePixelRay(glm::vec2(m_PickedPixel.x, m_PickedPixel.y), m_windowSize, m_camera);
		Intersection intersect;
		if (HitPlane(ray, glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), intersect))
			lightSource.origin = intersect.uv;

		m_IsPicking = false;
	}

	m_objectManipulator.Update(updateInfo.DeltaTimeInSec);
}

void RayTracer::SetCommonUniforms()
{
	glProgramUniformMatrix4fv(m_programID, ul(m_programID, "viewProj"), 1, GL_FALSE, glm::value_ptr(m_camera.GetViewProj()));
	glProgramUniform3fv(m_programID, ul(m_programID, "cameraPosition"), 1, glm::value_ptr(m_camera.GetEye()));
	glProgramUniform1f(m_programID, ul(m_programID, "showSceneObjects"), showSceneObjects);
}

void RayTracer::DrawObject(OGLObject& obj, const glm::mat4& world) {
	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(world));
	glUniformMatrix4fv(ul("worldIT"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(world))));
	glBindVertexArray(obj.vaoID);
	glDrawElements(GL_TRIANGLES, obj.count, GL_UNSIGNED_INT, nullptr);
}

void RayTracer::RenderSceneObject(SceneObject sceneObject)
{
	glProgramUniform3fv(m_programID, ul(m_programID, "color"), 1, glm::value_ptr(sceneObject.color));
	DrawObject(sceneObject.objContainer.oglObject, sceneObject.transform);
}

void RayTracer::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	SetCommonUniforms();

	glUseProgram(m_programID);

	glUniform1i(ul("texImage"), 0);


	glBindSampler(0, m_SamplerID);


	std::vector<glm::vec2> hits = lightSource.Shine(objects);

	if (showSceneObjects)
	{
		RenderSceneObject(SceneObject(circle, glm::translate(glm::vec3(lightSource.origin, 0)) * glm::scale(glm::vec3(0.3, 0.3, 1)), glm::vec3(1, 0, 0)));

		for (const SceneObject& sceneObject : objects)
			RenderSceneObject(sceneObject);

		for (glm::vec2 hit : hits)
			RenderSceneObject(SceneObject(circle, glm::translate(glm::vec3(hit.x, hit.y, -0.01)) * glm::scale(glm::vec3(0.2, 0.2, 1)), glm::vec3(1, 0, 0)));
	}

	if (!showSceneObjects)
	{
		for (const SceneObject& sceneObject : Genearate3DView(lightSource.origin, hits, quad))
			RenderSceneObject(sceneObject);
	}


	glBindTextureUnit(0, 0);
	glBindSampler(0, 0);

	glBindVertexArray(0);

	glUseProgram(0);
}

void RayTracer::RenderGUI()
{
	if (ImGui::Begin("Variables"))
	{
		ImGui::Checkbox("Show SceneObjects", &showSceneObjects);
		ImGui::SliderInt("Ray count", &lightSource.rayCount, 0, 200);
		ImGui::SliderFloat2("Position", &lightSource.origin.x, -5, 5);
		ImGui::SliderFloat("Direction", &lightSource.direction, 0, 360);
		ImGui::SliderInt("FOV", &lightSource.fov, 60, 180);
	}
	ImGui::End();
}

void RayTracer::KeyboardDown(const SDL_KeyboardEvent& key)
{
	if (key.repeat == 0)
	{
		if (key.keysym.sym == SDLK_F5 && key.keysym.mod & KMOD_CTRL)
		{
			CleanShaders();
			InitShaders();
		}
		if (key.keysym.sym == SDLK_F1)
		{
			GLint polygonModeFrontAndBack[2] = {};

			glGetIntegerv(GL_POLYGON_MODE, polygonModeFrontAndBack);
			GLenum polygonMode = (polygonModeFrontAndBack[0] != GL_FILL ? GL_FILL : GL_LINE);

			glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
		}

		if (key.keysym.sym == SDLK_LCTRL || key.keysym.sym == SDLK_RCTRL)
		{
			m_IsCtrlDown = true;
		}
	}
	m_objectManipulator.KeyboardDown(key);
}

void RayTracer::KeyboardUp(const SDL_KeyboardEvent& key)
{
	m_objectManipulator.KeyboardUp(key);
	if (key.keysym.sym == SDLK_LCTRL || key.keysym.sym == SDLK_RCTRL)
	{
		m_IsCtrlDown = false;
	}
}

void RayTracer::MouseMove(const SDL_MouseMotionEvent& mouse)
{
	m_objectManipulator.MouseMove(mouse);
}

void RayTracer::MouseDown(const SDL_MouseButtonEvent& mouse)
{
	if (m_IsCtrlDown)
		m_IsPicking = true;

	m_PickedPixel = { mouse.x, mouse.y };
}

void RayTracer::MouseUp(const SDL_MouseButtonEvent& mouse)
{

}

void RayTracer::MouseWheel(const SDL_MouseWheelEvent& wheel)
{

}

void RayTracer::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);
	m_windowSize = glm::uvec2(_w, _h);
	m_camera.SetAspect(static_cast<float>(_w) / _h);
}

void RayTracer::OtherEvent(const SDL_Event& ev)
{

}