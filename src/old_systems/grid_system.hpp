#pragma once

#include "rengb_camera.hpp"
#include "rengb_device.hpp"
#include "rengb_frame_info.hpp"
#include "rengb_game_object.hpp"
#include "rengb_pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace rengb {
class GridSystem {
 public:
  GridSystem(
      RengbDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
  ~GridSystem();

  GridSystem(const GridSystem &) = delete;
  GridSystem &operator=(const GridSystem &) = delete;

  // void update(FrameInfo &frameInfo, GlobalUbo &ubo);
  void render(FrameInfo &frameInfo, glm::vec4 cumFlash, glm::vec3 playerPo);

 private:
  void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
  void createPipeline(VkRenderPass renderPass);

  RengbDevice &rengbDevice;

  std::unique_ptr<RengbPipeline> rengbPipeline;
  VkPipelineLayout pipelineLayout;
};
}  // namespace rengb