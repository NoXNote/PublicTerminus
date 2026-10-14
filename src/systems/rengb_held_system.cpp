#include <iostream>
#include <cmath>

#include "rengb_held_system.hpp"
#include "components/renderable.hpp"
#include "components/transform.hpp"
#include "components/held_object.hpp"
#include "components/player.hpp"
#include "components/gun_object.hpp"
#include "components/world.hpp"

#include "core/rengb_coordinator.hpp"

#include <glm/gtc/epsilon.hpp>

namespace rengb
{
	extern Coordinator gCoordinator;


	struct SimplePushConstantData
	{
		glm::mat4 modelMatrix{1.f};
		glm::vec4 darken{0.5, 1.f, 1.f, 1};
		glm::vec4 widthHeightTime{0.f};
	};

	HeldSystem::HeldSystem(RengbDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout): rengbDevice{device}
	{
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	void HeldSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
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

	void HeldSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		RengbPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		rengbPipeline = std::make_unique<RengbPipeline>(
			rengbDevice,
			"shaders/holding_with_flash.vert.spv",
			"shaders/holdingboil.frag.spv",
			pipelineConfig);
	}
	void HeldSystem::Update(FrameInfo &frameInfo, Entity playerEnt)
	{
		auto &player = gCoordinator.GetComponent<Player>(playerEnt); // do once ideally
		for (auto const &entity : mEntities)
		{
			// auto &renderable = gCoordinator.GetComponent<Renderable>(entity);
			auto &transform = gCoordinator.GetComponent<Transform>(entity);
			auto &held = gCoordinator.GetComponent<HeldObject>(entity);

			held.smoothedMovement(frameInfo.frameTime, player.pitch, 
            player.yaw, glm::vec3(player.absoluteMovement.x,player.velocity.y,player.absoluteMovement.y), 
            player.crouching, player.onGround, player.crouchDistance);

	
			transform.translation = held.restPosition + (player.headBob * 0.02f * held.weight * player.targetSpeed) + held.momentumMovement;
			transform.rotation = glm::vec3{held.momentumMovement.y, held.arm * held.momentumMovement.z, held.momentumMovement.x};

		}
	}

	void HeldSystem::renderHeldObjects(FrameInfo &frameInfo, float time, float width, float height, glm::vec4 &cumFlash)
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
		push.widthHeightTime.x = width;
		push.widthHeightTime.y = height;
		push.widthHeightTime.z = time;
		push.darken = cumFlash;

		for (auto const &entity : mEntities)
		{
			auto &renderable = gCoordinator.GetComponent<Renderable>(entity);
			auto &transform = gCoordinator.GetComponent<Transform>(entity);

			push.modelMatrix = transform.mat4();

			if(!glm::epsilonEqual(renderable.loading, 0.f, glm::epsilon<float>()))
			{
				push.modelMatrix[3][renderable.loadDirection] += renderable.loading;
				renderable.loading -= renderable.loading*8*frameInfo.frameTime;
			}

            push.darken = glm::vec4(0,1.f,1.f,1.f);


			vkCmdPushConstants(
				frameInfo.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);
			renderable.model->bind(frameInfo.commandBuffer);
			renderable.model->draw(frameInfo.commandBuffer);

		}
	}

	void HeldSystem::destroySystem()
	{
		vkDestroyPipelineLayout(rengbDevice.device(), pipelineLayout, nullptr);
		rengbPipeline.reset();
	}

	HeldSystem::~HeldSystem()
	{
		destroySystem();
	}

}
