#pragma once
#include "rengb_system.hpp"

#include "rengb_camera.hpp"
#include "rengb_device.hpp"
#include "rengb_frame_info.hpp"

#include "rengb_pipeline.hpp"


namespace rengb{
    class HeldSystem : public System
    {
	public:
		HeldSystem(
			RengbDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~HeldSystem();

		void Update(FrameInfo &frameInfo, Entity playerEnt);

		HeldSystem(const HeldSystem &) = delete;
		HeldSystem &operator=(const HeldSystem &) = delete;

		void renderHeldObjects(FrameInfo &frameInfo, float time, float width, float height, glm::vec4 &cumFlash);
		void destroySystem();

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		RengbDevice &rengbDevice;


		std::unique_ptr<RengbPipeline> rengbPipeline;
		VkPipelineLayout pipelineLayout;

	};
}
