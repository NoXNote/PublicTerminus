#pragma once

#include "miniaudio.h"
#include <queue>
#include <json.hpp>
#include <string>
#include <fstream>
#include <glm/glm.hpp>

using json = nlohmann::json;

namespace rengb{
    class RengbAudio
    {
	public:
        RengbAudio();
		~RengbAudio();

        static constexpr size_t MAX_SHOTS = 16;
        static constexpr size_t MAX_WALKING = 18;
        static constexpr size_t MAX_EXPLODE = 16;

        void LoadAudio(std::string tracklist);

        ma_sound* ShootSound(int type, glm::vec3 position, bool loop);

        void CheckAllSounds();

        void StopLoop(ma_sound* toStop);
        void UpdateSoundPosition(ma_sound* toUpdate, glm::vec3 position);

        void PlayWindDown(glm::vec3 position);
        void UpdateWindDown(glm::vec3 position);

        void PlayHitSound(glm::vec3 position);
        void PlayShieldUp();
        void PlayShieldDown();

        void PlayReloadClick(glm::vec3 position);
        void UpdateReloadClick(glm::vec3 position);

        void PlayReloadComplete();

        void PlayItemPickUp();
        void PlayLevelUp();

        void Update(glm::vec3 position, glm::vec3 velocity, glm::vec3 orientation, glm::mat4 matrix);
        

	private:

        void setPositional(ma_sound *sound, glm::vec3 position);
        void setNonPositional(ma_sound *sound, glm::vec3 position);
        ma_engine engine;

        ma_sound bgm1, enemy;

        ma_sound startFire[MAX_SHOTS], 
            lightFire[MAX_SHOTS], mediumFire[MAX_SHOTS], heavyFire[MAX_SHOTS], 
            miniLightFire[MAX_SHOTS], miniMediumFire[MAX_SHOTS], miniHeavyFire[MAX_SHOTS],
            explode[MAX_EXPLODE], walking[MAX_WALKING];
        // player sounds

        ma_sound reloadClick, utility, windDown, hitSound, pickUp, levelUp, shieldUp, shieldDown;

        std::queue<size_t> availableStartShots, 
            availableLightShots, availableMediumShots, availableHeavyShots, 
            availableMiniLightShots, availableMiniMediumShots, availableMiniHeavyShots,
            availableExplode, availableWalking;


        glm::vec3 playerPosition{};
        glm::mat4 playerMatrix{};
	};
}
