#include <iostream>
#include <cmath>
#include <algorithm>

#include "rengb_projectile_system.hpp"
#include "components/projectile.hpp"
#include "components/renderable.hpp"
#include "components/transform.hpp"
#include "core/rengb_coordinator.hpp"

namespace rengb
{
	extern Coordinator gCoordinator;


	struct SimplePushConstantData
	{
		glm::mat4 modelMatrix{1.f};
		glm::vec4 color{};
		float time;

	};

	ProjectileSystem::ProjectileSystem(RengbDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout): rengbDevice{device}
	{
        rengbModel = RengbModel::createModelFromFile(rengbDevice, "models/bullet.obj");
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	void ProjectileSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
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

	void ProjectileSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		RengbPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		rengbPipeline = std::make_unique<RengbPipeline>(
			rengbDevice,
			"shaders/projectile.vert.spv",
			"shaders/projectile.frag.spv",
			pipelineConfig);
	}

	void ProjectileSystem::renderGameObjects(FrameInfo &frameInfo)
	{
		time += frameInfo.frameTime;
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
			auto &projectile = gCoordinator.GetComponent<Projectile>(entity);
 

			push.color = projectile.colour;

			for(auto &instance : projectile.rotations)
			{
				instance.first += frameInfo.frameTime;
				auto instTransf = Transform{.translation = transform.translation - glm::vec3(0.f,1.5f,0.f), .rotation = instance.second + transform.rotation};
				push.modelMatrix = instTransf.mat4();
				push.time = instance.first;

				vkCmdPushConstants(
					frameInfo.commandBuffer,
					pipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(SimplePushConstantData),
					&push);
				rengbModel->bind(frameInfo.commandBuffer);
				rengbModel->draw(frameInfo.commandBuffer);
			}
		}
	}

	void ProjectileSystem::destroySystem()
	{
		vkDestroyPipelineLayout(rengbDevice.device(), pipelineLayout, nullptr);
		rengbPipeline.reset();
	}

	ProjectileSystem::~ProjectileSystem()
	{
		destroySystem();
	}

}
