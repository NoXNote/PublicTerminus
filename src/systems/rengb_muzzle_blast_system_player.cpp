#include <iostream>
#include <cmath>

#include "rengb_muzzle_blast_system_player.hpp"
#include "components/transform.hpp"
#include "components/gun_object.hpp"
#include "core/rengb_coordinator.hpp"

namespace rengb
{
	extern Coordinator gCoordinator;


	struct SimplePushConstantData
	{
		glm::mat4 modelMatrix{1.f};
		glm::vec4 positionOnBarrel{1.f};
		float time;
	};

	BlastSystem::BlastSystem(RengbDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout): rengbDevice{device}
	{
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	void BlastSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
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

	void BlastSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		RengbPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		rengbPipeline = std::make_unique<RengbPipeline>(
			rengbDevice,
			"shaders/pmflash.vert.spv",
			"shaders/pmflash.frag.spv",
			pipelineConfig);
	}


	void BlastSystem::renderBlast(FrameInfo &frameInfo, glm::vec4 &cumFlash)
	{
		
		time = time > glm::two_pi<float>() ? 0.f : time + (frameInfo.frameTime); 

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
			auto &transform = gCoordinator.GetComponent<Transform>(entity);
			auto &gun = gCoordinator.GetComponent<GunObjectComponent>(entity);


			for (auto &position : gun.muzzlePositions)
			{
				if ((std::get<1>(position) < 0.f)) continue;
				cumFlash.r += std::get<1>(position)/2.f;
				cumFlash.g += std::get<1>(position);
				cumFlash.b += std::get<1>(position);
				cumFlash.w += std::get<1>(position)/1.f;
				std::get<1>(position) -= frameInfo.frameTime*4.f;

				push.modelMatrix = transform.mat4();
				push.time = std::get<1>(position);
				push.positionOnBarrel = glm::vec4(std::get<0>(position),1.f);

				for (auto &part : gun.flashParts)
				{
					vkCmdPushConstants(
						frameInfo.commandBuffer,
						pipelineLayout,
						VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
						0,
						sizeof(SimplePushConstantData),
						&push);
					part->bind(frameInfo.commandBuffer);
					part->draw(frameInfo.commandBuffer);					
				}
			}				
		}
	}

	void BlastSystem::destroySystem()
	{
		vkDestroyPipelineLayout(rengbDevice.device(), pipelineLayout, nullptr);
		rengbPipeline.reset();
	}

	BlastSystem::~BlastSystem()
	{
		destroySystem();

	}

}
