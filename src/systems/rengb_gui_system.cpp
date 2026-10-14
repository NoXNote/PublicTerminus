#include <iostream>
#include <iomanip>

#include "rengb_gui_system.hpp"
#include "components/renderable.hpp"
#include "components/transform.hpp"
#include "components/held_object.hpp"
#include "components/gun_object.hpp"
#include "components/player.hpp"
#include "text.hpp"

#include <sstream>


#include "core/rengb_coordinator.hpp"

namespace rengb
{
    extern Coordinator gCoordinator;

	struct SimplePushConstantData
	{
		glm::mat4 modelMatrix{1.f};
		glm::vec4 colour{1.f};
	};

	GameUI::GameUI(RengbDevice &device, Entity player) : playerConnect(player)
	{
		buildWeaponBoard(device);
		buildPopUpSelector(device);
		buildStatMonitor(device);
	}

	void GameUI::buildWeaponBoard(RengbDevice &device)
	{
		weaponBoard = gCoordinator.CreateEntity();

		auto builder = RengbModel::createBuilderFromFileUnindexed("models/text.obj");
		gCoordinator.AddComponent(weaponBoard, TextCollection{});

		auto& text = gCoordinator.GetComponent<TextCollection>(weaponBoard);
		auto& player = gCoordinator.GetComponent<Player>(playerConnect);
		auto& gun = gCoordinator.GetComponent<GunObjectComponent>(player.gun);
		int ammo = gun.ammo;

		std::ostringstream asciiForBoard;


		for(int i = 0; i < gunBoardHeight; ++i)
		{
			if(i < gun.segmentCount) asciiForBoard << "\n.";
			else asciiForBoard << "\n";
		}

		asciiForBoard << "\n>";

		text.textItems.push_back(TextItem{asciiForBoard.str(), gunBoardWidth, 6.f, 4.f, 1.f, Transform{.translation = glm::vec3(3.5f,-1.f,3.f), .scale = 0.02f, .rotation = {-glm::half_pi<float>(), glm::radians(30.f)*0, 0.f}}, player.equipedArtefacts});
		
		text.dynamicTextItems.push_back(TextItem{{}, gunBoardWidth, 6.f, 4.f, 1.f, Transform{.translation = glm::vec3(3.5f,-1.f,3.f), .scale = 0.02f, .rotation = {-glm::half_pi<float>(), glm::radians(30.f)*0, 0.f}}});

		text.fontTemplate = RengbModel::loadVertices("models/text.obj");
		text.makeTextObject();
		text.fontModel = RengbModel::createModelFromVertices(device, text.fontTemplate);
		auto model = text.buildText(device);
		gCoordinator.AddComponent(weaponBoard, Renderable{model});
		gCoordinator.AddComponent(weaponBoard, Transform{.scale=1.f});
		gCoordinator.AddComponent(weaponBoard, HeldObject{.arm = 0});
	}


	void GameUI::updateWeaponBoard()
	{
		auto& text = gCoordinator.GetComponent<TextCollection>(weaponBoard);
		auto& player = gCoordinator.GetComponent<Player>(playerConnect);
		auto& gun = gCoordinator.GetComponent<GunObjectComponent>(player.gun);

		float percentage;
		int filledSegments;


		std::ostringstream ammoBar;
		std::ostringstream colourBar;
		char colour;
		int levelSegments = static_cast<int>(gunBoardWidth * player.expHeld / player.nextLevelEXP);
		ammoBar << std::string(levelSegments, '_') << "\n\n";
		colourBar << std::string(levelSegments, 'w');	


		auto fillBar = [&] (std::string toFillBar, int segments, float colourPerc) 
		{
			if (colourPerc >= 0.7f) colour = 'g';
			else if (colourPerc >= 0.4f) colour = 'a';
			else if (colourPerc >= 0.f) colour = 'r';

			for (int i = 0; i < segments; ++i) {
				if (i < segments - filledSegments) 
				{
					ammoBar << " \n";
					// colourBar << "ww";
				} else 
				{
					ammoBar << " " << toFillBar << "\n";//" -\n";
					colourBar << std::string(toFillBar.length(), colour);
				}
			}

			ammoBar << std::string(9-segments, '\n');
		};



		if(gun.reloadTimer <= 0.f)
		{
			percentage = static_cast<float>(gun.ammo) / static_cast<float>(gun.maxAmmo);
			filledSegments = static_cast<int>(percentage * gun.segmentCount + 0.5f); 
			// toFillBar = " -\n";

			fillBar(gun.bulletLook, gun.segmentCount, percentage);
			ammoBar << " " << gun.ammo;
			
			if(gun.timeSinceFire > gun.passiveReloadDelay && gun.ammo < gun.maxAmmo)
			{
				colourBar << std::string(std::to_string(gun.ammo).length(), 'b');				
			} else
			{
				colourBar << std::string(std::to_string(gun.ammo).length(), 'c');						
			}

				
		} else
		{
			percentage = static_cast<float>(gun.reloadDuration - gun.reloadTimer) / static_cast<float>(gun.reloadDuration);
			filledSegments = static_cast<int>(percentage * 8 + 0.5f); 
			
			fillBar("|", 8, percentage);
			switch(static_cast<int>(percentage * 8 + 0.5f) % 4)
			{
				case 0:
					ammoBar << " |";	
					break;	
				case 1:
					ammoBar << " /";	
					break;	
				case 2:
					ammoBar << " -";	
					break;	
				case 3:
					ammoBar << " \\";
					break;
			}
			colourBar << "w";
		}
		text.dynamicTextItems[0].text = ammoBar.str();
		text.dynamicTextItems[0].colours = colourBar.str();
		
	}

	void GameUI::buildStatMonitor(RengbDevice &device)
	{
		statMonitor = gCoordinator.CreateEntity();
		auto builder = RengbModel::createBuilderFromFileUnindexed("models/text.obj");
		gCoordinator.AddComponent(statMonitor, TextCollection{});

		auto& text = gCoordinator.GetComponent<TextCollection>(statMonitor);
		auto& player = gCoordinator.GetComponent<Player>(playerConnect);


		hpBoardWidth = static_cast<int>((player.maxHP / 100.f) + 16.5f);
		shieldBoardWidth = static_cast<int>((player.maxShield / 20.f) + 10.5f);

		text.textItems.push_back(TextItem{"", hpBoardWidth, 6.f, 10.f, 0.f, Transform{.translation = glm::vec3(-2.5f,0.5f,2.5f), .scale = 0.02f, .rotation = {-glm::half_pi<float>(), glm::radians(-30.f)*0, 0.f}}});
		text.textItems.push_back(TextItem{"", shieldBoardWidth, 6.f, 10.f, 0.f, Transform{.translation = glm::vec3(-2.5f,0.65f,2.2f), .scale = 0.02f, .rotation = {-glm::half_pi<float>(), glm::radians(-30.f)*0, 0.f}}});


		text.dynamicTextItems.push_back(TextItem{"", hpBoardWidth, 6.f, 10.f, 0.f, Transform{.translation = glm::vec3(-2.5f,0.5f,2.5f), .scale = 0.02f, .rotation = {-glm::half_pi<float>(), glm::radians(-30.f)*0, 0.f}}});
		text.dynamicTextItems.push_back(TextItem{"", shieldBoardWidth, 6.f, 10.f, 0.f, Transform{.translation = glm::vec3(-2.5f,0.65f,2.2f), .scale = 0.02f, .rotation = {-glm::half_pi<float>(), glm::radians(-30.f)*0, 0.f}}});

		text.fontTemplate = RengbModel::loadVertices("models/text.obj");
		text.makeTextObject();
		text.fontModel = RengbModel::createModelFromVertices(device, text.fontTemplate);
		auto model = text.buildText(device);
		gCoordinator.AddComponent(statMonitor, Renderable{model});
		gCoordinator.AddComponent(statMonitor, Transform{.scale=1.f});
		gCoordinator.AddComponent(statMonitor, HeldObject{.arm = 0});
	}

	void GameUI::resetStatMonitor(RengbDevice &device)
	{

		auto& text = gCoordinator.GetComponent<TextCollection>(statMonitor);
		auto& renderable = gCoordinator.GetComponent<Renderable>(statMonitor);
		auto& player = gCoordinator.GetComponent<Player>(playerConnect);

		hpBoardWidth = static_cast<int>((player.maxHP / 100.f) + 16.5f);
		shieldBoardWidth = static_cast<int>((player.maxShield / 20.f) + 10.5f);

		text.textItems.clear();
		text.dynamicTextItems.clear();

		text.textItems.push_back(TextItem{"", hpBoardWidth, 6.f, 10.f, 0.f, Transform{.translation = glm::vec3(-2.5f,0.5f,2.5f), .scale = 0.02f, .rotation = {-glm::half_pi<float>(), glm::radians(-30.f)*0, 0.f}}});
		text.textItems.push_back(TextItem{"", shieldBoardWidth, 6.f, 10.f, 0.f, Transform{.translation = glm::vec3(-2.5f,0.65f,2.2f), .scale = 0.02f, .rotation = {-glm::half_pi<float>(), glm::radians(-30.f)*0, 0.f}}});


		text.dynamicTextItems.push_back(TextItem{"", hpBoardWidth, 6.f, 10.f, 0.f, Transform{.translation = glm::vec3(-2.5f,0.5f,2.5f), .scale = 0.02f, .rotation = {-glm::half_pi<float>(), glm::radians(-30.f)*0, 0.f}}});
		text.dynamicTextItems.push_back(TextItem{"", shieldBoardWidth, 6.f, 10.f, 0.f, Transform{.translation = glm::vec3(-2.5f,0.65f,2.2f), .scale = 0.02f, .rotation = {-glm::half_pi<float>(), glm::radians(-30.f)*0, 0.f}}});

		auto model = text.buildText(device);
		renderable.model = model; 

	}

	void GameUI::updateStatMonitor()
	{
		auto& text = gCoordinator.GetComponent<TextCollection>(statMonitor);
		auto& player = gCoordinator.GetComponent<Player>(playerConnect);

		int floorHP = static_cast<int>(player.rollingHP + 0.5f), floorShield = static_cast<int>(player.rollingShield + 0.5f);
		int healthLength = std::to_string(floorHP).length(), shieldLength = std::to_string(floorShield).length();

		int hpFilledSegments  = static_cast<int>(0.5f + (((hpBoardWidth - 6) * player.rollingHP)/ player.maxHP)), 
			shieldFilledSegments = static_cast<int>(0.5f + (((shieldBoardWidth - 6) * player.rollingShield)/ player.maxShield));

		int gap = (hpBoardWidth - 6) - hpFilledSegments;
		
		std::ostringstream bar;
		std::ostringstream colourBar;

		bar << std::string(hpFilledSegments, '=') << std::string(gap, '.') << '|' << std::string(6 - healthLength, ' ') << std::to_string(floorHP);
		colourBar << std::string(hpFilledSegments, 'g') << std::string(gap + healthLength, 'w'); 

		text.dynamicTextItems[0].text = bar.str();
		text.dynamicTextItems[0].colours = colourBar.str();

		bar.str("");
		bar.clear();

		colourBar.str("");
		colourBar.clear();

		gap = shieldBoardWidth - shieldFilledSegments - 6;

		bar << std::string(shieldFilledSegments, '=') << std::string(gap, '.') << '|' << std::string(6 - shieldLength, ' ') << std::to_string(floorShield);
		colourBar << std::string(shieldFilledSegments, 'b') << std::string(gap + shieldLength, 'w'); 


		text.dynamicTextItems[1].text = bar.str();
		text.dynamicTextItems[1].colours = colourBar.str();
	}

	void GameUI::buildPopUpSelector(RengbDevice &device)
	{
		popUpSelector = gCoordinator.CreateEntity();

		gCoordinator.AddComponent(popUpSelector, TextCollection{});
		auto& text = gCoordinator.GetComponent<TextCollection>(popUpSelector);

		text.textItems.push_back(TextItem{"[]    []    []", -1, 6.f, 10.f, 0.5f, Transform{.translation = glm::vec3(0,2.f, 2.f), .scale = 0.02f, .rotation = {-glm::half_pi<float>(), 0.f, 0.f}}});
		text.fontTemplate = RengbModel::loadVertices("models/text.obj");
		text.makeTextObject();
		auto model = text.buildText(device);

		gCoordinator.AddComponent(popUpSelector, Renderable{model});
		gCoordinator.AddComponent(popUpSelector, Transform{.scale=1.f});
		gCoordinator.AddComponent(popUpSelector, HeldObject{.arm = 0});
	}

	GuiSystem::GuiSystem(RengbDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout): rengbDevice{device}
	{
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	void GuiSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
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

	void GuiSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		RengbPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		rengbPipeline = std::make_unique<RengbPipeline>(
			rengbDevice,
			"shaders/dynamic_text.vert.spv",
			"shaders/dynamic_text.frag.spv",
			pipelineConfig);
	}



	void GuiSystem::renderUI(FrameInfo &frameInfo, float time)
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

		for (auto const &entity : mEntities)
		{
			auto &transform = gCoordinator.GetComponent<Transform>(entity);
			auto &textCollection = gCoordinator.GetComponent<TextCollection>(entity);

            for(TextItem item : textCollection.dynamicTextItems)
            {
				// item
                int wHeadx = 0, wHeady = 0;
				size_t colourIterator = 0;

                if (item.boxWidth <= -1) item.boxWidth = item.text.length();
                if (item.centering != 0.f) item.centreText();
				glm::mat4 modelSpecific = transform.mat4() * item.specificTransform.mat4();


                for(char c : item.text)
                {
                    if (c == '\n')
                    {
                        wHeadx = -1;
                        ++wHeady;
                    }
                    else if(c != ' ')
                    {


						push.modelMatrix = modelSpecific * glm::mat4{
							{1, 0.0f, 0.0f, 0.0f},
							{0.0f, 1.f, 0.0f, 0.0f},
							{0.0f, 0.0f, 1.f, 0.0f},
							{wHeadx * item.kerning, 0.f, wHeady * (item.lineSpace+1), 1.0f}}; 
							// get it so the matrix represents a translation via headx * kerning on x and heady * linespace+1, followed by specific transform, follwed by model matrix.

						switch(item.colours[colourIterator])
						{
							case 'w': // white
								push.colour = glm::vec4(1.f,1.f,1.f,1.f);
								break;
							case 'r': // red
								push.colour = glm::vec4(1.f,0.f,0.f,1.f);
								break;
							case 'g': // green
								push.colour = glm::vec4(0.f,1.f,0.f,1.f);
								break;
							case 'a': // amber
								push.colour = glm::vec4(1.f,1.f,0.f,1.f);
								break;
							case 'c': // cyan
								push.colour = glm::vec4(0.f,1.f,1.f,1.f);
								break;
							case 'b': // blue
								push.colour = glm::vec4(0.f,0.5f,1.f,1.f);
								break;				
							case 'd': // dark blue
								push.colour = glm::vec4(0.f,0.f,1.f,1.f);
								break;						
							
						}
						++colourIterator;

						vkCmdPushConstants(
							frameInfo.commandBuffer,
							pipelineLayout,
							VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
							0,
							sizeof(SimplePushConstantData),
							&push);

						textCollection.fontModel->bind(frameInfo.commandBuffer);
						size_t start = textCollection.vertexOffsets[c - 33 + 1], end = textCollection.vertexOffsets[c - 33]; 

						textCollection.fontModel->drawRange(frameInfo.commandBuffer, start,   end);
					}
                    if(item.boxWidth != 0 && wHeadx >= item.boxWidth)
                    {
                        wHeadx = 0;
                        ++wHeady;
                    } else ++wHeadx;
                }
			}
		}
	}

	void GuiSystem::destroySystem()
	{
		vkDestroyPipelineLayout(rengbDevice.device(), pipelineLayout, nullptr);
		rengbPipeline.reset();
	}

	GuiSystem::~GuiSystem()
	{
		destroySystem();
	}


}

