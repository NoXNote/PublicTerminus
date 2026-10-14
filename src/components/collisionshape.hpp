#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <vector>
#include <memory>
#include <variant>
#include <set>
#include <tuple>  

#include "rengb_utils.hpp"

namespace rengb
{
	struct AABB
	{
		glm::vec3 minVertex{};
		glm::vec3 maxVertex{};
		AABB(glm::vec3 min = {}, glm::vec3 max = {}) : minVertex(min), maxVertex(max) {}
	};

	struct SphereCollider
	{
		glm::vec3 centre{};
		float radius{}; 
		SphereCollider(glm::vec3 c = {}, float r = 1.f) : centre(c), radius(r) {}
	};

	struct PointCollider
	{
		glm::vec3 point{};
		PointCollider(glm::vec3 p = {}) : point(p) {}
	};

	struct CollisionEvent
	{
		Entity with;
		glm::vec3 depth;
	};

	struct CollisionShape
	{
		std::vector<std::variant<SphereCollider, AABB, PointCollider>> colliderCollection;
		std::vector<std::tuple<Entity, glm::vec3>> collidingWith;
	};    



}

