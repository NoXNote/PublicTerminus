#pragma once
#include <unordered_set>
#include <glm/glm.hpp>
#include "rengb_utils.hpp"

namespace rengb
{
	class EnemyParent
	{
	public:
		float moveSpeed = 8.f;
		float runCycle = 0.f;
		float timeElapsed = 0.f;
		float interpolatedLegAmplitude = 0.f;
		float turning = 0.f;
		glm::vec3 velocity{0.f};
		std::unordered_set<Entity> children;
		float hitpoints{100.f};
		std::string type;
		Entity inRoom;
		Entity weapon;

		std::vector<glm::vec3> pathPositions;
		int indexOn{0};
	};
}
