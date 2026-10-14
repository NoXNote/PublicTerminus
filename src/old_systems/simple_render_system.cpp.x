#include "simple_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>
#include <iostream>

namespace rengb
{

	struct SimplePushConstantData
	{
		glm::mat4 modelMatrix{1.f};
		glm::vec4 darken{0.5, 1.f, 1.f, 1};
	};

	SimpleRenderSystem::SimpleRenderSystem(RengbDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout): rengbDevice{device}
	{
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	SimpleRenderSystem::~SimpleRenderSystem()
	{
		vkDestroyPipelineLayout(rengbDevice.device(), pipelineLayout, nullptr);
	}

	void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
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

	void SimpleRenderSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		RengbPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		rengbPipeline = std::make_unique<RengbPipeline>(
			rengbDevice,
			//"shaders/simple_shader.vert.spv",
			"shaders/world.vert.spv",
			// "shaders/simple_shader.frag.spv",
			"shaders/world.frag.spv",
			pipelineConfig);
	}


	void SimpleRenderSystem::renderGameObjects(FrameInfo &frameInfo)
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

		// for (auto &kv : frameInfo.gameObjects)
		// {
		// 	auto &obj = kv.second;
		// 	if (obj.model == nullptr)
		// 		continue;
			
		// 	push.modelMatrix = obj.transform.mat4();
			

		// 	vkCmdPushConstants(
		// 		frameInfo.commandBuffer,
		// 		pipelineLayout,
		// 		VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		// 		0,
		// 		sizeof(SimplePushConstantData),
		// 		&push);
		// 	obj.model->bind(frameInfo.commandBuffer);
		// 	obj.model->draw(frameInfo.commandBuffer);
		// }
	}

} // namespace rengb