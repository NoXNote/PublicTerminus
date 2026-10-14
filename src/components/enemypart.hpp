#pragma once
#include "rengb_utils.hpp"

namespace rengb{
	struct EnemyPart
	{
		Entity parent{}; // the entity which holds this body part
		glm::vec3 rootPosition{}; // where on the body the part is located
		float HP; // number of hits part can take before breaking and falling off
		std::string type; // what function the part has
	};    
}
