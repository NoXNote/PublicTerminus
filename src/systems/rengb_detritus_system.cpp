#include <iostream>

#include "rengb_detritus_system.hpp"

#include "components/detritus.hpp"
#include "components/transform.hpp"
#include "components/rigidbody.hpp"
#include "components/collisionshape.hpp"
#include "components/world.hpp"
#include "systems/rengb_traversal_system.hpp"

#include "core/rengb_coordinator.hpp"

namespace rengb
{
    extern Coordinator gCoordinator;
    
    void DetritusSystem::killDet(RengbRenderer &renderer, RengbDevice &device)
    {
        for (auto it = detToKill.begin(); it != detToKill.end();) // needs to use an iterator so that it doesnt get invalidated on removal
		{
            VkFence fence = renderer.getInFlightFence((*it).first);
            VkResult result = vkGetFenceStatus(device.device(), fence);

            if(result == VK_SUCCESS)
            {
                auto &detritus = gCoordinator.GetComponent<Detritus>((*it).second);
                auto &transform = gCoordinator.GetComponent<Transform>((*it).second);

                TraversalSystem::modifyTerrain(detritus.roomIn, transform.translation, device);                
                gCoordinator.DestroyEntity((*it).second);

                it = detToKill.erase(it); 
          
            } else 
            {
                std::cerr<<"Fenced entity!"<<std::endl;

                it = detToKill.erase(it); // may make it act while dead
            }
		}
    }

    void DetritusSystem::Update(FrameInfo &frameInfo, RengbDevice &device)
    {
        for (auto const &entity : mEntities)
        {
            auto &detritus = gCoordinator.GetComponent<Detritus>(entity);
            auto &rigidbody = gCoordinator.GetComponent<RigidBody>(entity);
            auto &transform = gCoordinator.GetComponent<Transform>(entity);

            if(rigidbody.rotTime < 0.f)
            {
                // Set to be destroyed if is on floor too long
                detToKill.push_back({frameInfo.frameIndex, entity});
            }

        }
    }
}

