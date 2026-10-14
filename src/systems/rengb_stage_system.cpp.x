#include <iostream>

#include "rengb_stage_system.hpp"

#include "components/collisionshape.hpp"
#include "components/door.hpp"
#include "components/room.hpp"

#include "core/rengb_coordinator.hpp"

namespace rengb
{
    extern Coordinator gCoordinator;

    void StageSystem::Init()
    {

    }

    void StageSystem::Update()
    {
        for (auto const &entity : mEntities)
        {

        }
    }

    void ConstructRooms(uint_fast32_t seed, int stage)
    {
        // switch(stage)
        // {
        //     case 0:

        // }
    }

}

