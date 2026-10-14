// // door system updates what room the player is in, and what rooms are neighbours. 
// // check for events when the player intersects with a a door's hitbox. Store when a player enters.
// // if the player is stored as being in the box one frame, then out the next check what side of the door they ended up in  (half point test) to see if they exited or entered
// // when they enter the door radius, load the next room and open the door. when they exit the door radius, and are on the other side (exited a room)
// // Potentially run second check so that rooms only disappear when you aren't facing them (would see than unload)

#include <iostream>

#include "rengb_door_system.hpp"

#include "components/transform.hpp"
#include "components/door.hpp"
#include "components/world.hpp"
#include "components/room.hpp"
#include "components/sleep.hpp"
#include "components/enemyparent.hpp"
#include "components/terrain.hpp"
#include "components/rigidbody.hpp"
#include "components/renderable.hpp"
#include "components/player.hpp"

#include "core/rengb_coordinator.hpp"

namespace rengb
{
    extern Coordinator gCoordinator;

    void DoorSystem::Init()
    {

    }


    void DoorSystem::Update(Entity playerEnt)
    {
        // door connects from door roomThatIn
        // door connects to door roomToGoto
        auto &player = gCoordinator.GetComponent<Player>(playerEnt);
        auto &playerTransform = gCoordinator.GetComponent<Transform>(playerEnt);

        for (auto const &entity : mEntities)
        {
            
            auto &door = gCoordinator.GetComponent<Door>(entity);
            auto &position = gCoordinator.GetComponent<Transform>(entity);

            auto spherePointCollide = [&] () 
            {
                glm::vec3 vecDistance((playerTransform.translation) - (position.translation));
                float distanceSquared = glm::dot(vecDistance,vecDistance);

                if(distanceSquared < glm::pow(door.radius,2)) // colliding
                {
                    return true;                    
                }
                else return false;
            };

            auto resultingSideOfDoor = [&] ()
            {
                auto dir = playerTransform.translation - position.translation;
                return (float)glm::dot(glm::vec2(position.translation.x, position.translation.z) - glm::vec2(playerTransform.translation.x,playerTransform.translation.z), glm::vec2(glm::sin(position.rotation.y), glm::cos(position.rotation.y))); // distance from plane
            };

            if(spherePointCollide() && !hasEnteredDoor)  // Entering Door Collider
            {
                float sideCheck = resultingSideOfDoor();
                lastSide = sideCheck;
                if(sideCheck > 0) 
                {

                    // we are walking in the direction of the doors connection, so load the room it connects to.
                    // player is in the room that the door connects from
                    // Door
                    //  ^
                    // awaken all enemies sleeping in room connected to
                    gCoordinator.GetComponent<Renderable>(door.roomToGoTo).loading = 20.f;
                    gCoordinator.AddComponent(door.roomToGoTo, World{});
                    for(auto& doorFromRoom : gCoordinator.GetComponent<Room>(door.roomToGoTo).doors)
                    {
                        // load all doors from loaded room, except the one we passed through
                        if(doorFromRoom!=entity)
                        {
                            gCoordinator.GetComponent<Renderable>(doorFromRoom).loading = 20.f;
                            gCoordinator.AddComponent(doorFromRoom, World{});
                        }
                    }
                    for(auto& anEnemy : gCoordinator.GetComponent<Room>(door.roomToGoTo).enemiesIn)
                    {
                        // remove sleep, move to this room, add world
                        gCoordinator.GetComponent<EnemyParent>(anEnemy).inRoom = door.roomToGoTo;
                        gCoordinator.GetComponent<Transform>(anEnemy).translation = gCoordinator.GetComponent<Terrain>(door.roomToGoTo).randomPoint(playerTransform.translation)  + glm::vec3{0.f, 0.4f,0.f};
                        gCoordinator.GetComponent<RigidBody>(anEnemy).velocity = glm::vec3(0.f);
                        gCoordinator.GetComponent<Renderable>(anEnemy).loading = 20.f;

                        gCoordinator.AddComponent(anEnemy, World{});
                        gCoordinator.RemoveComponent<Sleep>(anEnemy);
                    }
                }
                else
                {
                    // we are walking in the direction against the doors connection, so load the room that it connects from
                    // player is in the room that the door connects to
                    //  v
                    // Door
                    gCoordinator.GetComponent<Renderable>(door.roomThatIn).loading = 20.f;
                    gCoordinator.AddComponent(door.roomThatIn, World{});
                    for(auto& doorFromRoom : gCoordinator.GetComponent<Room>(door.roomThatIn).doors)
                    {
                        // load all doors from loaded room, except the one we passed through
                        if(doorFromRoom!=entity)
                        {
                            gCoordinator.GetComponent<Renderable>(doorFromRoom).loading = 20.f;
                            gCoordinator.AddComponent(doorFromRoom, World{});
                        }
                    }
                    // awaken all enemiesin room connecting from
                    for(auto& anEnemy : gCoordinator.GetComponent<Room>(door.roomThatIn).enemiesIn)
                    {
                        // remove sleep, move to this room, add world
                        gCoordinator.GetComponent<EnemyParent>(anEnemy).inRoom = door.roomThatIn;
                        gCoordinator.GetComponent<Transform>(anEnemy).translation = gCoordinator.GetComponent<Terrain>(door.roomThatIn).randomPoint(playerTransform.translation)  + glm::vec3{0.f,-0.4f,0.f};
                        gCoordinator.GetComponent<RigidBody>(anEnemy).velocity = glm::vec3(0.f);
                        gCoordinator.GetComponent<Renderable>(anEnemy).loading = 20.f;
                        

                        gCoordinator.AddComponent(anEnemy, World{});
                        gCoordinator.RemoveComponent<Sleep>(anEnemy);
                    }
                }
                hasEnteredDoor = true; // to avoid checking against wrong door, as doorCurrentlyIn has no way of knowing it is relevent anymore.
                doorCurrentlyIn = entity; // if colliding with door area, say we are in that door.
            }
            if(hasEnteredDoor && entity == doorCurrentlyIn && spherePointCollide() == false) // if the last time we checked we said we were in a door, but now we arent, we must have left.
            {
                // Exiting Door Collider
                float sideCheck = resultingSideOfDoor();
                if(sideCheck > 0) 
                {
                    player.inRoom = door.roomThatIn;
                    // we are leaving the door against the direction it connects to, hence unload the room it connects to
                    // player is in the room that the door connects from
                    // Door
                    //  v
                    // 
                    gCoordinator.RemoveComponent<World>(door.roomToGoTo);
                    for(auto& doorFromRoom : gCoordinator.GetComponent<Room>(door.roomToGoTo).doors)
                    {
                        // remove all doors from unloaded room, except the one we passed through
                        if(doorFromRoom!=entity)
                        {
                            gCoordinator.RemoveComponent<World>(doorFromRoom);
                        }
                    }
                    for(auto& anEnemy : gCoordinator.GetComponent<Room>(door.roomToGoTo).enemiesIn) // unfortunate two togoto's but change later
                    {
                        // add sleep, tell sleep this room, remove world. allow to chase quick by telling which room player went to
                        gCoordinator.AddComponent(anEnemy, Sleep{
                            .roomIn = door.roomToGoTo,
                            .knownRoomToGo = true,
                            .roomToGoTo = door.roomThatIn}); // set timer based on geodesic distance, defaults to 4 seconds.
                        
                        gCoordinator.RemoveComponent<World>(anEnemy);
                    }

                }
                else
                {
                    player.inRoom = door.roomToGoTo;
                    // we are leaving the door in the direction it connects to, hence unload the room it connects from
                    // player is in the room that the door connects to
                    //  ^  
                    // Door
                    gCoordinator.RemoveComponent<World>(door.roomThatIn);
                    for(auto& doorFromRoom : gCoordinator.GetComponent<Room>(door.roomThatIn).doors)
                    {
                        // remove all doors from unloaded room, except the one we passed through
                        if(doorFromRoom!=entity)
                        {
                            gCoordinator.RemoveComponent<World>(doorFromRoom);
                        }
                    }

                    for(auto& anEnemy : gCoordinator.GetComponent<Room>(door.roomThatIn).enemiesIn)
                    {
                        // add sleep, tell sleep this room, remove world. allow to chase quick by telling which room player went to
                        gCoordinator.AddComponent(anEnemy, Sleep{
                            .roomIn = door.roomThatIn,
                            .knownRoomToGo = true,
                            .roomToGoTo = door.roomToGoTo}); // set timer based on geodesic distance, defaults to 4 seconds.
                        
                        gCoordinator.RemoveComponent<World>(anEnemy);
                    }
                }
                hasEnteredDoor = false;
            }
        }
    }

}

