#include <iostream>
#include <cmath>

#include "rengb_render_system.hpp"
#include "components/renderable.hpp"
#include "components/transform.hpp"
#include "components/world.hpp"
#include "core/rengb_coordinator.hpp"

namespace rengb
{
	extern Coordinator gCoordinator;


	struct SimplePushConstantData
	{
		glm::mat4 modelMatrix{1.f};
		glm::vec4 darken{0.5, 1.f, 1.f, 1};
		glm::vec4 widthHeightTime{0.f};
	};

	RenderSystem::RenderSystem(RengbDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout): rengbDevice{device}
	{
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	void RenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
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

	void RenderSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		RengbPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		rengbPipeline = std::make_unique<RengbPipeline>(
			rengbDevice,
			"shaders/world.vert.spv",
			"shaders/boil.frag.spv",
			pipelineConfig);
	}

	void RenderSystem::renderGameObjects(FrameInfo &frameInfo, float time, float width, float height, glm::vec4 &cumFlash)
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
			auto &world = gCoordinator.GetComponent<World>(entity);

			push.modelMatrix = transform.mat4();
			if(glm::abs(renderable.loading) > glm::epsilon<float>())
			{
				push.modelMatrix[3][1] += renderable.loading;
				renderable.loading -= renderable.loading*8*frameInfo.frameTime;
			}
			

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

	void RenderSystem::destroySystem()
	{
		vkDestroyPipelineLayout(rengbDevice.device(), pipelineLayout, nullptr);
		rengbPipeline.reset();
	}

	RenderSystem::~RenderSystem()
	{
		destroySystem();
	}

}
