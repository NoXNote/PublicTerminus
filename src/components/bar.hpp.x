#pragma once

#include <glm/glm.hpp>
#include <string>
#include "rengb_model.hpp"

namespace rengb{
struct Bar {
    float length;
    float height;
    float depth;
    std::vector<RengbModel::Vertex> vertices;

    Bar(float length, float height, float depth, glm::vec3 color, glm::vec3 flashColor) 
        : length(length), height(height), depth(depth) {
        // Define the 8 vertices of the cube (you can modify the color logic as needed)


        // Front face
        vertices.push_back({glm::vec3(0.0f, 0.0f, 0.0f), color, flashColor});           // Bottom-left
        vertices.push_back({glm::vec3(length, 0.0f, 0.0f), color, flashColor});        // Bottom-right
        vertices.push_back({glm::vec3(length, height, 0.0f), color, flashColor});      // Top-right
        vertices.push_back({glm::vec3(0.0f, height, 0.0f), color, flashColor});       // Top-left

        // Back face
        vertices.push_back({glm::vec3(0.0f, 0.0f, -depth), color, flashColor});       // Bottom-left
        vertices.push_back({glm::vec3(length, 0.0f, -depth), color, flashColor});      // Bottom-right
        vertices.push_back({glm::vec3(length, height, -depth), color, flashColor});    // Top-right
        vertices.push_back({glm::vec3(0.0f, height, -depth), color, flashColor});     // Top-left
    }
        
    };
}

