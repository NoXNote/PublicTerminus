#include <iostream>
#include <cmath>

#include "rengb_enemy_system.hpp"
#include "components/renderable.hpp"
#include "components/transform.hpp"
#include "components/enemyparent.hpp"
#include "components/enemypart.hpp"
#include "components/rigidbody.hpp"
#include "components/world.hpp"
#include "components/collisionshape.hpp"
#include "components/player.hpp"
#include "components/gun_object.hpp"
#include "components/projectile.hpp"
#include "components/terrain.hpp"
#include "components/room.hpp"
#include "components/detritus.hpp"
#include "components/pickup.hpp"

#include "core/rengb_coordinator.hpp"
#include "systems/rengb_traversal_system.hpp"
#include <json.hpp>
#include <vector>
#include <fstream>

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/quaternion.hpp>

using json = nlohmann::json;

namespace rengb
{
	extern Coordinator gCoordinator;


	struct SimplePushConstantData
	{
		glm::mat4 modelMatrix{1.f};
		glm::vec4 timeSpeedAmplitudeTurning;
		glm::vec4 playerPositionWeight;
	};

	EnemySystem::EnemySystem(RengbDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout): rengbDevice{device}
	{
		createPipelineLayout(globalSetLayout);
		createPipelines(renderPass);
	}

	void EnemySystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(rengbDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
			VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void EnemySystem::createPipelines(VkRenderPass renderPass)
	{
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		specializationEntry.constantID = 0;   // Matches GLSL `constant_id = 0`
		specializationEntry.offset = 0;
		specializationEntry.size = sizeof(int);
		
		for (int partID : partIDs) 
		{
    		int partIDValue = partID;

			VkSpecializationInfo specializationInfo{};
			specializationInfo.mapEntryCount = 1;
			specializationInfo.pMapEntries = &specializationEntry;
			specializationInfo.dataSize = sizeof(int);
			specializationInfo.pData = &partIDValue;

			PipelineConfigInfo pipelineConfig{};
			RengbPipeline::defaultPipelineConfigInfo(pipelineConfig);
			pipelineConfig.renderPass = renderPass;
			pipelineConfig.pipelineLayout = pipelineLayout;

			pipelines.push_back(std::make_unique<RengbPipeline>(
				rengbDevice,
				"shaders/enemy.vert.spv",
				"shaders/enemy.frag.spv", 
				pipelineConfig,
				&specializationInfo
			));
		}
	}

	void EnemySystem::pathEnemies(std::shared_ptr<RengbModel> vaser)// Takes a model, used to trace a path through.
	{

		for (auto const &entity : mEntities)
		{
			auto &transform = gCoordinator.GetComponent<Transform>(entity);
			auto &rigid = gCoordinator.GetComponent<RigidBody>(entity);
			auto &enemyparent = gCoordinator.GetComponent<EnemyParent>(entity);

			enemyparent.pathPositions = TraversalSystem::getPathPositions(enemyparent.inRoom, transform.translation);
			for(auto position : enemyparent.pathPositions)
			{
				Entity vase = gCoordinator.CreateEntity();
				gCoordinator.AddComponent(vase, Renderable{.model = vaser});
				gCoordinator.AddComponent(vase, World{});
				gCoordinator.AddComponent(vase, Transform{.translation = position});
			}
			enemyparent.indexOn = 0;
			
		}
	}

	void EnemySystem::pathEnemies()
	{

		for (auto const &entity : mEntities)
		{
			auto &transform = gCoordinator.GetComponent<Transform>(entity);
			auto &rigid = gCoordinator.GetComponent<RigidBody>(entity);
			auto &enemyparent = gCoordinator.GetComponent<EnemyParent>(entity);

			enemyparent.pathPositions = TraversalSystem::getPathPositions(enemyparent.inRoom, transform.translation);

			enemyparent.indexOn = 0;
			
		}
	}

	void EnemySystem::followPath(FrameInfo &frameInfo, glm::vec3 playerPosition)
	{
		for (auto const &entity : mEntities)
		{
			auto &transform = gCoordinator.GetComponent<Transform>(entity);
			auto &rigid = gCoordinator.GetComponent<RigidBody>(entity);
			auto &collision = gCoordinator.GetComponent<CollisionShape>(entity);
			auto &enemyparent = gCoordinator.GetComponent<EnemyParent>(entity);



			if(enemyparent.pathPositions.size() > 0)
			{
				if(glm::dot((enemyparent.pathPositions[enemyparent.indexOn] - transform.translation), (enemyparent.pathPositions[enemyparent.indexOn] - transform.translation)) < 2.f)
				{
					if(enemyparent.indexOn != enemyparent.pathPositions.size()-1)
					{
						++enemyparent.indexOn;
					} else
					{
						rigid.velocity -= rigid.velocity * frameInfo.frameTime;	
					}
					

				} else
				{

					rigid.velocity += ((32.f * glm::normalize(enemyparent.pathPositions[enemyparent.indexOn] - transform.translation)) - rigid.velocity )* frameInfo.frameTime * 4.f;	

				}		

				glm::vec2 dir{rigid.velocity.x,rigid.velocity.z};
				enemyparent.moveSpeed = glm::sqrt(glm::dot(dir,dir));

			}
			if (glm::dot(rigid.velocity, rigid.velocity) > std::numeric_limits<float>::epsilon()) // checks if moving
			{
				transform.rotation.y += (glm::eulerAngles(glm::rotation({0.f,0.f,1.f}, glm::normalize(rigid.velocity))).y - transform.rotation.y) * frameInfo.frameTime * 16.f;
				transform.rotation.x = 0.f;
				transform.rotation.z = 0.f;
			} else
			{
				std::cout << "pointing to player" <<  std::endl;
				transform.rotation += (glm::eulerAngles(glm::rotation({0.f,0.f,1.f}, glm::normalize(playerPosition - transform.translation))) - transform.rotation) * frameInfo.frameTime * 8.f;
				transform.rotation.x = 0.f;
				transform.rotation.z = 0.f;
			}


			enemyparent.velocity = rigid.velocity;

		}
	}

	void EnemySystem::explode(FrameInfo &frameInfo, Entity entity)
	{
		deadEnemies.push_back({frameInfo.frameIndex, entity});
	}

	void EnemySystem::killEnemies(RengbRenderer &renderer, RengbDevice &device)
	{
		std::random_device rd;
		std::mt19937 generator(rd());

		std::uniform_real_distribution<float> randVelX(-8.f, 8.f);
		std::uniform_real_distribution<float> randVelY(-8.f, 8.f);
		std::uniform_real_distribution<float> randVelZ(-8.f, 8.f);

        std::ifstream file("../data/enemy_positions.json");
		json data = json::parse(file);


	
        for (auto it = deadEnemies.begin(); it != deadEnemies.end();) // needs to use an iterator so that it doesnt get invalidated on removal
		{
			auto &parenttransform = gCoordinator.GetComponent<Transform>((*it).second);

			if (gCoordinator.HasComponent<EnemyParent>((*it).second))
			{
				auto &parent = gCoordinator.GetComponent<EnemyParent>((*it).second);
				auto &parentrigid = gCoordinator.GetComponent<RigidBody>((*it).second);
				

				parentrigid.force = glm::vec3(0.f);
				parentrigid.velocity = glm::vec3(0.f);
				for (auto it = parent.children.begin(); it != parent.children.end();) // needs to use an iterator so that it doesnt get invalidated on removal
				{
					auto child = *it;  // Capture the current value
					auto &childTransform = gCoordinator.GetComponent<Transform>(child);
					auto &childPart = gCoordinator.GetComponent<EnemyPart>(child);
		
					glm::vec3 position = glm::vec3(data[childPart.type]["x"], -(float)(data[childPart.type]["z"]),data[childPart.type]["y"]);

		
					childTransform.translation = parenttransform.translation;
					gCoordinator.RemoveComponent<EnemyPart>(child);
					gCoordinator.AddComponent(
						child,
						CollisionShape{.colliderCollection{SphereCollider(position, data[childPart.type]["radius"])}}
					);
		
					gCoordinator.AddComponent(child, Detritus{.roomIn = parent.inRoom});		
		
					gCoordinator.AddComponent(
						child,
						RigidBody{
						.velocity = (parent.velocity/256.f) + glm::vec3(randVelX(generator), randVelY(generator), randVelZ(generator)),
						.acceleration = {0,10,0},
						.rotational = {0.f,0.f,0.f}, 
						.mass = 10.f,
						.collision = false,
						.bounce = 1.f,
						.rots = true});
					gCoordinator.AddComponent(child,World{});
					it = parent.children.erase(it); 
				}

				gCoordinator.GetComponent<Room>(parent.inRoom).enemiesIn.erase((*it).second);
			}


			
            VkFence fence = renderer.getInFlightFence((*it).first);
            VkResult result = vkGetFenceStatus(device.device(), fence);
            if(result == VK_SUCCESS)
            {
				Entity drop = gCoordinator.CreateEntity();

				glm::vec3 itemV = glm::vec3(16.f + randVelX(generator), randVelY(generator), randVelZ(generator)), 
				hpV = glm::vec3(randVelX(generator), 16.f + randVelY(generator), randVelZ(generator)), 
				sV = glm::vec3(randVelX(generator), randVelY(generator), 16.f + randVelZ(generator)), 
				expV = glm::vec3(randVelX(generator), 10.f + randVelY(generator), 10.f + randVelZ(generator));
	
	
				
	
				gCoordinator.AddComponent(drop, PickUp{
					.itemOrbitalVelocity =  itemV/ 1.f,
					.hpOrbitalVelocity =  hpV/ 1.f,
					.shieldOrbitalVelocity = sV/ 1.f,
					.expOrbitalVelocity = expV/ 1.f
				});
				gCoordinator.AddComponent(drop, Transform{parenttransform.translation + glm::vec3(0.f,-2.f,0.f)});
				gCoordinator.AddComponent(drop, RigidBody{});
				gCoordinator.AddComponent(drop, Renderable{RengbModel::getOrMakeModelFromFile(device, "models/flat_vase.obj")});
	


                gCoordinator.DestroyEntity((*it).second);
                it = deadEnemies.erase(it); 
           
            } else 
            {
                std::cerr<<"Enemy Fenced!"<<std::endl;

                it = deadEnemies.erase(it); // may make it act while dead

            }
		}
	}
	void EnemySystem::attackEnemies(FrameInfo &frameInfo, Entity player, RengbAudio &audio)
	{
		auto &playerTransform = gCoordinator.GetComponent<Transform>(player);
		auto &playerComp = gCoordinator.GetComponent<Player>(player);
		auto &playerGun = gCoordinator.GetComponent<GunObjectComponent>(playerComp.gun);

		rollingHitTime += frameInfo.frameTime;
		int numberKilled{0};
		for (auto const &entity : mEntities)
		{


			auto &transform = gCoordinator.GetComponent<Transform>(entity);
			auto &parent = gCoordinator.GetComponent<EnemyParent>(entity);
			glm::quat rotation = glm::quat(playerTransform.rotation);
			glm::vec3 forward = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
			//forward.y *= 0.5f;
			glm::vec3 toEntity = glm::normalize(playerTransform.translation - (transform.translation + glm::vec3(0.f,-1.5f,0.f)));

			int numberHit = 0;
			if(glm::dot(forward, toEntity) > 0.9f && playerGun.shooting)
			{
				++numberHit;
				parent.hitpoints-=playerGun.damage;
				gCoordinator.GetComponent<Projectile>(entity).rotations.push_back(std::make_pair(0.f, playerTransform.rotation + (glm::vec3(((float)(rand()) / (float)(RAND_MAX)) - 0.5, ((float)(rand()) / (float)(RAND_MAX)) - 0.5, 0.f))/2.f));
			}
			if(numberHit > 0 && rollingHitTime > 0.05f) 
			{
				audio.PlayHitSound(transform.translation);
				rollingHitTime = 0.f;
			}

			if(parent.hitpoints<0.f)
			{
				++numberKilled;
				explode(frameInfo, entity);			
				audio.ShootSound(100, transform.translation, false);	
			}
		}
	}

	void EnemySystem::renderEnemies(FrameInfo &frameInfo, glm::vec3 playerPosition)
	{

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&frameInfo.globalDescriptorSet,
			0,
			nullptr);

		SimplePushConstantData push{};
		push.playerPositionWeight = glm::vec4(playerPosition, 1.f);

		for (auto const &entity : mEntities)
		{

			auto &transform = gCoordinator.GetComponent<Transform>(entity);
            auto &enemyparent = gCoordinator.GetComponent<EnemyParent>(entity);

			enemyparent.runCycle += frameInfo.frameTime *(enemyparent.moveSpeed + (enemyparent.moveSpeed > 0.f ? 4.f : -4.f));
			enemyparent.timeElapsed += frameInfo.frameTime;
			enemyparent.interpolatedLegAmplitude += (((enemyparent.moveSpeed > 9.f ? 9.f : (enemyparent.moveSpeed < -6.f ? -6 : enemyparent.moveSpeed))/14.f) - enemyparent.interpolatedLegAmplitude) * frameInfo.frameTime * (enemyparent.moveSpeed > 0.f ? 3.f : 2.f);


			push.timeSpeedAmplitudeTurning.x = enemyparent.runCycle;
			push.timeSpeedAmplitudeTurning.z = enemyparent.interpolatedLegAmplitude;
			push.timeSpeedAmplitudeTurning.y = 6.f;
			push.timeSpeedAmplitudeTurning.w = enemyparent.turning;

            for(auto child : enemyparent.children)
            {
                auto &childTransform = gCoordinator.GetComponent<Transform>(child);
				auto &childRenderable = gCoordinator.GetComponent<Renderable>(child);
                push.modelMatrix = transform.mat4() * childTransform.mat4();
                
				int partID = childRenderable.partType;  // Assuming `partType` is mapped 0-5
				bool mirrorable = childRenderable.mirrorable;

				// Bind the pipeline with pre-compiled `partID`
				pipelines[partID]->bind(frameInfo.commandBuffer);
				
				vkCmdPushConstants(
                    frameInfo.commandBuffer,
                    pipelineLayout,
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    sizeof(SimplePushConstantData),
                    &push);
                    
                childRenderable.model->bind(frameInfo.commandBuffer);
                childRenderable.model->draw(frameInfo.commandBuffer);

				if(mirrorable)
				{
					pipelines[partID+nonMirroredCount]->bind(frameInfo.commandBuffer);
				
					vkCmdPushConstants(
						frameInfo.commandBuffer,
						pipelineLayout,
						VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
						0,
						sizeof(SimplePushConstantData),
						&push);
						
					childRenderable.model->bind(frameInfo.commandBuffer);
					childRenderable.model->draw(frameInfo.commandBuffer);
				}
            }

			
		}
	}

	void EnemySystem::destroySystem()
	{
		vkDestroyPipelineLayout(rengbDevice.device(), pipelineLayout, nullptr);
		for(auto& ptr : pipelines)
		{
			std::cout<<"destrying a " << std::endl;
			ptr.reset();			
		}

	}

	EnemySystem::~EnemySystem()
	{
		destroySystem();
	}

}
