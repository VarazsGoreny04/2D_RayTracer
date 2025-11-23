#include "LightSource.h"

LightSource::LightSource(glm::vec2 origin, float direction, int fov, int rayCount) : origin(origin), direction(direction), fov(fov), rayCount(rayCount) {}

static std::vector<std::vector<glm::vec2>> GetAllPoints(std::vector<SceneObject> sceneObjects)
{
	std::vector<std::vector<glm::vec2>> allPoints = {};

	for (const SceneObject& sceneObject : sceneObjects)
	{
		std::vector<glm::vec2> points = {};

		for (const Vertex& vertex : sceneObject.objContainer.meshObject.vertexArray)
			points.push_back(sceneObject.transform * glm::vec4(vertex.position, 1));

		allPoints.push_back(points);
	}

	return allPoints;
}

static Ray CastRay(glm::vec2 origin, float angle)
{
	Ray ray{
		ray.origin = glm::vec3(origin, 0),
		ray.direction = glm::vec3(glm::cos(angle), glm::sin(angle), 0)
	};

	return ray;
}

static std::vector<Ray> CastRays(glm::vec2 origin, float direction, int fov, int rayCount)
{
	std::vector<Ray> rays = {};
	float halfOfRayCount = (rayCount - 1.f) / 2.f;

	for (float i = -halfOfRayCount; i <= halfOfRayCount; ++i)
	{
		float radian = glm::radians(static_cast<float>(fov) / rayCount * i + direction);
		rays.push_back(CastRay(origin, radian));
	}

	return rays;
}

static bool ClosestIntersection(Ray& ray, std::vector<std::vector<glm::vec2>> allPoints, glm::vec2& result)
{
	float minDistance = INFINITY;

	for (std::vector<glm::vec2> points : allPoints)
	{
		glm::vec2 intersection;
		if (Intersect(ray, points, intersection))
		{
			float distance = glm::distance(ray.origin, glm::vec3(intersection, 0));

			if (distance < minDistance)
			{
				minDistance = distance;
				result = intersection;
			}
		}
	}

	return minDistance < INFINITY;
}

std::vector<glm::vec2> LightSource::Shine(std::vector<SceneObject> sceneObjects) const
{
	std::vector<std::vector<glm::vec2>> allPoints = GetAllPoints(sceneObjects);

	std::vector<glm::vec2> intersections = {};

	for (Ray ray : CastRays(origin, direction, fov, rayCount))
	{
		glm::vec2 intersection;

		if (ClosestIntersection(ray, allPoints, intersection))
			intersections.push_back(intersection);
	}

	return intersections;
}