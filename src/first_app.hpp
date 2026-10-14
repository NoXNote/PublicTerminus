#pragma once

#include "rengb_descriptors.hpp"
#include "rengb_device.hpp"
#include "rengb_frame_info.hpp"
#include "rengb_renderer.hpp"
#include "rengb_window.hpp"
#include "rengb_json.hpp"
#include "rengb_audio.hpp"



// std
#include <memory>
#include <vector>

namespace rengb
{
	class FirstApp
	{
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		FirstApp();
		~FirstApp();

		FirstApp(const FirstApp &) = delete;
		FirstApp &operator=(const FirstApp &) = delete;

		void run();
		void swapGun();

	private:
		void loadGameObjects(std::vector<Entity> &entities);
		void loadGameObjectsNew(std::vector<Entity> &entities);
		void configureCoordinator();
		static void playMultipleAudio(std::string filename, int range);
		static void playLoopAudio(std::string filename);

		RengbWindow rengbWindow{WIDTH, HEIGHT, "Terminus: NEA Project"};
		RengbDevice rengbDevice{rengbWindow};
		RengbRenderer rengbRenderer{rengbWindow, rengbDevice};
		RengbAudio rengbAudio{};

		// note: order of declarations matters
		std::unique_ptr<RengbDescriptorPool> globalPool{};
		
		//RengbGameObject::Map gameObjects;
	};
} // namespace rengb