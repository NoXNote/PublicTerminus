#pragma once
#include "rengb_system.hpp"
#include "rengb_window.hpp"
#include "rengb_audio.hpp"
#include "rengb_device.hpp"
#include <glm/glm.hpp>

namespace rengb
{
	class PlayerSystem : public System
	{
	public:
		// void Init();
		void Update(GLFWwindow *window, float dt, RengbAudio &audio);
		void SwapWeapon(RengbDevice &device, Entity entity, std::string weaponName, RengbAudio &audio);

	private:
		float rollingAdder{0.f};
	};
}