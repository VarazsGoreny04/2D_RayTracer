#ifndef RAY_H
#define RAY_H

#include <vector>
#include <glm/glm.hpp>
#include "../includes/GLUtils.hpp"

class Ray
{
public:
	glm::vec3 origin;
	glm::vec3 direction;

	Ray(glm::vec3 origin, glm::vec3 direction);
	Ray(glm::vec2 origin, glm::vec2 direction);
	~Ray();
};

bool Intersect(Ray& ray, std::vector<glm::vec2> points, glm::vec2& result);

#endif // RAY_H