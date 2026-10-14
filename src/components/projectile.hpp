#pragma once
#include "rengb_utils.hpp"
#include <glm/glm.hpp>


namespace rengb
{
	struct Projectile
	{
		std::vector<std::pair<float, glm::vec3>> rotations;
        glm::vec4 colour{0.f, 1.f, 1.f ,10.f};
	};
}
