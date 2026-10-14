#include "first_app.hpp"

#include "rengb_buffer.hpp"
#include "rengb_camera.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_SWIZZLE 
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <json.hpp>
using json = nlohmann::json;

// std
#include <array>
#include <cassert>
#include <chrono>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <random>

// grid system
#include "old_systems/grid_system.hpp"

//ecs
#include "core/rengb_coordinator.hpp"

#include "components/transform.hpp"
#include "components/rigidbody.hpp"
#include "components/renderable.hpp"
#include "components/collisionshape.hpp"
#include "components/enemypart.hpp"
#include "components/enemyparent.hpp"
#include "components/world.hpp"
#include "components/player.hpp"
#include "components/terrain.hpp"
#include "components/door.hpp"
#include "components/room.hpp"
#include "components/held_object.hpp"
#include "components/gun_object.hpp"
#include "components/sleep.hpp"
#include "components/projectile.hpp"
#include "components/detritus.hpp"
#include "components/text.hpp"
#include "components/pickup.hpp"

#include "systems/rengb_physics_system.hpp"
#include "systems/rengb_render_system.hpp"
#include "systems/rengb_collision_system.hpp"
#include "systems/rengb_enemy_system.hpp"
#include "systems/rengb_player_system.hpp"
#include "systems/rengb_traversal_system.hpp"
#include "systems/rengb_pickup_system.hpp"
#include "systems/rengb_held_system.hpp"
#include "systems/rengb_door_system.hpp"
#include "systems/rengb_projectile_system.hpp"
#include "systems/rengb_detritus_system.hpp"
#include "systems/rengb_muzzle_blast_system_player.hpp"
#include "systems/rengb_gui_system.hpp"


namespace rengb
{
	Coordinator gCoordinator;
	FirstApp::FirstApp()
	{
		globalPool =
			RengbDescriptorPool::Builder(rengbDevice)
				.setMaxSets(RengbSwapChain::MAX_FRAMES_IN_FLIGHT)
				.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, RengbSwapChain::MAX_FRAMES_IN_FLIGHT)
				.build();
	}

	FirstApp::~FirstApp() {}

	void FirstApp::run()
	{
		
		std::vector<std::unique_ptr<RengbBuffer>> uboBuffers(RengbSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++)
		{
			uboBuffers[i] = std::make_unique<RengbBuffer>(
				rengbDevice,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			uboBuffers[i]->map();
		}

		auto globalSetLayout =
			RengbDescriptorSetLayout::Builder(rengbDevice)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
				.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(RengbSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++)
		{
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			RengbDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		//===========SET UP AUDIO================
		rengbAudio.LoadAudio("default_tracks");

		//===========REGISTER COMPONENTS=============
		gCoordinator.Init();
		gCoordinator.RegisterComponent<RigidBody>();
		gCoordinator.RegisterComponent<Transform>();
		gCoordinator.RegisterComponent<Renderable>();
		gCoordinator.RegisterComponent<CollisionShape>();
		gCoordinator.RegisterComponent<EnemyParent>();
		gCoordinator.RegisterComponent<EnemyPart>();
		gCoordinator.RegisterComponent<World>();
		gCoordinator.RegisterComponent<Player>();
		gCoordinator.RegisterComponent<Terrain>();
		gCoordinator.RegisterComponent<Door>();
		gCoordinator.RegisterComponent<Room>();
		gCoordinator.RegisterComponent<GunObjectComponent>();
		gCoordinator.RegisterComponent<HeldObject>();
		gCoordinator.RegisterComponent<Sleep>();
		gCoordinator.RegisterComponent<Projectile>();
		gCoordinator.RegisterComponent<Detritus>();
		gCoordinator.RegisterComponent<TextCollection>();
		gCoordinator.RegisterComponent<PickUp>();
		//===========REGISTER SYSTEMS=============
		auto physicsSystem = gCoordinator.RegisterSystem<PhysicsSystem>();
		auto collisionSystem = gCoordinator.RegisterSystem<CollisionSystem>();
		auto playerSystem = gCoordinator.RegisterSystem<PlayerSystem>();
		auto doorSystem = gCoordinator.RegisterSystem<DoorSystem>();
		auto detritusSystem = gCoordinator.RegisterSystem<DetritusSystem>();

		auto traversalSystem = gCoordinator.RegisterRenderSystem<TraversalSystem>(rengbDevice, rengbRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
		auto renderSystem = gCoordinator.RegisterRenderSystem<RenderSystem>(rengbDevice, rengbRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
		auto enemySystem = gCoordinator.RegisterRenderSystem<EnemySystem>(rengbDevice, rengbRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
		auto heldSystem = gCoordinator.RegisterRenderSystem<HeldSystem>(rengbDevice, rengbRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
		auto projectileSystem = gCoordinator.RegisterRenderSystem<ProjectileSystem>(rengbDevice, rengbRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
		auto blastSystem = gCoordinator.RegisterRenderSystem<BlastSystem>(rengbDevice, rengbRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
		auto guiSystem = gCoordinator.RegisterRenderSystem<GuiSystem>(rengbDevice, rengbRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
		auto pickupSystem = gCoordinator.RegisterRenderSystem<PickupSystem>(rengbDevice, rengbRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout());
		//===========SET SIGNATURES FOR SYSTEMS=============
		Signature signature;

		signature.set(gCoordinator.GetComponentType<Transform>());
		signature.set(gCoordinator.GetComponentType<Renderable>());
		signature.set(gCoordinator.GetComponentType<World>());
		gCoordinator.SetSystemSignature<RenderSystem>(signature);

		signature.reset();
		signature.set(gCoordinator.GetComponentType<Transform>());
		signature.set(gCoordinator.GetComponentType<Renderable>());
		signature.set(gCoordinator.GetComponentType<HeldObject>());
		gCoordinator.SetSystemSignature<HeldSystem>(signature);

		signature.reset();
		signature.set(gCoordinator.GetComponentType<Transform>());
		signature.set(gCoordinator.GetComponentType<RigidBody>());
		gCoordinator.SetSystemSignature<PhysicsSystem>(signature);
		
		signature.reset();
		signature.set(gCoordinator.GetComponentType<Transform>());
		signature.set(gCoordinator.GetComponentType<CollisionShape>());
		signature.set(gCoordinator.GetComponentType<World>());
		gCoordinator.SetSystemSignature<CollisionSystem>(signature);

		signature.reset();
		signature.set(gCoordinator.GetComponentType<Transform>());
		signature.set(gCoordinator.GetComponentType<EnemyParent>());
		signature.set(gCoordinator.GetComponentType<World>());
		gCoordinator.SetSystemSignature<EnemySystem>(signature); 

		signature.reset();
		signature.set(gCoordinator.GetComponentType<Transform>());
		signature.set(gCoordinator.GetComponentType<RigidBody>());
		signature.set(gCoordinator.GetComponentType<Player>());
		signature.set(gCoordinator.GetComponentType<CollisionShape>());
		gCoordinator.SetSystemSignature<PlayerSystem>(signature); 

		signature.reset();
		signature.set(gCoordinator.GetComponentType<Door>());
		signature.set(gCoordinator.GetComponentType<Transform>());
		signature.set(gCoordinator.GetComponentType<World>());
		gCoordinator.SetSystemSignature<DoorSystem>(signature);

		signature.reset();
		signature.set(gCoordinator.GetComponentType<Terrain>());
		signature.set(gCoordinator.GetComponentType<World>());
		gCoordinator.SetSystemSignature<TraversalSystem>(signature);

		signature.reset();
		signature.set(gCoordinator.GetComponentType<Transform>());
		signature.set(gCoordinator.GetComponentType<Projectile>());
		gCoordinator.SetSystemSignature<ProjectileSystem>(signature);

		signature.reset();
		signature.set(gCoordinator.GetComponentType<Transform>());
		signature.set(gCoordinator.GetComponentType<GunObjectComponent>());
		gCoordinator.SetSystemSignature<BlastSystem>(signature);


		signature.reset();
		signature.set(gCoordinator.GetComponentType<Transform>());
		signature.set(gCoordinator.GetComponentType<Detritus>());
		signature.set(gCoordinator.GetComponentType<RigidBody>());
		gCoordinator.SetSystemSignature<DetritusSystem>(signature);

		signature.reset();
		signature.set(gCoordinator.GetComponentType<Transform>());
		signature.set(gCoordinator.GetComponentType<HeldObject>());
		signature.set(gCoordinator.GetComponentType<Renderable>());
		signature.set(gCoordinator.GetComponentType<TextCollection>());
		gCoordinator.SetSystemSignature<GuiSystem>(signature);		

		signature.reset();
		signature.set(gCoordinator.GetComponentType<Transform>());
		signature.set(gCoordinator.GetComponentType<Renderable>());
		signature.set(gCoordinator.GetComponentType<RigidBody>());
		signature.set(gCoordinator.GetComponentType<PickUp>());
		gCoordinator.SetSystemSignature<PickupSystem>(signature);		



		//===========CREATE ENTITIES=============
		std::vector<Entity> entities(MAX_ENTITIES);

		RengbJson::generateLevel("start_stage", rengbDevice, 12, 0, 0);
		Entity pshove = RengbJson::generatePlayer("default_player", rengbDevice);

		GameUI gameUI{rengbDevice, pshove};
		
		

		GridSystem gridSystem{
			rengbDevice,
			rengbRenderer.getSwapChainRenderPass(),
			globalSetLayout->getDescriptorSetLayout()};

		RengbCamera camera{};

		auto currentTime = std::chrono::high_resolution_clock::now();

		glfwSetInputMode(rengbWindow.getGLFWwindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		float fov = 90.f; // l:75 m:90 h:120
		float simSpeed= 1.f;
		glm::vec4 cumFlash{0.f};

		glm::vec3 lastDistance{-999.f};
		float time{0.f};
		bool swapped{true};
		while (!rengbWindow.shouldClose())
		//while (false)
		{
			auto& playerTransform = gCoordinator.GetComponent<Transform>(pshove);
			auto& playerRigid = gCoordinator.GetComponent<RigidBody>(pshove);
			auto& playerPlayer =gCoordinator.GetComponent<Player>(pshove);

			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime =
				std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;
			time += (frameTime); 
			float aspect = rengbRenderer.getAspectRatio();

			if(playerPlayer.sprinting && !playerPlayer.downSights) fov += (100.f-fov)*frameTime*2.f; // in charge of sprint effect
			else if (playerPlayer.downSights) fov += (60.f-fov)*frameTime*8.f;
			else fov += (90.f-fov)*frameTime*4.f;

			camera.setPerspectiveProjection(glm::radians(fov), aspect, 0.001f, 20.f);

			camera.setViewYXZ(
				+ playerTransform.translation 
				+ playerPlayer.headBob 
				+ glm::vec3(0.f, playerPlayer.crouchDistance, 0.f),
				playerTransform.rotation);


			if (auto commandBuffer = rengbRenderer.beginFrame())
			{
				
				rengbRenderer.setFSR({4,4});
				enemySystem->killEnemies(rengbRenderer, rengbDevice);

				detritusSystem->killDet(rengbRenderer, rengbDevice);	
				pickupSystem->killPickups(rengbRenderer, rengbDevice);	
				
				int frameIndex = rengbRenderer.getFrameIndex();
				FrameInfo frameInfo{
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex]
					};

				// --update--

				if(glfwGetKey(rengbWindow.getGLFWwindow(), GLFW_KEY_1) == GLFW_PRESS) // press 2 to disable mouse capture
					glfwSetInputMode(rengbWindow.getGLFWwindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				else if(glfwGetKey(rengbWindow.getGLFWwindow(), GLFW_KEY_2) == GLFW_PRESS)
					glfwSetInputMode(rengbWindow.getGLFWwindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				else if(glfwGetKey(rengbWindow.getGLFWwindow(), GLFW_KEY_3) == GLFW_PRESS && swapped)
				{
					playerSystem->SwapWeapon(rengbDevice, pshove, "minigun2", rengbAudio);		
					swapped = false;			
				}
				else if(glfwGetKey(rengbWindow.getGLFWwindow(), GLFW_KEY_4) == GLFW_PRESS && !swapped)
				{
					playerSystem->SwapWeapon(rengbDevice, pshove, "maximumCannon2", rengbAudio);	
					swapped = true;				
				}
				else if(glfwGetKey(rengbWindow.getGLFWwindow(), GLFW_KEY_5) == GLFW_PRESS && !swapped)
				{
					std::cout<< glm::degrees(gCoordinator.GetComponent<Transform>(playerPlayer.inRoom).rotation.y) << std::endl;
				}

				rengbAudio.Update(playerTransform.translation, playerRigid.velocity, playerTransform.rotation, playerTransform.mat4());
				rengbAudio.CheckAllSounds();

				GlobalUbo ubo{};
				ubo.projection = camera.getProjection();
				ubo.view = camera.getView();

				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				
				rengbRenderer.beginSwapChainRenderPass(commandBuffer);
				heldSystem->Update(frameInfo, pshove);
				pickupSystem->update(frameInfo, pshove, rengbAudio);
				playerSystem->Update(rengbWindow.getGLFWwindow(), frameTime, rengbAudio);
				gridSystem.render(frameInfo, glm::vec4{0,1,1,1}, gCoordinator.GetComponent<Transform>(pshove).translation);

				if(glm::dot((lastDistance - playerTransform.translation), lastDistance - playerTransform.translation) > 64.f)
				{
					lastDistance = playerTransform.translation;
					traversalSystem->Update(playerTransform.translation);
					enemySystem->pathEnemies();
				}

				enemySystem->attackEnemies(frameInfo, pshove, rengbAudio);
				detritusSystem->Update(frameInfo, rengbDevice);
				enemySystem->followPath(frameInfo, playerTransform.translation);
				physicsSystem->Update(frameTime);
				collisionSystem->Update(frameInfo);
				doorSystem->Update(pshove);
				gameUI.updateWeaponBoard();
				gameUI.updateStatMonitor();

				traversalSystem->renderGameObjects(frameInfo, time, rengbWindow.getExtent().width, rengbWindow.getExtent().height, cumFlash);

				renderSystem->renderGameObjects(frameInfo, time, rengbWindow.getExtent().width, rengbWindow.getExtent().height, cumFlash);
				enemySystem->renderEnemies(frameInfo, playerTransform.translation);
				projectileSystem->renderGameObjects(frameInfo);
				heldSystem->renderHeldObjects(frameInfo, time, rengbWindow.getExtent().width, rengbWindow.getExtent().height, cumFlash);
				pickupSystem->renderGameObjects(frameInfo, time);

				cumFlash = glm::vec4(0.f);
				blastSystem->renderBlast(frameInfo, cumFlash);
				guiSystem->renderUI(frameInfo, time);

				rengbRenderer.endSwapChainRenderPass(commandBuffer);
				
				rengbRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(rengbDevice.device());

		// Clean Up
		gCoordinator.ClearArray<RigidBody>();
		gCoordinator.ClearArray<Transform>();
		gCoordinator.ClearArray<Renderable>();
		gCoordinator.ClearArray<CollisionShape>();
		gCoordinator.ClearArray<EnemyParent>();
		gCoordinator.ClearArray<Renderable>();
		gCoordinator.ClearArray<EnemyPart>();
		gCoordinator.ClearArray<World>();
		gCoordinator.ClearArray<Player>();
		gCoordinator.ClearArray<Terrain>();
		gCoordinator.ClearArray<Renderable>();
		gCoordinator.ClearArray<Door>();
		gCoordinator.ClearArray<Room>();

		traversalSystem->destroySystem();
		renderSystem->destroySystem();
		enemySystem->destroySystem();
		heldSystem->destroySystem();
		projectileSystem->destroySystem();
		blastSystem->destroySystem();
		guiSystem->destroySystem();
		pickupSystem->destroySystem();

		RengbModel::clearModelCache();
	}



} // namespace rengb