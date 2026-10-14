#pragma once
#include "rengb_system.hpp"

#include "rengb_camera.hpp"

#include "rengb_frame_info.hpp"

#include "rengb_audio.hpp"


#include<unistd.h>
#include <unordered_set>

#include "rengb_renderer.hpp"
#include "rengb_pipeline.hpp"
#include "rengb_device.hpp"
#include <random>




namespace rengb{
    class EnemySystem : public System
    {
	public:
		EnemySystem(
			RengbDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~EnemySystem();

		

		EnemySystem(const EnemySystem &) = delete;
		EnemySystem &operator=(const EnemySystem &) = delete;
		
		void followPath(FrameInfo &frameInfo, glm::vec3 playerPosition);

		void attackEnemies(FrameInfo &frameInfo, Entity player, RengbAudio &audio);

		void killEnemies(RengbRenderer &renderer, RengbDevice &device);

		void pathEnemies(std::shared_ptr<RengbModel> vase);

		void pathEnemies();

		void renderEnemies(FrameInfo &frameInfo, glm::vec3 playerPosition);

		void explodeAll();

		void explode(FrameInfo &frameInfo, Entity entity);

		void destroySystem();

		std::vector<std::pair<size_t, Entity>> deadEnemies; // stores last used frame index as well as entity
	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipelines(VkRenderPass renderPass);

		RengbDevice &rengbDevice;

		std::vector<std::unique_ptr<RengbPipeline>> pipelines;
		VkPipelineLayout pipelineLayout;

		VkSpecializationMapEntry specializationEntry{};
		static constexpr int partIDs[]{0, 1, 2, 3, 4, 5, 6, 7, -1, -2, -3, -4, -5, -6, -7};
		static constexpr int nonMirroredCount = 7;

		float rollingHitTime{0.f};
	};
}
