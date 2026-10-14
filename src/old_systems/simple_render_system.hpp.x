#pragma once

#include "rengb_camera.hpp"
#include "rengb_device.hpp"
#include "rengb_frame_info.hpp"
//#include "rengb_game_object.hpp"
#include "rengb_pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace rengb
{
	class SimpleRenderSystem
	{
	public:
		SimpleRenderSystem(
			RengbDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem &) = delete;
		SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

		void renderGameObjects(FrameInfo &frameInfo);

		// void update(float timeSinceFire, float dt);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		RengbDevice &rengbDevice;
		//glm::vec4 flashAmount;

		std::unique_ptr<RengbPipeline> rengbPipeline;
		VkPipelineLayout pipelineLayout;
	};
} // namespace rengb