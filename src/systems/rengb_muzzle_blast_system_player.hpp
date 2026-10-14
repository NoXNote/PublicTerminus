#pragma once
#include "rengb_system.hpp"

#include "rengb_camera.hpp"
#include "rengb_device.hpp"
#include "rengb_frame_info.hpp"
//#include "rengb_game_object.hpp"
#include "rengb_pipeline.hpp"


namespace rengb{
    class BlastSystem : public System
    {
	public:
		BlastSystem(
			RengbDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~BlastSystem();

		BlastSystem(const BlastSystem &) = delete;
		BlastSystem &operator=(const BlastSystem &) = delete;

		void renderBlast(FrameInfo &frameInfo, glm::vec4 &cumFlash);
		void destroySystem();


	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		RengbDevice &rengbDevice;


		std::unique_ptr<RengbPipeline> rengbPipeline;
		VkPipelineLayout pipelineLayout;
		float time{0.f};
	};
}
