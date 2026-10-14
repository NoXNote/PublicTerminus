#pragma once

#include "rengb_device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace rengb {

class RengbDescriptorSetLayout {
 public:
  class Builder {
   public:
    Builder(RengbDevice &rengbDevice) : rengbDevice{rengbDevice} {}

    Builder &addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count = 1);
    std::unique_ptr<RengbDescriptorSetLayout> build() const;

   private:
    RengbDevice &rengbDevice;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
  };

  RengbDescriptorSetLayout(
      RengbDevice &rengbDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
  ~RengbDescriptorSetLayout();
  RengbDescriptorSetLayout(const RengbDescriptorSetLayout &) = delete;
  RengbDescriptorSetLayout &operator=(const RengbDescriptorSetLayout &) = delete;

  VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

 private:
  RengbDevice &rengbDevice;
  VkDescriptorSetLayout descriptorSetLayout;
  std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

  friend class RengbDescriptorWriter;
};

class RengbDescriptorPool {
 public:
  class Builder {
   public:
    Builder(RengbDevice &rengbDevice) : rengbDevice{rengbDevice} {}

    Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
    Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
    Builder &setMaxSets(uint32_t count);
    std::unique_ptr<RengbDescriptorPool> build() const;

   private:
    RengbDevice &rengbDevice;
    std::vector<VkDescriptorPoolSize> poolSizes{};
    uint32_t maxSets = 1000;
    VkDescriptorPoolCreateFlags poolFlags = 0;
  };

  RengbDescriptorPool(
      RengbDevice &rengbDevice,
      uint32_t maxSets,
      VkDescriptorPoolCreateFlags poolFlags,
      const std::vector<VkDescriptorPoolSize> &poolSizes);
  ~RengbDescriptorPool();
  RengbDescriptorPool(const RengbDescriptorPool &) = delete;
  RengbDescriptorPool &operator=(const RengbDescriptorPool &) = delete;

  bool allocateDescriptorSet(
      const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

  void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

  void resetPool();

 private:
  RengbDevice &rengbDevice;
  VkDescriptorPool descriptorPool;

  friend class RengbDescriptorWriter;
};

class RengbDescriptorWriter {
 public:
  RengbDescriptorWriter(RengbDescriptorSetLayout &setLayout, RengbDescriptorPool &pool);

  RengbDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
  RengbDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

  bool build(VkDescriptorSet &set);
  void overwrite(VkDescriptorSet &set);

 private:
  RengbDescriptorSetLayout &setLayout;
  RengbDescriptorPool &pool;
  std::vector<VkWriteDescriptorSet> writes;
};

}  // namespace rengb