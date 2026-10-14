#include <iostream>

#include "rengb_physics_system.hpp"

#include "components/rigidbody.hpp"
#include "components/transform.hpp"
#include "components/collisionshape.hpp"
#include "components/terrain.hpp"
#include "systems/rengb_traversal_system.hpp"

#include "core/rengb_coordinator.hpp"

namespace rengb
{
    extern Coordinator gCoordinator;

    void PhysicsSystem::Update(float dt)
    {
        float frameTime = dt;

        for (auto const &entity : mEntities)
        {
            auto &rigidBody = gCoordinator.GetComponent<RigidBody>(entity);

            auto &transform = gCoordinator.GetComponent<Transform>(entity);

            rigidBody.velocity += (rigidBody.acceleration+(rigidBody.force/rigidBody.mass)) * frameTime * simSpeed;
            rigidBody.velocity += rigidBody.impulse/rigidBody.mass;
            glm::vec3 step = rigidBody.velocity * frameTime * simSpeed;

            transform.translation += step;
            transform.rotation += rigidBody.rotational * dt;

        }
    }

}

