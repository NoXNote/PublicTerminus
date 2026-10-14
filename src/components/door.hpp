#pragma once
#include "rengb_utils.hpp"
#include <glm/glm.hpp>

namespace rengb
{
	struct Door
	{
		Entity roomThatIn;
		Entity roomToGoTo;
		float radius{6.f};
		int orientation;
	};
}
