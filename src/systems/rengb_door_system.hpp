#pragma once
#include "rengb_system.hpp"
#include <glm/glm.hpp>

namespace rengb{
    class DoorSystem : public System
    {
    public:
	    void Init();

	    void Update(Entity playerEnt);


        bool hasEnteredDoor{false};
        Entity doorCurrentlyIn{0};
        float lastSide{0.f};

    };
}
