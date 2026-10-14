#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <fstream>

#include "rengb_device.hpp"

#include "rengb_system.hpp"

#include <random>


#include "core/rengb_coordinator.hpp"
#include "systems/rengb_traversal_system.hpp"

#include <json.hpp>

using json = nlohmann::json;

namespace rengb
{
    
    class RengbJson
    {
        public:
            static Entity generateEnemy(std::string name, RengbDevice &device, glm::vec3 initialPosition, Entity parentRoom, bool active);
            static void generateLevel(std::string name, RengbDevice &device, int limit = 1, Entity originatingDoorID = 0, int depth = 0); 
            static Entity generatePlayer(std::string name, RengbDevice &device);
            // 0 is default case for recursion. -1 is impossible for the entity 
            // Make it so that if it reaches a certain depth it guarantees a room, 
            // and if it reaches the end, return the entity of the door 
            // so that it can be put onto a stack which can be used to trace a path to show how to get to the goal.
        private:   
    };
}

