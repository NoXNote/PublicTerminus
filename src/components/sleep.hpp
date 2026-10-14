#pragma once
#include <glm/glm.hpp>
#include "rengb_utils.hpp"

namespace rengb{
	class Sleep
	{
		public:
            Entity roomIn;
			bool traversing{false};
			bool knownRoomToGo{false};
			Entity roomToGoTo{0};
            float timeToTravel{4.f};
            float currentTime{4.f};  
	};    
}
