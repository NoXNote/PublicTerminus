#pragma once

#include "rengb_camera.hpp"
#include "rengb_game_object.hpp"

// lib
#include <vulkan/vulkan.h>

namespace rengb {

#define MAX_LIGHTS 10

// struct PointLight {
//   glm::vec4 position{};  // ignore w
//   glm::vec4 color{};     // w is intensity
// };

struct GlobalUbo {
  glm::mat4 projection{1.f};
  glm::mat4 view{1.f};
  //glm::vec4 ambientLightColor{1.f, 1.f, 1.f, 0.008f};  // w is intensity
  // PointLight pointLights[MAX_LIGHTS];
  // int numLights;
};

struct FrameInfo {
  int frameIndex;
  float frameTime;
  VkCommandBuffer commandBuffer;
  RengbCamera &camera;
  VkDescriptorSet globalDescriptorSet;
  //RengbGameObject::Map &gameObjects;
};
}  // namespace rengb