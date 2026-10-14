#pragma once
#include "miniaudio.h"
#include "rengb_utils.hpp"
#include "rengb_model.hpp"
#include "rengb_audio.hpp"

namespace rengb
{
	class GunObjectComponent
	{
		public:
		float recoil{1.f};
		float rateDelay{0.02f};
		float timeSinceFire{0.f};
		int barrelToFire{0}; // -1 to fire all
		glm::vec3 downSightsMovement{0.f};
		glm::vec3 downSightsRotation{0.f};
		bool shooting{false};
		float damage{20.f};


		std::vector<std::tuple<glm::vec3, float>> muzzlePositions{};

		std::vector<std::shared_ptr<RengbModel>> flashParts{}; 
		float flashSize{1.f};
		float flashBrightness{1.f};
		std::vector<std::shared_ptr<RengbModel>> flareParts{};

		glm::vec3 recoilMovement{0.f};
		glm::vec3 wavering{0.f};

		
		int maxAmmo{200};
		int ammo{200};
		float reloadTimer{0.f};
		float reloadDuration{1.f};
		std::string bulletLook{" -"};
		int segmentCount{8};

		float passiveReloadDelay{1.f};

		int ID{0};
		int type{0}; // 0 for single shot, 1 for mini

		ma_sound* sound = nullptr;
		bool currentlyShooting{false};

		void shoot(float dt)
		{
			barrelToFire = (++barrelToFire) % muzzlePositions.size();
			auto typeA = [&] () {
				// single shot
				recoilMovement = glm::vec3{((float)(rand()) / (float)(RAND_MAX))-0.5f, ((float)(rand()) / (float)(RAND_MAX))-0.5f,-1.f}*recoil/512.f;
				wavering += recoilMovement;
			};
			auto typeB = [&] () {
				// minigun
				recoilMovement = glm::vec3{((float)(rand()) / (float)(RAND_MAX))-0.5f, ((float)(rand()) / (float)(RAND_MAX))-0.5f,-1.f}*recoil/512.f;
				wavering += recoilMovement;
			};

			--ammo;

			if(barrelToFire == -1)
			{
				for(auto& barrels : muzzlePositions)
				{
					std::get<1>(barrels) = flashSize;
				}
			} else
			{
				std::get<1>(muzzlePositions[barrelToFire]) = flashSize;
				shooting = true;
			}

			switch(type)
			{
				case 0:
					typeA();
					break;
				case 1:
					typeB();
					break;
				default:
					typeA();
					break;
			}
			
			timeSinceFire = 0.f;			
			if (ammo==0) reloadTimer = reloadDuration;	
	}
	};
}
