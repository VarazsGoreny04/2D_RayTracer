#include "Ray.h"

Ray::Ray()
{
	origin = glm::vec2(0);
	direction = glm::vec2(0);
}

Ray::Ray(glm::vec2 origin, glm::vec2 direction)
{
	this->origin = glm::vec3(origin.x, origin.y, 0);
	this->direction = glm::vec3(direction.x, direction.y, 0);
}

Ray::~Ray() {}

/*static bool HitPlane(const Ray& ray, const glm::vec3& planeQ, const glm::vec3& planeI, const glm::vec3& planeJ, Intersection& result)
{
	// sík parametrikus egyenlete: palneQ + u * planeI + v * planeJ
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
}*/

/*static bool HitSphere(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& sphereCenter, float sphereRadius, float& t)
{
	glm::vec3 p_m_c = rayOrigin - sphereCenter;
	float a = glm::dot(rayDir, rayDir);
	float b = 2.0f * glm::dot(rayDir, p_m_c);
	float c = glm::dot(p_m_c, p_m_c) - sphereRadius * sphereRadius;

	float discriminant = b * b - 4.0f * a * c;

	if (discriminant < 0.0f)
	{
		return false;
	}

	float sqrtDiscriminant = sqrtf(discriminant);

	// Mivel 2*a, es sqrt(D) mindig pozitívak, ezért tudjuk, hogy t0 < t1
	float t0 = (-b - sqrtDiscriminant) / (2.0f * a);
	float t1 = (-b + sqrtDiscriminant) / (2.0f * a);

	if (t1 < 0.0f) // mivel t0 < t1, ha t1 negatív, akkor t0 is az
		return false;

	if (t0 < 0.0f)
		t = t1;
	else
		t = t0;

	return true;
}*/

/*static Ray CalculatePixelRay(glm::vec2 pixel, glm::vec2 windowSize, Camera camera)
{
	// NDC koordináták kiszámítása
	glm::vec3 pickedNDC = glm::vec3(
		2.0f * (pixel.x + 0.5f) / windowSize.x - 1.0f,
		1.0f - 2.0f * (pixel.y + 0.5f) / windowSize.y, 0.0f);

	// A világ koordináták kiszámítása az inverz ViewProj mátrix segítségével
	glm::vec4 pickedWorld = glm::inverse(camera.GetViewProj()) * glm::vec4(pickedNDC, 1.0f);
	pickedWorld /= pickedWorld.w; // homogén osztás

	glm::vec3 origin = camera.GetEye();
	Ray ray(origin, glm::vec3(pickedWorld) - origin);

	return ray;
}*/

glm::vec2 Intersect(Ray& ray, Ray & side)
{
	glm::mat2 A(ray.direction, -side.direction);
	glm::vec2 B = side.origin - ray.origin;

	if (fabsf(glm::determinant(A)) < 1e-6)
		return glm::vec2(ray.origin);

	glm::vec2 X = glm::inverse(A) * B;

	if (X.x < 0.0)
		return glm::vec2(ray.origin);
	else
		return ray.direction * X.x + ray.origin;
}

glm::vec2 Intersect(Ray& ray, glm::vec2 a, glm::vec2 b)
{
	Ray &ab = Ray(a, b - a);

	return Intersect(ray, ab);
}

glm::vec2 Intersect(Ray& ray, std::vector<Vertex> points)
{
	int length = points.size();
	std::vector<glm::vec2> intersections = {};

	for (int i = 0; i < length; ++i)
	{
		glm::vec3 pointI = points[i].position;
		glm::vec3 pointINext = points[(i + 1) % length].position;
		intersections.push_back(Intersect(ray, glm::vec2(pointI.x, pointI.y), glm::vec2(pointINext.x, pointINext.y)));
	}

	float minDistance = INFINITY;
	glm::vec2 minIntersection = ray.origin;

	for (int i = 0; i < length; ++i)
	{
		if (intersections[i] == ray.origin)
			continue;

		float distance = glm::distance(ray.origin, intersections[i]);

		if (distance < minDistance)
		{
			minDistance = distance;
			minIntersection = intersections[i];
		}
	}

	return minIntersection;
}