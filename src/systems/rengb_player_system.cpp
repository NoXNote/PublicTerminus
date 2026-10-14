#include "rengb_player_system.hpp"
#include "globals.hpp"
#include <iostream>
#include <limits>

#include "components/rigidbody.hpp"
#include "components/renderable.hpp"
#include "components/detritus.hpp"
#include "components/transform.hpp"
#include "components/world.hpp"
#include "components/player.hpp"
#include "components/collisionshape.hpp"
#include "components/held_object.hpp"
#include "components/gun_object.hpp"
#include "core/rengb_coordinator.hpp"
#include <glm/gtc/epsilon.hpp>


double lastX, lastY;


float const crouchSpeedMult = 0.4f; 

bool firstMouse = true;




namespace rengb
{
    extern Coordinator gCoordinator;


    void PlayerSystem::SwapWeapon(RengbDevice &device, Entity entity, std::string weaponName, RengbAudio& audio)
    {
        auto& player = gCoordinator.GetComponent<Player>(entity);
        auto& playerTransform = gCoordinator.GetComponent<Transform>(entity);
        auto& playerRigid = gCoordinator.GetComponent<RigidBody>(entity);

        auto& gun = gCoordinator.GetComponent<GunObjectComponent>(player.gun);
        auto& gunRenderable = gCoordinator.GetComponent<Renderable>(player.gun);
        auto& gunTransformable = gCoordinator.GetComponent<Transform>(player.gun);

        gunRenderable.loading = -3.f;
        gunRenderable.loadDirection = 2;

        if (gun.sound != nullptr) 
        {
            audio.StopLoop(gun.sound);    
        }

        Entity scrap = gCoordinator.CreateEntity();

        gCoordinator.AddComponent(scrap, RigidBody{
            .velocity = (0.1f,-2.f,0.f) + playerRigid.velocity,
            .acceleration = {0.f,10.f,0.f},
            .rotational = {0.4f,0.f,1.f},
            .mass = 10.f,
            .collision = false,
            .bounce = 1.f,
            .rots = true,
            .rotTime = 0.2f
        });
        gCoordinator.AddComponent(scrap, Detritus{.roomIn = player.inRoom});	
        gCoordinator.AddComponent(scrap,World{});

        glm::mat4 inWorld = playerTransform.mat4() * gunTransformable.mat4();

        gCoordinator.AddComponent(scrap,Transform{
            .translation = {inWorld[3][0],inWorld[3][1],inWorld[3][2]},
            .rotation = playerTransform.rotation + gunTransformable.rotation
        });
        gCoordinator.AddComponent(scrap, Renderable{gunRenderable.model});

        gCoordinator.AddComponent(
            scrap,
            CollisionShape{.colliderCollection{SphereCollider(glm::vec3(0.f), 0.25f)}}
        );

        std::string weaponFilePath = "../data/" + weaponName + ".json"; 
        std::ifstream weaponf(weaponFilePath);
        json weapondata = json::parse(weaponf);


        std::string modelPath{"models/" + weaponName + ".obj"};
        gunRenderable.model = RengbModel::getOrMakeModelFromFile(device, modelPath);
        gun.muzzlePositions.clear();
        for(auto barrel : weapondata["barrels"])
        {
            gun.muzzlePositions.push_back(std::make_tuple(glm::vec3{barrel["posx"], barrel["posy"], barrel["posz"]}, 0.f));
        }

        gun.recoil = weapondata["recoil"];
        gun.rateDelay = weapondata["rateDelay"];
        gun.ID = weapondata["id"];
        gun.type = weapondata["type"];
        gun.damage = weapondata["damage"];
        gun.flashSize = weapondata["flashSize"];
        gun.maxAmmo = weapondata["maxAmmo"];
        gun.ammo = weapondata["maxAmmo"];
        gun.reloadDuration = weapondata["reloadDuration"];
        gun.bulletLook = weapondata["bulletLook"];
        gun.segmentCount = weapondata["segmentCount"];

        //============
        gun.reloadTimer = 0.f;




    }

    void PlayerSystem::Update(GLFWwindow* window, float dt, RengbAudio& audio)
    {
        for (auto const &entity : mEntities)
        {
            // ------camera-------
            auto& transform = gCoordinator.GetComponent<Transform>(entity);
            auto& rigid = gCoordinator.GetComponent<RigidBody>(entity);
            auto& player = gCoordinator.GetComponent<Player>(entity);
            auto& collision = gCoordinator.GetComponent<CollisionShape>(entity);

            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            if (firstMouse) {
                lastX = xpos;
                lastY = ypos;
                firstMouse = false;
            }
            float xoffset = xpos - lastX;
            float yoffset = lastY - ypos;

            lastX = xpos;
            lastY = ypos;

            xoffset *= player.sensitivity;
            yoffset *= player.sensitivity;

            player.yaw += xoffset;
            player.pitch += yoffset;

            player.pitch = glm::clamp(player.pitch, -1.5f, 1.5f);

            transform.rotation.x = player.pitch;
            transform.rotation.y = player.yaw;

            // ----movement-----
            player.onGround = false;
            glm::vec3 floorNormal{0,-1,0};

            for(auto event : collision.collidingWith)
            {
                if(std::get<1>(event).y < -0.2)
                {
                    player.onGround = true;
                    floorNormal = std::get<1>(event);
                    if(player.hasLeftGround)
                    {
                        audio.ShootSound(101, transform.translation + glm::vec3(0.f,10.f,0.f), false); // landing sound
                        player.hasLeftGround = false;
                    }
                }
            }


            const glm::vec3 flatForwardDir{sin(player.yaw), 0.f, cos(player.yaw)};
            const glm::vec3 forwardDir = glm::normalize(flatForwardDir - (player.onGround ? glm::dot(flatForwardDir, floorNormal) * floorNormal : glm::vec3(0.f)));
            const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
            const glm::vec3 upDir{0.f, -1.f, 0.f};

            glm::vec3 moveDir{0.f};
            if(glfwGetKey(window, keys.moveForward) == GLFW_PRESS){
                moveDir += forwardDir;
                player.absoluteMovement.y += 1.f;
            }
            if(glfwGetKey(window, keys.moveBackward) == GLFW_PRESS){
                moveDir -= forwardDir;
                player.absoluteMovement.y -= 1.f;
            }
            if(glfwGetKey(window, keys.moveRight) == GLFW_PRESS){
                moveDir += rightDir;
                player.absoluteMovement.x += 1.f;
            }
            if(glfwGetKey(window, keys.moveLeft) == GLFW_PRESS){
                moveDir -= rightDir;
                player.absoluteMovement.x -= 1.f;
            }
            if(glfwGetKey(window, keys.diagnosis) == GLFW_PRESS)
            {
                player.onGround =  true;
            }
            if(glfwGetKey(window, keys.jump) == GLFW_PRESS && player.onGround)
            {
                if (rigid.velocity.y > -2*player.jumpHeight)
                {
                    rigid.velocity += upDir*player.jumpHeight;
                }

                if(!player.hasLeftGround)
                {
                    audio.ShootSound(101, transform.translation + glm::vec3(0.f,10.f,0.f), false); // jumping sound
                    player.hasLeftGround = true;
                }
            }
            if(glfwGetKey(window, keys.sprint) == GLFW_PRESS)
            {
                player.sprinting = true;

            } else
            {
                player.sprinting = false;
            }


            if(glfwGetKey(window, keys.crouch) == GLFW_PRESS)
            {
                player.crouching = true;
                if(player.onGround)
                {
                    if(player.crouchDistance < player.crouchLimit)
                        player.crouchDistance += (player.crouchLimit - player.crouchDistance) * 10.f * dt; // move toward
                    else 
                        player.crouchDistance = player.crouchLimit;
                    
                    
                }
                else
                {
                    if(player.crouchDistance > std::numeric_limits<float>::epsilon()) 
                        player.crouchDistance -= (player.crouchDistance) * 6.f * dt;
                    else 
                        player.crouchDistance = 0.f;
                }
                
            } else 
            {
                player.crouching = false;
                if(player.crouchDistance > std::numeric_limits<float>::epsilon()) 
                    player.crouchDistance -= (player.crouchDistance) * 6.f * dt;
                else 
                    player.crouchDistance = 0.f;
            }
            std::get<AABB>(collision.colliderCollection[0]).minVertex.y = player.crouchDistance;

            player.targetSpeed = player.walkSpeed;

            if(player.sprinting)
                player.targetSpeed *= 1.5f;
            if(player.crouching)
                player.targetSpeed *= 0.4f;

            if(player.targetSpeed > player.walkSpeed)
                player.moveSpeed += (player.targetSpeed - player.moveSpeed) * dt * 8.f;
            else 
                player.moveSpeed = player.targetSpeed;

            if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) // checks if vec3!=0
            {
                player.absoluteMovement = glm::normalize(player.absoluteMovement) * player.moveSpeed;
                player.bobloop = player.bobloop > glm::two_pi<float>()*2 ? 0.f : player.bobloop + dt*player.moveSpeed*player.bobSpeed;
                player.stepLoop = player.stepLoop > glm::two_pi<float>() ? 0.f : player.stepLoop + dt*player.moveSpeed*player.bobSpeed;
                if(player.onGround)
                {
                    if (player.stepLoop == 0.f) audio.ShootSound(101, transform.translation + glm::vec3(0.f,10.f,0.f), false);
                    player.headBob.y = sin(player.bobloop)/24;
                    player.headBob.x = sin(player.bobloop/2)/32;
                    player.headBob.z = sin(player.bobloop/2)/32;  
       
                } else
                {
                    if(fabs(player.headBob.y) > std::numeric_limits<float>::epsilon())
                    {
                        player.bobloop = 0.f;
                        player.headBob.x-= player.headBob.x*dt*16;
                        player.headBob.y-= player.headBob.y*dt*16;
                        player.headBob.z-= player.headBob.z*dt*16;
                    }
                    else if(player.headBob.y != 0.f) player.headBob.y = 0.f;
                }

                glm::vec3 normalisedMovement = player.moveSpeed * glm::normalize(moveDir);
                rigid.velocity.x = normalisedMovement.x;
                rigid.velocity.z = normalisedMovement.z;
            } else
            {
                player.absoluteMovement = glm::vec3{};
                rigid.velocity.x = 0.f;
                rigid.velocity.z = 0.f;
                
                if(fabs(player.headBob.y) > std::numeric_limits<float>::epsilon())
                {
                    player.bobloop = 0.f;
                    player.headBob.x-= player.headBob.x*dt*16;
                    player.headBob.y-= player.headBob.y*dt*16;
                    player.headBob.z-= player.headBob.z*dt*16;
                }
                else if(player.headBob.y != 0.f) player.headBob.y = 0.f;
            }

            transform.rotation.z += ((player.absoluteMovement.x/64.f) - transform.rotation.z) *4* dt;
            player.velocity = rigid.velocity;

            //===GUN CONTROL=====
            auto& gun = gCoordinator.GetComponent<GunObjectComponent>(player.gun);
            auto& gunTransform = gCoordinator.GetComponent<Transform>(player.gun);
            auto& gunHeld = gCoordinator.GetComponent<HeldObject>(player.gun);


            gun.timeSinceFire += dt;
            gun.shooting = false;

            glm::vec4 gunPosition = {std::get<0>(gun.muzzlePositions[0]), 1.f};
            gunPosition = transform.mat4() * gunTransform.mat4() * gunPosition; 


            if(gun.type == 1)   
            {
                audio.UpdateWindDown(gunPosition);
            }
            audio.UpdateReloadClick(gunPosition);

            auto endFiring = [&] () {
                if(gun.currentlyShooting == true)
                {
                    gun.currentlyShooting = false;         
                    if(gun.type == 1)        
                    {
                        audio.PlayWindDown(gunPosition);
                        if (gun.sound != nullptr) 
                        {
                            audio.StopLoop(gun.sound);       
                            gun.sound = nullptr;                 
                        }
                    } 
                }  
            }; 


            if(gun.reloadTimer > 0.f)
            {
                gun.reloadTimer -= dt;
                audio.PlayReloadClick(gunPosition);
                endFiring();
            } else
            {
                if (gun.ammo == 0)
                {
                    audio.PlayReloadComplete();
                    gun.ammo = gun.maxAmmo;
                    std::cout << "Gun has " << gun.ammo << " bullets" << std::endl;
                    gun.timeSinceFire = 0.f;                 
                } else
                {
                    if(gun.timeSinceFire > gun.passiveReloadDelay && gun.ammo < gun.maxAmmo)
                    {
                        rollingAdder += (dt * gun.maxAmmo)/gun.reloadDuration;
                        if(rollingAdder >= 1.f)
                        {
                            audio.PlayReloadClick(gunPosition);
                            gun.ammo += static_cast<int>(rollingAdder);
                            rollingAdder -= static_cast<int>(rollingAdder);
        
                            if(gun.ammo >= gun.maxAmmo) 
                            {
                                std::cout << "at or over max ammo!!! " << std::endl;
                                gun.ammo = gun.maxAmmo;
                                audio.PlayReloadComplete();
                            }
                        }
        
                    }

                    if((glfwGetMouseButton(window, keys.shoot) == GLFW_PRESS))
                    {
                        if(gun.timeSinceFire > gun.rateDelay)
                        {
                            gun.shoot(dt);  
                            if(gun.type == 0)
                            {
                                std::cout<<"Shooting Normally"<<std::endl;
                                audio.ShootSound(gun.ID, gunPosition, false); //puts in position of first barrel                
                            } else if(gun.type == 1)
                            {



                                if(gun.currentlyShooting == false)
                                {
                                    
                                    gun.currentlyShooting = true;

                                    gun.sound = audio.ShootSound(gun.ID, gunPosition, true);
                                } else
                                {
                                    audio.UpdateSoundPosition(gun.sound, gunPosition);

                                }
                            }
                        }
                    }
                    else endFiring();

                }
            }
    
            gun.wavering.x += (0.f- gun.wavering.x)* dt * 32.f; 
            gun.wavering.y += (0.f- gun.wavering.y)* dt * 16.f; 
            gun.wavering.z += (0.f- gun.wavering.z)* dt * 4.f; 
            gun.recoilMovement.z = -gun.wavering.x;

            gun.recoilMovement -= gun.recoilMovement * dt * 16.f;
    
            if(glfwGetMouseButton(window, keys.sights) == GLFW_PRESS)
            {
                player.downSights = true;
                gun.downSightsMovement.x += (gunHeld.restPosition.x - gun.downSightsMovement.x)* dt * 16.f; 
                gun.downSightsMovement.z += (0.4f-gun.downSightsMovement.z)* dt * 16.f; 
                gun.downSightsMovement.y += (0.1f-gun.downSightsMovement.y)* dt * 16.f; 

                gun.downSightsRotation.z += (-glm::quarter_pi<float>() - gun.downSightsRotation.z) * dt * 16.f;
            } else
            {
                player.downSights = false;
                gun.downSightsMovement.x -= gun.downSightsMovement.x * dt * 8.f; 
                gun.downSightsMovement.z += (0.f- gun.downSightsMovement.z)* dt * 16.f; 
                gun.downSightsMovement.y += (0.f- gun.downSightsMovement.y)* dt * 16.f; 
                gun.downSightsRotation.z -= gun.downSightsRotation.z * dt * 16.f;
            } 
            
            gunTransform.translation += (gun.wavering/2.f) - gun.downSightsMovement;
            gunTransform.rotation += gun.downSightsRotation;
            gunTransform.rotation.x -= gun.wavering.z;
            gunTransform.rotation.y += gun.wavering.x;
            gunTransform.rotation.z += gun.wavering.y;

            auto& playerArm = gCoordinator.GetComponent<Transform>(player.arm);

            playerArm.translation += (gun.wavering/2.f) - gun.downSightsMovement;
            playerArm.rotation += gun.downSightsRotation;
            playerArm.rotation.x -= gun.wavering.z;
            playerArm.rotation.y += gun.wavering.x;
            playerArm.rotation.z += gun.wavering.y;
            


            player.rollingHP += (player.hp - player.rollingHP) * dt * 16.f;
            player.rollingShield += (player.shield - player.rollingShield) * dt * 16.f; 
            player.timeSinceHit += dt;
            if(player.shield != player.maxShield && player.timeSinceHit >= player.shieldRechargeTime)
            {
                player.shield = player.maxShield;
                audio.PlayShieldUp();
            }


            if(glfwGetKey(window, keys.whatever) == GLFW_PRESS)
            {
                rigid.velocity.y=0.f;   
            }

        }

    }
}


