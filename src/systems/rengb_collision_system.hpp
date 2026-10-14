#pragma once
#include "rengb_system.hpp"
#include "rengb_frame_info.hpp"

#include "components/transform.hpp"
#include "components/collisionshape.hpp"

#include <glm/glm.hpp>

namespace rengb
{
    class CollisionSystem : public System
    {
    public:
        void Update(FrameInfo &frameInfo);

        std::tuple<bool, glm::vec3, float> checkCollision(const SphereCollider &sphere1, const SphereCollider &sphere2, glm::vec3 pos1, glm::vec3 pos2, bool invert);
        std::tuple<bool, glm::vec3, float> checkCollision(const SphereCollider &sphere, const AABB &box, glm::vec3 pos1, glm::vec3 pos2, bool invert);
        std::tuple<bool, glm::vec3, float> checkCollision(const SphereCollider &sphere, const PointCollider &point, glm::vec3 pos1, glm::vec3 pos, bool invert);
        std::tuple<bool, glm::vec3, float> checkCollision(const AABB &box1, const AABB &box2, glm::vec3 pos1, glm::vec3 pos2, bool invert);
        std::tuple<bool, glm::vec3, float> checkCollision(const AABB &box, const PointCollider &point, glm::vec3 pos1, glm::vec3 pos2, bool invert);

        std::tuple<bool, glm::vec3, float> dispatchCollision(
            const std::variant<SphereCollider, AABB, PointCollider>& shape1, 
            const std::variant<SphereCollider, AABB, PointCollider>& shape2, 
            glm::vec3 pos1, 
            glm::vec3 pos2);

    };    
}
