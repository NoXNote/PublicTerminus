#define MINIAUDIO_IMPLEMENTATION

#define DELAY_IN_SECONDS 0.2f
#define DECAY 0.8f /* Volume falloff for each echo. */

#include "rengb_audio.hpp"
#include <stdexcept>
#include <iostream>

namespace rengb
{
	RengbAudio::RengbAudio()
	{
        if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
            std::cerr << "Could Not Initialise Audio System" << std::endl;
        }
        ma_delay_node_config delayNodeConfig;
        ma_lpf_node_config lowPassNodeConfig;
        ma_uint32 channels;
        ma_uint32 sampleRate;
    
        channels = ma_engine_get_channels(&engine);
        sampleRate = ma_engine_get_sample_rate(&engine);

        // Set the listener's position.
        ma_engine_listener_set_position(&engine, 0, 0.0f, 0.0f, 1.0f);
    
    
        // Set the listener's forward and up vectors for orientation.
        ma_engine_listener_set_direction(&engine, 0, 0.0f, 0.0f, -1.0f); // Looking down -Z.
        ma_engine_listener_set_velocity(&engine, 0, 0.0f, 0.0f, 0.0f);   // Stationary listener.
        
	}

    void RengbAudio::LoadAudio(std::string trackList)
    {
        std::ifstream f("../data/" + trackList + ".json");
		json data = json::parse(f);

        std::string filepath{"../sounds/song/" + (std::string)(data["bgm"])};

        if(ma_sound_init_from_file(
            &engine, 
            filepath.c_str(), 
            MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION, // prevents filling memory
            NULL, NULL, 
            &bgm1) != MA_SUCCESS)
        {
            std::cerr << "Could Not Load Background Audio:" << filepath << std::endl;
        } 
        else
        {
            ma_sound_set_start_time_in_pcm_frames(&bgm1, 0);
            ma_sound_set_volume(&bgm1, 0.1f);
            ma_sound_start(&bgm1);
        }

        auto init = [&] (std::string const path, ma_uint32 flags, ma_sound* sound) 
        {
            if(ma_sound_init_from_file(
                &engine, 
                path.c_str(), 
                flags, // prevents filling memory
                NULL, NULL, 
                sound) != MA_SUCCESS)
            {
                std::cerr << "Could Not Load Audio: " << filepath << std::endl;
            }
        };

        init("../sounds/song/" + (std::string)(data["enemy"]), MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION, &enemy);
        init("../sounds/minigun_wind_down.wav", 0, &windDown);

        init("../sounds/medigun_no_target.wav", MA_SOUND_FLAG_NO_SPATIALIZATION, &utility);
        ma_sound_set_volume(&utility, 0.1f);

        init("../sounds/bison_reload.wav", MA_SOUND_FLAG_NO_SPATIALIZATION, &reloadClick);
        ma_sound_set_volume(&reloadClick, 0.1f);

        init("../sounds/Hitsound.wav", 0, &hitSound);
        ma_sound_set_volume(&hitSound, 2.f);

        init("../sounds/grenade_launcher_drum_load.wav", MA_SOUND_FLAG_NO_SPATIALIZATION, &pickUp);
        ma_sound_set_volume(&pickUp, 0.05f);
        init("../sounds/sentry_finish.wav", MA_SOUND_FLAG_NO_SPATIALIZATION, &levelUp);
        ma_sound_set_volume(&levelUp, 0.1f);

        init("../sounds/buffed_on.wav", MA_SOUND_FLAG_NO_SPATIALIZATION, &shieldUp);        
        init("../sounds/buffed_off.wav", MA_SOUND_FLAG_NO_SPATIALIZATION, &shieldDown);


        { // =================================WEAPONS=================================
            for (size_t i = 0; i < MAX_SHOTS; ++i)
            {
                // Iterates over range of available sounds
                std::string suffix = std::to_string((i%3+1)) + ".wav";
                std::string lightPath = "../sounds/reserve/reserve_shooter_0" + suffix;
                std::string mediumPath = "../sounds/widow/widow_maker_shot_0" + suffix;
                std::string heavyPath = "../sounds/diamond/diamond_back_0" + suffix;

                init("../sounds/enforcer_shoot.wav", 0, &startFire[i]);
                availableStartShots.push(i);

                init(lightPath, 0, &lightFire[i]);
                availableLightShots.push(i);
        
                init(mediumPath, 0, &mediumFire[i]);
                availableMediumShots.push(i);

                init(heavyPath, 0, &heavyFire[i]);
                availableHeavyShots.push(i);

                init("../sounds/minifun_shoot.wav", 0, &miniLightFire[i]);
                availableMiniLightShots.push(i);

                init("../sounds/minigun_shoot.wav", 0, &miniMediumFire[i]);
                availableMiniMediumShots.push(i);

                init("../sounds/gatling_shoot.wav", 0, &miniHeavyFire[i]);
                availableMiniHeavyShots.push(i);
            }
        }
        { // =================================EXPLOSIONS=================================
            for (size_t i = 0; i < MAX_EXPLODE; ++i)
            {
                std::string explodePath = "../sounds/explode/explodes_0" + std::to_string((i%2+1)) + ".wav";

                init(explodePath, 0, &explode[i]);
                ma_sound_set_volume(&explode[i], 2.f);

                availableExplode.push(i);
            }
        }
        { // ================================WALKING=====================================
            for (size_t i = 0; i < MAX_WALKING; ++i)
            {
                std::ostringstream oss;
                oss << "../sounds/walking/step_" << std::setw(2) << std::setfill('0') << (i%18)+1 << ".wav";
                std::string walkPath = oss.str();

                init(walkPath, 0, &walking[i]);
                ma_sound_set_volume(&walking[i], 1.f);

                availableWalking.push(i);
            }
        }
    }

    void RengbAudio::StopLoop(ma_sound* toStop)
    {
        std::cout<<"stopping sound!"<<std::endl;
        ma_sound_set_looping(toStop, MA_FALSE);
        ma_sound_stop(toStop);
    }

    void RengbAudio::UpdateSoundPosition(ma_sound* toUpdate, glm::vec3 position)
    {
        ma_sound_set_position(toUpdate, position.x, position.y, position.z);
    }

    ma_sound* RengbAudio::ShootSound(int type, glm::vec3 position, bool loop = false)
    {
        auto trySound = [&] (std::queue<size_t> &soundQueue, ma_sound (&soundPool)[]) -> ma_sound* {
            if (!soundQueue.empty())
            {
                int index = soundQueue.front();
                soundQueue.pop();
                std::cout<<"SHOT INDEX : " << index << std::endl;
                ma_sound_set_start_time_in_pcm_frames(&soundPool[index], 0);
                ma_sound_start(&soundPool[index]);
                ma_sound_set_looping(&soundPool[index], loop);

                setPositional(&soundPool[index], position);

                std::cout<<"Given the POINTER"<<std::endl;
                return &(soundPool[index]);
            }
            return nullptr;
        }; 

        switch(type)
        {
            case 0: // starter weapon
                return trySound(availableStartShots, startFire);
                break;
            case 1: // cannon
                return trySound(availableLightShots, lightFire);
            case 2: // dual cannon
                return trySound(availableMediumShots, mediumFire);
                break;
            case 3: // max cannon
                return trySound(availableHeavyShots, heavyFire);
                break;
            case 4: // minigun
                return trySound(availableMiniLightShots, miniLightFire);
                break;
            case 5: // dual minigun
                return trySound(availableMiniMediumShots, miniMediumFire);
                break;
            case 6: // max minigun
                return trySound(availableMiniHeavyShots, miniHeavyFire);
                break;
            case 100: // explode
                return trySound(availableExplode, explode);
                break;
            case 101: // walk
                std::cout<<"Trying to walk"<<std::endl;
                return trySound(availableWalking, walking);
                break;
        }
        return nullptr;
    }

    void RengbAudio::setPositional(ma_sound* sound, glm::vec3 position)
    {
        ma_sound_set_position(sound, position.x, position.y, position.z);
    }

    void RengbAudio::setNonPositional(ma_sound* sound, glm::vec3 position)
    {
        glm::vec4 correctedPosition = playerMatrix * glm::vec4{position, 1.f};
        ma_sound_set_position(sound, correctedPosition.x, correctedPosition.y, correctedPosition.z);
    }

    void RengbAudio::CheckAllSounds()
    {
        for(size_t i = 0; i < MAX_SHOTS; ++i)
        {
            if(!ma_sound_is_playing(&startFire[i])) availableStartShots.push(i);
            if(!ma_sound_is_playing(&lightFire[i])) availableLightShots.push(i);
            if(!ma_sound_is_playing(&mediumFire[i])) availableMediumShots.push(i);
            if(!ma_sound_is_playing(&heavyFire[i])) availableHeavyShots.push(i);
            if(!ma_sound_is_playing(&miniLightFire[i])) availableMiniLightShots.push(i);    
            if(!ma_sound_is_playing(&miniMediumFire[i])) availableMiniMediumShots.push(i);    
            if(!ma_sound_is_playing(&miniHeavyFire[i])) availableMiniHeavyShots.push(i);   
            if(!ma_sound_is_playing(&explode[i])) availableExplode.push(i); 
            if(!ma_sound_is_playing(&walking[i])) availableWalking.push(i); 
            

        }
    }

    void RengbAudio::PlayWindDown(glm::vec3 position)
    {
        ma_sound_stop(&windDown);
        ma_sound_seek_to_pcm_frame(&windDown, 0);
        setPositional(&windDown, position);
        ma_sound_start(&windDown);
    }

    void RengbAudio::UpdateWindDown(glm::vec3 position)
    {
        setPositional(&windDown, position);
    }

    void RengbAudio::PlayHitSound(glm::vec3 position)
    {
        ma_sound_stop(&hitSound);
        ma_sound_seek_to_pcm_frame(&hitSound, 0);
        // ma_sound_set_position(&hitSound, position.x, position.y, position.z);
        setPositional(&hitSound, position);
        ma_sound_start(&hitSound);
    }

    void RengbAudio::PlayReloadClick(glm::vec3 position)
    {
        ma_sound_set_start_time_in_pcm_frames(&reloadClick, 0);
        setPositional(&windDown, position);
        ma_sound_start(&reloadClick);
    }
    void RengbAudio::UpdateReloadClick(glm::vec3 position)
    {
        setPositional(&reloadClick, position);
    }    
    void RengbAudio::PlayReloadComplete()
    {
        ma_sound_stop(&utility);
        ma_sound_set_pitch(&utility, 1.2f);
        ma_sound_seek_to_pcm_frame(&utility, 0);
        ma_sound_start(&utility);
    }
    void RengbAudio::PlayItemPickUp()
    {
        ma_sound_stop(&pickUp);
        ma_sound_seek_to_pcm_frame(&pickUp, 0);
        ma_sound_start(&pickUp);
    }
    void RengbAudio::PlayLevelUp()
    {
        ma_sound_seek_to_pcm_frame(&levelUp, 0);
        ma_sound_start(&levelUp);
    }

    void RengbAudio::PlayShieldUp()
    {
        ma_sound_seek_to_pcm_frame(&shieldUp, 0);
        ma_sound_start(&shieldUp);
    }

    void RengbAudio::PlayShieldDown()
    {
        ma_sound_seek_to_pcm_frame(&shieldDown, 0);
        ma_sound_start(&shieldDown);
    }    
    void RengbAudio::Update(glm::vec3 position, glm::vec3 velocity, glm::vec3 orientation, glm::mat4 matrix)
    {
        playerPosition = position;
        playerMatrix = matrix;
        float cosPitch = cos(orientation.x);
        float sinPitch = sin(orientation.x);
        float cosYaw = cos(orientation.y);
        float sinYaw = sin(orientation.y);
    
        glm::vec3 forward;
        forward.x = sinYaw * cosPitch;
        forward.y = -sinPitch;
        forward.z = cosYaw * cosPitch;

        forward = -glm::normalize(forward);

        ma_engine_listener_set_position(&engine, 0, position.x, position.y, position.z);
        ma_engine_listener_set_direction(&engine, 0, forward.x, forward.y, forward.z);
        ma_engine_listener_set_velocity(&engine, 0, velocity.x, velocity.y, velocity.z); 
    }

	RengbAudio::~RengbAudio()
	{
        for(size_t i = 0; i < MAX_SHOTS; ++i)
        {
            ma_sound_uninit(&lightFire[i]);
            ma_sound_uninit(&mediumFire[i]);
            ma_sound_uninit(&heavyFire[i]);
        }

        ma_sound_uninit(&bgm1);
        ma_sound_uninit(&enemy);

        ma_engine_uninit(&engine);

	}
}
