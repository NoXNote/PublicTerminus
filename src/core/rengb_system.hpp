#pragma once
#include "rengb_utils.hpp"
#include <set>

namespace rengb {
	class System
	{
	public:
		// Entities that fall under system's signature
		std::set<Entity> mEntities;
	};
}
