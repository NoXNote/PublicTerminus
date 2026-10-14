#include <iostream>
#include <cmath>

#include "rengb_pickup_system.hpp"
#include "components/renderable.hpp"
#include "components/transform.hpp"
#include "components/pickup.hpp"
#include "components/rigidbody.hpp"
#include "components/player.hpp"
#include "core/rengb_coordinator.hpp"

#include <glm/gtx/norm.hpp>

namespace rengb
{
	extern Coordinator gCoordinator;


	struct SimplePushConstantData
	{
		glm::mat4 modelMatrix{1.f};
		glm::vec4 colour{1.f};
	};

	PickupSystem::PickupSystem(RengbDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout): rengbDevice{device}
	{
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	void PickupSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
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

	void PickupSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		RengbPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		rengbPipeline = std::make_unique<RengbPipeline>(
			rengbDevice,
			"shaders/pickup.vert.spv",
			"shaders/pickup.frag.spv",
			pipelineConfig);
	}

	void PickupSystem::killPickups(RengbRenderer &renderer, RengbDevice &device)
    {
        for (auto it = pickupToKill.begin(); it != pickupToKill.end();) // needs to use an iterator so that it doesnt get invalidated on removal
		{
            VkFence fence = renderer.getInFlightFence((*it).first);
            VkResult result = vkGetFenceStatus(device.device(), fence);
            // auto thing = *it;
            if(result == VK_SUCCESS)
            {
                gCoordinator.DestroyEntity((*it).second);
                

                it = pickupToKill.erase(it); 
            } else 
            {
                std::cerr<<"Fenced entity!"<<std::endl;
                it = pickupToKill.erase(it); // may make it act while dead
            }
		}
    }

	void PickupSystem::update(FrameInfo &frameInfo, Entity player, RengbAudio &audio)
	{
		auto &playerTransform = gCoordinator.GetComponent<Transform>(player);
		auto &playerPlayer = gCoordinator.GetComponent<Player>(player);

		for (auto const &entity : mEntities)
		{
			auto &rigidbody = gCoordinator.GetComponent<RigidBody>(entity);
			auto &transform = gCoordinator.GetComponent<Transform>(entity);
            auto &pickup = gCoordinator.GetComponent<PickUp>(entity);

			if (glm::length2(playerTransform.translation - transform.translation) < 64.f) rigidbody.force = glm::normalize((playerTransform.translation + glm::vec3(0.f,1.f,0.f)) - transform.translation) * 16.f/glm::length2(playerTransform.translation - transform.translation);
			rigidbody.velocity -= rigidbody.velocity * frameInfo.frameTime; 

			if (glm::length2(playerTransform.translation - transform.translation) < 1.f)
			{
				playerPlayer.expHeld += pickup.exp;
				if(playerPlayer.expHeld > playerPlayer.nextLevelEXP)
				{
					playerPlayer.expHeld -= playerPlayer.nextLevelEXP;
					playerPlayer.nextLevelEXP += (playerPlayer.nextLevelEXP * 0.5)+20.f;
					audio.PlayLevelUp();
					++playerPlayer.currentLevel;
				}

				playerPlayer.hp += pickup.hp;
				if(playerPlayer.hp > playerPlayer.maxHP)
				{
					playerPlayer.hp = playerPlayer.maxHP;
				}

				playerPlayer.shield += pickup.shield;
				if(playerPlayer.shield > playerPlayer.maxShield)
				{
					playerPlayer.shield = playerPlayer.maxShield;
				}

				if(pickup.give.length() != 0)
				{
					playerPlayer.equipedArtefacts.push_back(pickup.give);
				}
				pickupToKill.push_back({frameInfo.frameIndex, entity});
				audio.PlayItemPickUp();

			}
			
		}
	}

	void PickupSystem::renderGameObjects(FrameInfo &frameInfo, float time)
	{
		rengbPipeline->bind(frameInfo.commandBuffer);

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

		for (auto const &entity : mEntities)
		{
			auto &renderable = gCoordinator.GetComponent<Renderable>(entity);
			auto &transform = gCoordinator.GetComponent<Transform>(entity);
            auto &pickup = gCoordinator.GetComponent<PickUp>(entity);

			auto bindAndDraw = [&] (glm::vec3 &position, glm::vec3 &velocity, glm::vec4 colour)
			{
				velocity -= position * frameInfo.frameTime * 1024.f;
				position += velocity * frameInfo.frameTime / 1024.f;
	
				push.modelMatrix = transform.mat4();
				push.modelMatrix[3][0] += position.x;
				push.modelMatrix[3][1] += position.y;
				push.modelMatrix[3][2] += position.z;

				push.colour = colour;
	
				vkCmdPushConstants(
					frameInfo.commandBuffer,
					pipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(SimplePushConstantData),
					&push);
				renderable.model->bind(frameInfo.commandBuffer);
				renderable.model->draw(frameInfo.commandBuffer);
			};

			if(pickup.give.length() != 0)
			{
				bindAndDraw(pickup.itemOrbitalPosition, pickup.itemOrbitalVelocity, glm::vec4(1.f, 0.f, 0.f, 1.f));

			}
			if (pickup.hp != 0)
			{
				bindAndDraw(pickup.hpOrbitalPosition, pickup.hpOrbitalVelocity, glm::vec4(0.f, 1.f, 0.5f, 1.f));
			} 
			if (pickup.shield != 0)
			{
				bindAndDraw(pickup.shieldOrbitalPosition, pickup.shieldOrbitalVelocity, glm::vec4(0.f, 0.5f, 1.f, 1.f));
			}
			if (pickup.exp != 0)
			{
				bindAndDraw(pickup.expOrbitalPosition, pickup.expOrbitalVelocity, glm::vec4(1.f));
			}

		}
	}

	void PickupSystem::destroySystem()
	{
		vkDestroyPipelineLayout(rengbDevice.device(), pipelineLayout, nullptr);
		rengbPipeline.reset();
	}

	PickupSystem::~PickupSystem()
	{
		destroySystem();
	}

}
