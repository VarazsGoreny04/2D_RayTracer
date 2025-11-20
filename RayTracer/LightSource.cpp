#include "LightSource.h"

LightSource::LightSource(glm::vec2 origin, int rayCount)
{
	this->origin = origin;
	this->rayCount = rayCount;
}

static Ray CastRay(glm::vec2 origin, float angle)
{
	Ray ray{
		ray.origin = origin,
		ray.direction = glm::vec2(glm::cos(angle), glm::sin(angle))
	};

	return ray;
}

std::vector<Ray> LightSource::CastRays()
{
	std::vector<Ray> rays(rayCount);

	for (int i = 0; i < rayCount; ++i)
	{
		float radian = glm::radians(360.0f / rayCount * i);
		rays[i] = CastRay(origin, radian);
	}

	return rays;
}

std::vector<glm::vec2> LightSource::Shine(SceneObject sceneObject)
{
	std::vector<glm::vec2> points = {};

	for (Vertex vertex : sceneObject.objContainer.meshObject.vertexArray)
	{
		points.push_back(vertex.position * sceneObject.transform);
	}

	std::vector<glm::vec2> intersections = {};

	for (Ray ray : CastRays())
	{
		//intersections.push_back(ray.direction + ray.origin);
		intersections.push_back(Intersect(ray, points));
	}

	return intersections;
}