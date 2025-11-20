#include "RayTracer.h"
#include "../includes/ObjParser.h"
#include "../includes/SDL_GLDebugMessageCallback.h"

#include <imgui.h>

RayTracer::RayTracer() {}

RayTracer::~RayTracer() {}

void RayTracer::SetupDebugCallback()
{
	// engedélyezzük és állítsuk be a debug callback függvényt ha debug context-ben vagyunk 
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

static MeshObject<Vertex> createQuad()
{
	MeshObject<Vertex> mesh;

	mesh.vertexArray = {
		{{ 0.5f,  0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{-0.5f,  0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
		{{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
		{{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
		/*{{-0.5f,  0.5f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
		{{ 0.5f,  0.5f, 0.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
		{{ 0.5f, -0.5f, 0.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
		{{-0.5f, -0.5f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},*/
	};

	mesh.indexArray = { 0, 1, 2, 0, 2, 3/*, 4, 6, 5, 6, 4, 7 */};

	return mesh;
}

static MeshObject<Vertex> createCircle(int sides)
{
	MeshObject<Vertex> mesh;

	for (int i = 0; i < sides; ++i)
	{
		float radian = glm::radians(360.0f / sides * i);
		mesh.vertexArray.push_back({ {glm::cos(radian) / 2, glm::sin(radian) / 2, 0}, {0, 1, 0}, {0, 0} });
	}

	for (int i = 1; i < sides - 1; ++i)
	{
		mesh.indexArray.push_back(0);
		mesh.indexArray.push_back(i);
		mesh.indexArray.push_back(i + 1);
	}

	/*for (int i = 0; i < sides; ++i)
	{
		float radian = glm::radians(360.0f / sides * i);
		mesh.vertexArray.push_back({ {glm::cos(radian) / 2, glm::sin(radian) / 2, 0}, {0, -1, 0}, {0, 0} });
	}

	for (int i = sides; i < 2 * sides; ++i)
	{
		mesh.indexArray.push_back(sides);
		mesh.indexArray.push_back(i + 1);
		mesh.indexArray.push_back(i);
	}*/

	return mesh;
}

void RayTracer::InitGeometry()
{
	const std::initializer_list<VertexAttributeDescriptor> vertexAttribList =
	{
		{0, offsetof(Vertex, position), 3, GL_FLOAT},
		{1, offsetof(Vertex, normal), 3, GL_FLOAT},
		{2, offsetof(Vertex, texcoord), 2, GL_FLOAT},
	};

	quad.meshObject = createQuad();
	quad.oglObject = CreateGLObjectFromMesh(quad.meshObject, vertexAttribList);

	circle.meshObject = createCircle(30);
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
	objects.push_back(SceneObject(quad, glm::translate(glm::vec3(0.5, 0, 0)) * glm::scale(glm::vec3(0.1, 1, 1))));
	/*objects.push_back(SceneObject(quad, glm::translate(glm::vec3(-0.5, 0, 0)) * glm::scale(glm::vec3(0.1, 1, 1))));
	objects.push_back(SceneObject(quad, glm::translate(glm::vec3(0, 0.5, 0)) * glm::scale(glm::vec3(1, 0.1, 1))));
	objects.push_back(SceneObject(quad, glm::translate(glm::vec3(0, -0.5, 0)) * glm::scale(glm::vec3(1, 0.1, 1))));*/
}

bool RayTracer::Init()
{
	SetupDebugCallback();

	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

	InitShaders();
	InitGeometry();
	InitTextures();
	InitObjects();

	glEnable(GL_CULL_FACE); // kapcsoljuk be a hátrafelé nézõ lapok eldobását
	glCullFace(GL_BACK);    // GL_BACK: a kamerától "elfelé" nézõ lapok, GL_FRONT: a kamera felé nézõ lapok

	glEnable(GL_DEPTH_TEST); // mélységi teszt bekapcsolása (takarás)

	m_camera.SetView(
		glm::vec3(0.0, 0.0, 2.0),  // honnan nézzük a színteret	   - eye
		glm::vec3(0.0, 0.0, 0.0),  // a színtér melyik pontját nézzük - at
		glm::vec3(0.0, 1.0, 0.0)); // felfelé mutató irány a világban - up

	m_cameraManipulator.SetCamera(&m_camera);

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

	/*if (m_IsPicking) {
		// a felhasználó Ctrl + kattintott, itt kezeljük le
		// sugár indítása a kattintott pixelen át
		Ray ray = CalculatePixelRay(glm::vec2(m_PickedPixel.x, m_PickedPixel.y));
		Intersection intersect;

		m_IsPicking = false;
	}*/

	m_cameraManipulator.Update(updateInfo.DeltaTimeInSec);
}

void RayTracer::SetCommonUniforms()
{
	// - Uniform paraméterek

	// view és projekciós mátrix
	glProgramUniformMatrix4fv(m_programID, ul(m_programID, "viewProj"), 1, GL_FALSE, glm::value_ptr(m_camera.GetViewProj()));
	// - Fényforrások beállítása
	glProgramUniform3fv(m_programID, ul(m_programID, "cameraPosition"), 1, glm::value_ptr(m_camera.GetEye()));
	glProgramUniform1f(m_programID, ul(m_programID, "lightSwitch"), showSceneObjects);

	glProgramUniform1i(m_programID, ul(m_programID, "state"), state);
}

void RayTracer::DrawObject(OGLObject& obj, const glm::mat4& world) {
	glUniformMatrix4fv(ul("world"), 1, GL_FALSE, glm::value_ptr(world));
	glUniformMatrix4fv(ul("worldIT"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(world))));
	glBindVertexArray(obj.vaoID);
	glDrawElements(GL_TRIANGLES, obj.count, GL_UNSIGNED_INT, nullptr);
}

void RayTracer::RenderSceneObject(SceneObject sceneObject)
{
	DrawObject(sceneObject.objContainer.oglObject, sceneObject.transform);
}

void RayTracer::Render()
{
	// töröljük a frampuffert (GL_COLOR_BUFFER_BIT)...
	// ... és a mélységi Z puffert (GL_DEPTH_BUFFER_BIT)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	SetCommonUniforms();

	glUseProgram(m_programID);

	glUniform1i(ul("texImage"), 0);
	//glUniform1i(ul("textureShine"), 1);

	glBindSampler(0, m_SamplerID);
	//glBindSampler(1, m_SamplerID);

	if (showSceneObjects)
		RenderSceneObject(SceneObject(circle, glm::translate(glm::vec3(lightSource.origin, 0)) * glm::scale(glm::vec3(0.03, 0.03, 1))));

	for (const SceneObject& sceneObject : objects)
	{
		if (showSceneObjects)
			RenderSceneObject(sceneObject);

		std::vector<glm::vec2> hits = lightSource.Shine(sceneObject);

		for (glm::vec2 hit : hits)
			RenderSceneObject(SceneObject(circle, glm::translate(glm::vec3(hit.x, hit.y, 0.1)) * glm::scale(glm::vec3(0.02, 0.02, 0.1))));
	}


	glBindTextureUnit(0, 0);
	glBindSampler(0, 0);

	glBindVertexArray(0);
	// shader kikapcsolasa
	glUseProgram(0);
}

void RayTracer::RenderGUI()
{
	if (ImGui::Begin("Variables"))
	{
		ImGui::Checkbox("Show SceneObjects", &showSceneObjects);

		glm::vec3 camPos = m_camera.GetEye();
		ImGui::DragFloat3("Camera position", &camPos.x);
		m_camera.SetView(camPos, m_camera.GetAt(), m_camera.GetWorldUp());

		ImGui::SliderInt("Ray count", &lightSource.rayCount, 0, 20);
		ImGui::DragFloat2("Ray count", &lightSource.origin.x, 0.01, -0.5, 0.5);
	}
	ImGui::End();
}

// https://wiki.libsdl.org/SDL2/SDL_KeyboardEvent
// https://wiki.libsdl.org/SDL2/SDL_Keysym
// https://wiki.libsdl.org/SDL2/SDL_Keycode
// https://wiki.libsdl.org/SDL2/SDL_Keymod

void RayTracer::KeyboardDown(const SDL_KeyboardEvent& key)
{
	if (key.repeat == 0) // Elõször lett megnyomva
	{
		if (key.keysym.sym == SDLK_F5 && key.keysym.mod & KMOD_CTRL)
		{
			CleanShaders();
			InitShaders();
		}
		if (key.keysym.sym == SDLK_F1)
		{
			GLint polygonModeFrontAndBack[2] = {};
			// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGet.xhtml
			glGetIntegerv(GL_POLYGON_MODE, polygonModeFrontAndBack); // Kérdezzük le a jelenlegi polygon módot! Külön adja a front és back módokat.
			GLenum polygonMode = (polygonModeFrontAndBack[0] != GL_FILL ? GL_FILL : GL_LINE); // Váltogassuk FILL és LINE között!
			// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glPolygonMode.xhtml
			glPolygonMode(GL_FRONT_AND_BACK, polygonMode); // Állítsuk be az újat!
		}

		if (key.keysym.sym == SDLK_LCTRL || key.keysym.sym == SDLK_RCTRL)
		{
			m_IsCtrlDown = true;
		}
	}
	m_cameraManipulator.KeyboardDown(key);
}

void RayTracer::KeyboardUp(const SDL_KeyboardEvent& key)
{
	m_cameraManipulator.KeyboardUp(key);
	if (key.keysym.sym == SDLK_LCTRL || key.keysym.sym == SDLK_RCTRL)
	{
		m_IsCtrlDown = false;
	}
}

// https://wiki.libsdl.org/SDL2/SDL_MouseMotionEvent

void RayTracer::MouseMove(const SDL_MouseMotionEvent& mouse)
{
	m_cameraManipulator.MouseMove(mouse);
}

// https://wiki.libsdl.org/SDL2/SDL_MouseButtonEvent

void RayTracer::MouseDown(const SDL_MouseButtonEvent& mouse)
{
	if (m_IsCtrlDown)
	{
		m_IsPicking = true;
	}
	m_PickedPixel = { mouse.x, mouse.y };
}

void RayTracer::MouseUp(const SDL_MouseButtonEvent& mouse)
{

}

// https://wiki.libsdl.org/SDL2/SDL_MouseWheelEvent

void RayTracer::MouseWheel(const SDL_MouseWheelEvent& wheel)
{
	m_cameraManipulator.MouseWheel(wheel);
}

// a két paraméterben az új ablakméret szélessége (_w) és magassága (_h)
// található
void RayTracer::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);
	m_windowSize = glm::uvec2(_w, _h);
	m_camera.SetAspect(static_cast<float>(_w) / _h);
}

// Le nem kezelt, egzotikus esemény kezelése
// https://wiki.libsdl.org/SDL2/SDL_Event

void RayTracer::OtherEvent(const SDL_Event& ev)
{

}