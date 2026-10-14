#pragma once
#include "rengb_system.hpp"

#include "rengb_camera.hpp"
#include "rengb_device.hpp"
#include "rengb_frame_info.hpp"
// #include "rengb_game_object.hpp"
#include "rengb_pipeline.hpp"
#include <unordered_set>

namespace rengb
{
	class ProjectileSystem : public System
	{
	public:
		ProjectileSystem(
			RengbDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~ProjectileSystem();

		ProjectileSystem(const ProjectileSystem &) = delete;
		ProjectileSystem &operator=(const ProjectileSystem &) = delete;

		void renderGameObjects(FrameInfo &frameInfo);
		void destroySystem();
		//void killProjectiles();
		// void update(float timeSinceFire, float dt);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		RengbDevice &rengbDevice;
		// glm::vec4 flashAmount;
		std::shared_ptr<RengbModel> rengbModel;

		std::unique_ptr<RengbPipeline> rengbPipeline;
		VkPipelineLayout pipelineLayout;
		float time{0.f};
		std::unordered_set<Entity> deadProjectiles;
	};
}
