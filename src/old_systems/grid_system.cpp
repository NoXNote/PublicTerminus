#include "grid_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>
#include <map>


namespace rengb
{

	struct GridPushConstants
	{
        glm::vec4 colour{};
		glm::vec4 position{};
	};

	GridSystem::GridSystem(
		RengbDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
		: rengbDevice{device}
	{
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	GridSystem::~GridSystem()
	{
		vkDestroyPipelineLayout(rengbDevice.device(), pipelineLayout, nullptr);
		rengbPipeline.reset();
	}

	void GridSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(GridPushConstants);

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

	void GridSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		RengbPipeline::defaultPipelineConfigInfo(pipelineConfig);
		RengbPipeline::enableAlphaBlending(pipelineConfig);
		RengbPipeline::disableDepthStencil(pipelineConfig);
		pipelineConfig.attributeDescriptions.clear();
		pipelineConfig.bindingDescriptions.clear();
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		rengbPipeline = std::make_unique<RengbPipeline>(
			rengbDevice,
			"shaders/infini_grid.vert.spv",
			"shaders/infini_grid.frag.spv",
			pipelineConfig);
	}

	void GridSystem::render(FrameInfo &frameInfo, glm::vec4 cumFlash, glm::vec3 playerPos)
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
        
        GridPushConstants push{};

		glm::vec3 color{.1f,.1f,.1f};
        push.colour = cumFlash;
		push.position = glm::vec4(playerPos, 1.f);

        vkCmdPushConstants(
            frameInfo.commandBuffer,
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(GridPushConstants),
            &push);
        vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
		
	}

} // namespace rengb