#pragma once
#include "rengb_system.hpp"
#include <glm/glm.hpp>
#include <stack>

namespace rengb{
    class StageSystem : public System
    {
    public:
	    void Init();

	    void Update();

        //std::stack<Entity> roomsPassed;
        Entity currentRoom;

        void ConstructRooms(uint_fast32_t seed, int stage);

        int EnemiesLeft;

    };
}
