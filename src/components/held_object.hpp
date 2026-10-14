#pragma once
#include "rengb_utils.hpp"

namespace rengb{
	class HeldObject
	{
        public:
        float weight = 1.f;
        bool isMainHand = false;
        glm::vec3 restPosition{0.f};
        glm::vec2 rotationalSmooth{0.f};
        glm::vec3 positionalSmooth{0.f};
        glm::vec3 momentumMovement{0.f};

        int arm{1};

        void smoothedMovement(float dt, float pitch, 
            float yaw, glm::vec3 movement, 
            bool crouching, bool onGround, float crouchDistance)
        {
            float const rotationalWeight = 4.f/weight, translationalWeightX = 128.f/weight, translationalWeightY = 32.f/weight, translationalWeightZ = 128.f/weight;
            auto applyMomentumCurve = [](float displacement) -> float // assumes that displacement is clamped -1.f to 1.f
            {
                if (displacement > 0)
                    return (1 + glm::pow<float>(displacement -1, 3))/4; // /3 to get closer to linear curve at low values
                else return -(1 + glm::pow<float>(fabs(displacement) -1, 3))/4;
            };
    
            rotationalSmooth.x += ((pitch - rotationalSmooth.x) * dt * rotationalWeight) * (crouching ? 2.f : 1.f);
            rotationalSmooth.y += ((yaw - rotationalSmooth.y) * dt * rotationalWeight) * (crouching ? 2.f : 1.f);
    
            positionalSmooth.x += (movement.x - positionalSmooth.x) * dt * 8.f;
            positionalSmooth.z += (movement.z - positionalSmooth.z) * dt * 8.f;
            positionalSmooth.y += ((movement.y+crouchDistance) - positionalSmooth.y) * dt * 4.f * (onGround ? 4.f : 1.f);
    
            momentumMovement.y = applyMomentumCurve((pitch - rotationalSmooth.x)/32.f)/4.f -(positionalSmooth.y /translationalWeightY); // division into momentum curve to keep within 1
            momentumMovement.x = applyMomentumCurve((-yaw + rotationalSmooth.y)/32.f)/2.f -(positionalSmooth.x /translationalWeightX); // division into momentum curve for same
            momentumMovement.z = -positionalSmooth.z /translationalWeightZ;
        }
    };    
}
