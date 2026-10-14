#include <iostream>

#include "rengb_gun_system.hpp"

#include "components/gun_object.hpp"
#include "components/transform.hpp"

#include "core/rengb_coordinator.hpp"

namespace rengb
{
    extern Coordinator gCoordinator;

    void GunSystem::Init()
    {

    }

    void GunSystem::Update(float dt)
    {
        // for (auto const &entity : mEntities)
        // {
        //     auto &transform = gCoordinator.GetComponent<Transform>(entity);
        //     auto &gun = gCoordinator.GetComponent<GunObjectComponent>(entity);

        //     gun.timeSinceFire-=dt;
		
        //     if(gun.timeSinceFire < 0)
        //     {
        //         gun.shoot(dt);
        //     }
    
        //     gun.wavering.x += (0.f- gun.wavering.x)* dt * 32.f; 
        //     gun.wavering.y += (0.f- gun.wavering.y)* dt * 16.f; 
        //     gun.wavering.z += (0.f- gun.wavering.z)* dt * 16.f; 
        //     gun.recoilMovement.z = -gun.wavering.x;
        // }
    }
}

