#pragma once
#include "rengb_system.hpp"

#include "rengb_camera.hpp"
#include "rengb_device.hpp"
#include "rengb_frame_info.hpp"
//#include "rengb_game_object.hpp"
#include "rengb_pipeline.hpp"


namespace rengb{
    class RenderSystem : public System
    {
	public:
		RenderSystem(
			RengbDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~RenderSystem();

		

		RenderSystem(const RenderSystem &) = delete;
		RenderSystem &operator=(const RenderSystem &) = delete;

		void renderGameObjects(FrameInfo &frameInfo, float time, float width, float height, glm::vec4 &cumFlash);
		void destroySystem();
		// void update(float timeSinceFire, float dt);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		RengbDevice &rengbDevice;
		//glm::vec4 flashAmount;

		std::unique_ptr<RengbPipeline> rengbPipeline;
		VkPipelineLayout pipelineLayout;

	};
}
