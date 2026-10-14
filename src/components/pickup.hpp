#pragma once
#include <unordered_set>
#include <glm/glm.hpp>
#include "rengb_utils.hpp"

namespace rengb
{
	class PickUp
	{
	public:
		bool gravity{false};
        float radius{0.5f};
        float attractDistance{3.f};
        
        std::string give{""};
        float hp{10.f};
        float shield{10.f};
        float exp{10.f};


        glm::vec3 itemOrbitalVelocity{1.f,0.f,0.f};
        glm::vec3 hpOrbitalVelocity{0.f,1.f,0.f};
        glm::vec3 shieldOrbitalVelocity{0.f,0.f,1.f};
        glm::vec3 expOrbitalVelocity{0.f,0.7f,0.7f};

        glm::vec3 itemOrbitalPosition{0.f, -0.5f, -0.5f};
        glm::vec3 hpOrbitalPosition{0.5f, 0.f, 0.5f};
        glm::vec3 shieldOrbitalPosition{0.5f, 0.5f, 0.f};
        glm::vec3 expOrbitalPosition{1.f, 0.f, 0.f};
	};
}
