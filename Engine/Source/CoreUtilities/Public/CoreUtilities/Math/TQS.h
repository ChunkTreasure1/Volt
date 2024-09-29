#pragma once

#include <glm/glm.hpp>

struct TQS
{
	glm::vec3 translation = 0.f;
	glm::quat rotation = glm::identity<glm::quat>();
	glm::vec3 scale = 1.f;
};
