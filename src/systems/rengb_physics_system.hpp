#pragma once
#include "rengb_system.hpp"
#include <glm/glm.hpp>

namespace rengb{
    class PhysicsSystem : public System
    {
    public:
	    //void Init();

	    void Update(float dt);
	    float friction = 1.5;
	    float simSpeed = 1.f;
    };
}
