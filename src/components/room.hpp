#pragma once
#include "rengb_utils.hpp"
#include <glm/glm.hpp>
#include <unordered_set>

namespace rengb
{
	struct Room
	{
        std::vector<Entity> doors;
		Entity joiningDoor;
        bool entered;
		int proximity; // 0 is player is in, 1 is it is connect to player room, 3 or more is not rendered at all;
		std::unordered_set<Entity> enemiesIn;
	};
}
