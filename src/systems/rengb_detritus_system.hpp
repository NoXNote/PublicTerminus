#pragma once
#include "rengb_system.hpp"
#include <glm/glm.hpp>
#include "unordered_set"
#include "rengb_device.hpp"
#include "rengb_renderer.hpp"
#include "rengb_frame_info.hpp"

namespace rengb{
    class DetritusSystem : public System
    {
    public:
	    //void Init();
        void killDet(RengbRenderer &renderer, RengbDevice &device);
	    void Update(FrameInfo &frameInfo, RengbDevice &device);
        std::vector<std::pair<size_t, Entity>> detToKill; // stores last used frame index as well as entity
    };
}
