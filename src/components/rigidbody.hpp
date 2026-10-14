#pragma once

#include <glm/glm.hpp>

namespace rengb{
	struct RigidBody
	{
		glm::vec3 velocity{0.f};
		glm::vec3 acceleration{0.f};
		glm::vec3 force{0.f};
		glm::vec3 impulse{0.f};
		glm::vec3 rotational{0.f};
		float mass = 1.f;
		bool collision = true;
		float bounce{};
		float friction{4};
		bool transformable{true};
		bool rots{false};
		float rotTime{2.f};
	};    
}

