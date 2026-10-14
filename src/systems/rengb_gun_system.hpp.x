#pragma once
#include "rengb_system.hpp"
#include <glm/glm.hpp>

namespace rengb{
    class GunSystem : public System
    {
    public:
	    void Init();

	    void Update(float dt);

    };
}
