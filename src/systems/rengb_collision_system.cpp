#include <iostream>

#include "rengb_collision_system.hpp"
#include "core/rengb_coordinator.hpp"
#include "rigidbody.hpp"

#include <algorithm>

namespace rengb
{
    extern Coordinator gCoordinator;

    // void CollisionSystem::Init()
    // {
    // }

    void CollisionSystem::Update(FrameInfo &frameInfo) 
    {
        for (Entity entity : mEntities)
        {
            gCoordinator.GetComponent<CollisionShape>(entity).collidingWith.clear();
        }
        
        for (auto it1 = mEntities.begin(); it1 != mEntities.end(); ++it1)
        {
            auto &transform1 = gCoordinator.GetComponent<Transform>(*it1);
            auto &collision1 = gCoordinator.GetComponent<CollisionShape>(*it1);

            for(auto& shape1 : collision1.colliderCollection)
            {
                //for each shape in object have
                // for each ite2 next etc
                //   and in that have for each shape in obj2
                // avoids checks within the entity

                for (auto it2 = std::next(it1); it2 != mEntities.end(); ++it2)
                {
                    auto &transform2 = gCoordinator.GetComponent<Transform>(*it2);
                    auto &collision2 = gCoordinator.GetComponent<CollisionShape>(*it2);

                    
                    for(auto& shape2 : collision2.colliderCollection)
                    {
                        std::tuple colliding = dispatchCollision(shape1, shape2, transform1.translation, transform2.translation);
                        if(std::get<0>(colliding)) // collision response
                        {
                            // Restitution (elasticity factor, assume 1 for perfectly elastic)
                            float e = 0.1f;
                            
                            glm::vec3 normal = glm::normalize(std::get<1>(colliding));

                            collision1.collidingWith.push_back(std::tuple{*it2, -normal});
                            collision2.collidingWith.push_back(std::tuple{*it1, normal});

                            float depth = std::get<2>(colliding);

                            bool isRigid1 = gCoordinator.GetSignature(*it1).test(gCoordinator.GetComponentType<RigidBody>());
                            bool isRigid2 = gCoordinator.GetSignature(*it2).test(gCoordinator.GetComponentType<RigidBody>());

                            if (isRigid1 && isRigid2) {
                                // Both are rigid bodies
                                auto& rigid1 = gCoordinator.GetComponent<RigidBody>(*it1);
                                auto& rigid2 = gCoordinator.GetComponent<RigidBody>(*it2);
                                if(rigid1.rots) rigid1.rotTime-=frameInfo.frameTime;
                                if(rigid2.rots) rigid2.rotTime-=frameInfo.frameTime;

                                float m1 = rigid1.mass;
                                float m2 = rigid2.mass;

                                glm::vec3 v1 = rigid1.velocity;
                                glm::vec3 v2 = rigid2.velocity;

                                // Relative velocity
                                glm::vec3 relativeVelocity = v2 - v1;
                                float velocityAlongNormal = glm::dot(relativeVelocity, normal);

                                if (velocityAlongNormal < 0)
                                {
                                    // Compute impulse scalar
                                    float j = -(1 + e) * velocityAlongNormal / (1 / m1 + 1 / m2);

                                    // Apply impulse
                                    glm::vec3 impulse = j * normal;
                                    rigid1.velocity -= impulse / m1;
                                    rigid2.velocity += impulse / m2;

                                    // Positional correction to prevent sinking
                                    const float percent = 0.8f; // Correction factor
                                    glm::vec3 correction = percent * depth * normal / (1 / m1 + 1 / m2);
                                    transform1.translation -= correction / m1;
                                    transform2.translation += correction / m2;                            
                                }
                                rigid1.velocity += -rigid1.velocity * frameInfo.frameTime * rigid1.friction;
                                rigid2.velocity += -rigid2.velocity * frameInfo.frameTime * rigid2.friction;
                            } 
                            else if (isRigid1 && !isRigid2) {
                                // it1 is rigid, it2 is static
                                auto& rigid1 = gCoordinator.GetComponent<RigidBody>(*it1);
                                if(rigid1.rots) rigid1.rotTime-=frameInfo.frameTime;

                                
                                // Reflect velocity
                                rigid1.velocity -= (1.0f + e) * glm::dot(rigid1.velocity, normal) * normal;
                                
                                // Positional correction
                                transform1.translation -= depth * normal;
                                glm::vec3 normalVelocity = glm::dot(rigid1.velocity, normal) * normal;
                                glm::vec3 tangentialVelocity = rigid1.velocity - normalVelocity;

                                // Apply friction only to tangential velocity
                                rigid1.velocity = normalVelocity + (tangentialVelocity * (1.0f - rigid1.friction * frameInfo.frameTime));
                            } 
                            else if (!isRigid1 && isRigid2) {
                                 
                                // it2 is rigid, it1 is static
                                auto& rigid2 = gCoordinator.GetComponent<RigidBody>(*it2);
                                if(rigid2.rots) rigid2.rotTime-=frameInfo.frameTime;

                                   
                                // Reflect velocity
                                rigid2.velocity -= (1.0f + e) * glm::dot(rigid2.velocity, normal) * normal;

                                // Positional correction
                                transform2.translation += depth * normal;
                                   
                                glm::vec3 normalVelocity = glm::dot(rigid2.velocity, normal) * normal;
                                glm::vec3 tangentialVelocity = rigid2.velocity - normalVelocity;

                                // Apply friction only to tangential velocity
                                rigid2.velocity = normalVelocity + (tangentialVelocity * (1.0f - rigid2.friction * frameInfo.frameTime));                        
                            }
                        }                        
                    }
                }                
            }


        }

    }
    // =====SPHERE VS SPHERE====
    //	   .ooo.	   .ooo.
    // 	  ooooooo	  ooooooo
    //	  ooooooo	  ooooooo
    //	   'ooo'	   'ooo'

    std::tuple<bool, glm::vec3, float> CollisionSystem::checkCollision(const SphereCollider &sphere1, const SphereCollider &sphere2, glm::vec3 pos1, glm::vec3 pos2, bool invert)
    {
        glm::vec3 vecDistance((sphere2.centre+pos2) - (sphere1.centre+pos1));
        float distanceSquared = glm::dot(vecDistance,vecDistance);
        float radiiSum = sphere1.radius + sphere2.radius;

        if(distanceSquared <= glm::pow(radiiSum, 2)) // colliding
        {
            float centreDistance = glm::sqrt(distanceSquared);
            glm::vec3 collisionNormal;

            if(centreDistance > 0.f) 
                collisionNormal = vecDistance / centreDistance;
            else 
                collisionNormal = glm::vec3(1.f,0.f,0.f); // spheres are on exactly the same spot

            float penetrationDepth = glm::max(radiiSum - centreDistance, 0.f);

            return {true, collisionNormal, penetrationDepth}; // return colliding, across normal, with penetration distance;
        }
        else return {false, glm::vec3{}, {}};
    }
    //======SPHERE VS AABB=====
    //	   .ooo.		#######
    //    ooooooo	  	#######
    //	  ooooooo	  	#######
    //	   'ooo'		#######

    std::tuple<bool, glm::vec3, float> CollisionSystem::checkCollision(const SphereCollider &spherecollider, const AABB &box, glm::vec3 pos1, glm::vec3 pos2, bool invert)
    {
        glm::vec3 boxMin = box.minVertex + pos2;
        glm::vec3 boxMax = box.maxVertex + pos2;
        glm::vec3 sphereCenter = spherecollider.centre + pos1;

        float x = std::max(boxMin.x, std::min(sphereCenter.x, boxMax.x));
        float y = std::max(boxMin.y, std::min(sphereCenter.y, boxMax.y));
        float z = std::max(boxMin.z, std::min(sphereCenter.z, boxMax.z));


        // this is the same as isPointInsideSphere
        float distanceSquared =
            glm::pow(x - sphereCenter.x, 2) +
            glm::pow(y - sphereCenter.y, 2) +
            glm::pow(z - sphereCenter.z, 2);


        glm::vec3 vecDistance = glm::vec3(x, y, z) - sphereCenter;

        if(distanceSquared < glm::pow(spherecollider.radius,2)) // colliding
        {
            float centreDistance = glm::sqrt(distanceSquared);
            glm::vec3 collisionNormal;

            if(centreDistance > 0.f)
                collisionNormal = vecDistance / centreDistance;
            else
                collisionNormal = glm::vec3(1.f,0.f,0.f); // spheres and point are on exactly the same spot



            float penetrationDepth = glm::max(spherecollider.radius - centreDistance, 0.f);

            return {true, (invert ? -1.f : 1.f) *collisionNormal,  penetrationDepth}; 
        }
        else return {false, glm::vec3{}, {}};
    }
    //======SPHERE VS POINT COLLIDER=====
    //   .ooo.			
    //	ooooooo	  		*
    //  ooooooo	  	
    //   'ooo'			
    std::tuple<bool, glm::vec3, float> CollisionSystem::checkCollision(const SphereCollider &spherecollider, const PointCollider &pointcollider, glm::vec3 pos1, glm::vec3 pos2, bool invert)
    {

        glm::vec3 vecDistance((pointcollider.point+pos2) - (spherecollider.centre+pos1));
        float distanceSquared = glm::dot(vecDistance,vecDistance);

        if(distanceSquared < glm::pow(spherecollider.radius,2)) // colliding
        {
            float centreDistance = glm::sqrt(distanceSquared);
            glm::vec3 collisionNormal;

            if(centreDistance > 0.f)
                collisionNormal = vecDistance / centreDistance;
            else
                collisionNormal = glm::vec3(1.f,0.f,0.f); // spheres and point are on exactly the same spot

            return {true, (invert ? -1.f : 1.f) * collisionNormal, (spherecollider.radius - centreDistance)}; // made negative distance to fix point bug
        }
        else return {false, glm::vec3{}, {}};
    }
    //=====AABB VS AABB=====
    //	#######		#######			
    //	#######		#######	
    //	#######	 	#######	 	
    //	#######		#######	
    std::tuple<bool, glm::vec3, float> CollisionSystem::checkCollision(const AABB &box1, const AABB &box2, glm::vec3 pos1, glm::vec3 pos2, bool invert)
    {

        glm::vec3 box1Min = box1.minVertex + pos1;
        glm::vec3 box1Max = box1.maxVertex + pos1;
        glm::vec3 box2Min = box2.minVertex + pos2;
        glm::vec3 box2Max = box2.maxVertex + pos2;

        if (box1Max.x > box2Min.x && box1Min.x < box2Max.x &&
            box1Max.y > box2Min.y && box1Min.y < box2Max.y &&
            box1Max.z > box2Min.z && box1Min.z < box2Max.z) {

            // Compute penetration depth along each axis
            float overlapX = std::min(box1Max.x - box2Min.x, box2Max.x - box1Min.x);
            float overlapY = std::min(box1Max.y - box2Min.y, box2Max.y - box1Min.y);
            float overlapZ = std::min(box1Max.z - box2Min.z, box2Max.z - box1Min.z);

            // Find the axis of minimum penetration
            float minOverlap = std::min({overlapX, overlapY, overlapZ});
            glm::vec3 collisionNormal(0.f);

            if (std::abs(minOverlap - overlapX) < 1e-6) 
                collisionNormal = glm::vec3((box1Min.x < box2Min.x) ? -1.f : 1.f, 0.f, 0.f);
            else if (std::abs(minOverlap - overlapY) < 1e-6) 
                collisionNormal = glm::vec3(0.f, (box1Min.y < box2Min.y) ? -1.f : 1.f, 0.f);
            else if (std::abs(minOverlap - overlapZ) < 1e-6) 
                collisionNormal = glm::vec3(0.f, 0.f, (box1Min.z < box2Min.z) ? -1.f : 1.f);

            return {true, -collisionNormal, minOverlap};
        }

        return {false, glm::vec3(0.f), 0.f};

    }
    //======AABB VS POINT COLLIDER====
    //	#######				
    //	#######			*
    //	#######	 		
    //	#######		
    std::tuple<bool, glm::vec3, float> CollisionSystem::checkCollision(const AABB &box, const PointCollider &pointcollider, glm::vec3 pos1, glm::vec3 pos2, bool invert)
    {
        // actual positions
        glm::vec3 boxMin = box.minVertex + pos1;
        glm::vec3 boxMax = box.maxVertex + pos1;
        glm::vec3 pointPos = pointcollider.point + pos2;

        if(	pointPos.x >= boxMin.x && pointPos.x <= boxMax.x &&
            pointPos.y >= boxMin.y && pointPos.y <= boxMax.y &&
            pointPos.z >= boxMin.z && pointPos.z <= boxMax.z)
        {
            // closest face normal
            float distLeft   = pointPos.x - boxMin.x;
            float distRight  = boxMax.x - pointPos.x;
            float distBottom = pointPos.y - boxMin.y;
            float distTop    = boxMax.y - pointPos.y;
            float distBack   = pointPos.z - boxMin.z;
            float distFront  = boxMax.z - pointPos.z;

            // find the closest face 
            float minDist = std::min({distLeft, distRight, distBottom, distTop, distBack, distFront});
            

            glm::vec3 normal(0.f);

            if (minDist == distLeft) normal = glm::vec3(-1.f, 0.f, 0.f);
            else if (minDist == distRight) normal = glm::vec3(1.f, 0.f, 0.f);
            else if (minDist == distBottom) normal = glm::vec3(0.f, -1.f, 0.f);
            else if (minDist == distTop) normal = glm::vec3(0.f, 1.f, 0.f);
            else if (minDist == distBack) normal = glm::vec3(0.f, 0.f, -1.f);
            else if (minDist == distFront) normal = glm::vec3(0.f, 0.f, 1.f);

            return {true, (invert ? -1.f : 1.f) * normal, minDist}; // made min dist negative to fix point collisions
        } else return {false, glm::vec3{}, 0.f};

    }

    std::tuple<bool, glm::vec3, float> CollisionSystem::dispatchCollision(
        const std::variant<SphereCollider, AABB, PointCollider> &shape1, 
        const std::variant<SphereCollider, AABB, PointCollider> &shape2,
        glm::vec3 pos1, glm::vec3 pos2)
    {
        return std::visit([&](auto &&collider1) -> std::tuple<bool, glm::vec3, float>
                        { return std::visit([&](auto &&collider2) -> std::tuple<bool, glm::vec3, float>
                                            {
                using T1 = std::decay_t<decltype(collider1)>;
                using T2 = std::decay_t<decltype(collider2)>;
                if constexpr (std::is_same_v<T1, SphereCollider> && std::is_same_v<T2, SphereCollider>) 
                {
                    return checkCollision(collider1, collider2, pos1, pos2, false); // SPHERE VS SPHERE
                } else if constexpr (std::is_same_v<T1, SphereCollider> && std::is_same_v<T2, AABB>) 
                {
                    return checkCollision(collider1, collider2, pos1, pos2, false); // SPHERE VS AABB
                } else if constexpr (std::is_same_v<T1, AABB> && std::is_same_v<T2, SphereCollider>) 
                {

                    return checkCollision(collider2, collider1, pos2, pos1, true); // symmetry SPHERE VS AABB
                } else if constexpr (std::is_same_v<T1, AABB> && std::is_same_v<T2, AABB>) 
                {
                    return checkCollision(collider1, collider2, pos1, pos2, false); // AABB VS AABB
                } else if constexpr (std::is_same_v<T1, SphereCollider> && std::is_same_v<T2, PointCollider>) 
                {
                    return checkCollision(collider1, collider2, pos1, pos2, false); // SPHERE VS POINT
                } else if constexpr (std::is_same_v<T1, PointCollider> && std::is_same_v<T2, SphereCollider>) 
                {

                    return checkCollision(collider2, collider1, pos2, pos1, true); // symmetry SPHERE VS POINT
                } else if constexpr (std::is_same_v<T1, AABB> && std::is_same_v<T2, PointCollider>) 
                {
                    return checkCollision(collider1, collider2, pos1, pos2, false); // AABB VS Point
                } else if constexpr (std::is_same_v<T1, PointCollider> && std::is_same_v<T2, AABB>) 
                {

                    return checkCollision(collider2, collider1, pos2, pos1, true); // symmetry AABB VS Point
                } else {
                    // Default case: Unsupported types
                    return {false, glm::vec3{}, {}};
                } }, shape2); }, shape1);
    }
}