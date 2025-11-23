#include "Ray.h"

Ray::Ray(glm::vec3 origin, glm::vec3 direction) : origin(origin), direction(direction) {}

Ray::Ray(glm::vec2 origin, glm::vec2 direction)
{
	this->origin = glm::vec3(origin, 0);
	this->direction = glm::vec3(direction, 0);
}

Ray::~Ray() {}

static bool OutOfSection(float section1, float section2, float value)
{
	if (section2 > section1)
	{
		float temp = section1;
		section1 = section2;
		section2 = temp;
	}

	section1 += 1e-6f;
	section2 -= 1e-6f;

	return section2 > value || value > section1;
}

static bool OutOfSection(glm::vec2 boxBound1, glm::vec2 boxBound2, glm::vec2 value)
{
	if (glm::distance(boxBound1.x, boxBound2.x) > glm::distance(boxBound1.y, boxBound2.y))
		return OutOfSection(boxBound1.x, boxBound2.x, value.x);
	else
		return OutOfSection(boxBound1.y, boxBound2.y, value.y);
}

static bool Intersect(Ray& ray, Ray& side, glm::vec2& result)
{
	glm::mat2 A(ray.direction, -side.direction);
	glm::vec2 B = side.origin - ray.origin;

	if (fabsf(glm::determinant(A)) < 1e-6)
		return false;

	glm::vec2 X = glm::inverse(A) * B;

	if (X.x < 0.0)
		return false;
	else
	{
		result = ray.direction * X.x + ray.origin;
		return true;
	}
}

static bool Intersect(Ray& ray, glm::vec2 a, glm::vec2 b, glm::vec2& result)
{
	Ray& ab = Ray(a, b - a);

	return Intersect(ray, ab, result) && !OutOfSection(a, b, result);
}

bool Intersect(Ray& ray, std::vector<glm::vec2> points, glm::vec2& result)
{
	int length = static_cast<int>(points.size());
	std::vector<glm::vec2> intersections = {};

	for (long i = 0; i < length; ++i)
	{
		glm::vec2 pointI = points[i];
		glm::vec2 pointINext = points[(i + 1l) % length];
		glm::vec2 intersection;

		if (Intersect(ray, pointI, pointINext, intersection))
			intersections.push_back(intersection);
	}

	float minDistance = INFINITY;

	for (glm::vec2 intersection : intersections)
	{
		float distance = glm::distance(ray.origin, glm::vec3(intersection, 0));

		if (distance < minDistance)
		{
			minDistance = distance;
			result = intersection;
		}
	}

	return minDistance < INFINITY;
}