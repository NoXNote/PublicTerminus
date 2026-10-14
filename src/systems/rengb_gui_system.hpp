#pragma once
#include "rengb_system.hpp"
#include <glm/glm.hpp>
#include "unordered_set"
#include "rengb_device.hpp"
#include "rengb_renderer.hpp"
#include "rengb_frame_info.hpp"
#include "rengb_pipeline.hpp"

namespace rengb{

	class GameUI
	{
	public:
		GameUI(RengbDevice &device, Entity player);
		void buildWeaponBoard(RengbDevice &device);
		void buildPopUpSelector(RengbDevice &device);
		void buildStatMonitor(RengbDevice &device);

		void resetWeaponBoard(RengbDevice &device);
		void resetStatMonitor(RengbDevice &device);

		void updateWeaponBoard();
		void updateStatMonitor();
		void addEquip();

	private:
		Entity weaponBoard;
		Entity popUpSelector;
		Entity statMonitor;
		int gunBoardWidth = 16, gunBoardHeight = 10;
		int hpBoardWidth = 16, hpBoardHeight = 1;
		int shieldBoardWidth = 16, shieldBoardHeight = 1;
		Entity playerConnect;
	};


    class GuiSystem : public System
    {
    public:
		GuiSystem(
			RengbDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~GuiSystem();

		void Update(FrameInfo &frameInfo, Entity playerEnt);

		GuiSystem(const GuiSystem &) = delete;
		GuiSystem &operator=(const GuiSystem &) = delete;

		void renderUI(FrameInfo &frameInfo, float time);
		void destroySystem();

	    void Update(FrameInfo &frameInfo, RengbDevice &device);

    private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		RengbDevice &rengbDevice;
		std::unique_ptr<RengbPipeline> rengbPipeline;
		VkPipelineLayout pipelineLayout;
    };
}
