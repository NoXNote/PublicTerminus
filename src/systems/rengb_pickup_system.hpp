#pragma once
#include "rengb_system.hpp"

#include "rengb_camera.hpp"
#include "rengb_device.hpp"
#include "rengb_frame_info.hpp"
#include "rengb_renderer.hpp"
#include "rengb_pipeline.hpp"
#include "rengb_audio.hpp"


namespace rengb{
    class PickupSystem : public System
    {
	public:
		PickupSystem(
			RengbDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~PickupSystem();

		

		PickupSystem(const PickupSystem &) = delete;
		PickupSystem &operator=(const PickupSystem &) = delete;

		void renderGameObjects(FrameInfo &frameInfo, float time);
		void killPickups(RengbRenderer &renderer, RengbDevice &device);
		void destroySystem();
		void update(FrameInfo &frameInfo, Entity player, RengbAudio &audio);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		RengbDevice &rengbDevice;
		//glm::vec4 flashAmount;

		std::unique_ptr<RengbPipeline> rengbPipeline;
		VkPipelineLayout pipelineLayout;

		std::vector<std::pair<size_t, Entity>> pickupToKill; // stores last used frame index as well as entity

	};
}
