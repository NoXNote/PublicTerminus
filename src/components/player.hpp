#pragma once

#include <glm/glm.hpp>

namespace rengb
{
	struct Player
	{
		bool sprinting{};
		bool crouching{};
		bool onGround{};
		bool hasLeftGround{false};

		float targetSpeed{};
		float moveSpeed;
		float yaw{0.f};
		float pitch{0.f};
		glm::vec3 headBob{};
		glm::vec3 armPump{};
		glm::vec3 velocity{};
		glm::vec2 absoluteMovement{};
		bool downSights;

		Entity arm;
		Entity offarm;
		Entity gun;
		std::vector<std::string> equipedArtefacts{"dropmag", "dropmag", "dropmag", "dropmag"}; // maybe use an enum later

		Entity inRoom;

		float maxHP{200};
		float maxShield{40};

		float hp{21};
		float shield{3};

		float rollingHP{200};
		float rollingShield{40};

		float expHeld{0};
		float nextLevelEXP{20};
		int currentLevel{1};

		float timeSinceHit{0.f};
		float shieldRechargeTime{5.f};

		float stepLoop{0.f};
		float bobloop{0.f};

		float crouchDistance{0.f}; // how far the player is currently crouching, caps at crouchLimit

		float walkSpeed{10.f}; // 10 normal
		float jumpHeight{7.f};
		float sensitivity{0.005f};
		float crouchLimit{1.5f};
		float height{3.0f};
		float bobSpeed{2.f};
	};
}
