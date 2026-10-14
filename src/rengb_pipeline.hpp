#pragma once

#include "rengb_device.hpp"

// std
#include <string>
#include <vector>

namespace rengb
{

	struct PipelineConfigInfo
	{
		PipelineConfigInfo() = default;
		PipelineConfigInfo(const PipelineConfigInfo &) = delete;
		PipelineConfigInfo &operator=(const PipelineConfigInfo &) = delete;

		std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		// ----------- my things ---------------------
		VkPipelineFragmentShadingRateStateCreateInfoKHR fragmentShadingRateInfo;
		VkPipelineRasterizationConservativeStateCreateInfoEXT conservativeRasterizationState;
		// ----------------------------------------
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class RengbPipeline
	{
	public:
		RengbPipeline(
			RengbDevice &device,
			const std::string &vertFilepath,
			const std::string &fragFilepath,
			const PipelineConfigInfo &configInfo,
			const VkSpecializationInfo* vertSpecializationInfo = nullptr,
			const VkSpecializationInfo* fragSpecializationInfo = nullptr);
		~RengbPipeline();

		RengbPipeline(const RengbPipeline &) = delete;
		RengbPipeline &operator=(const RengbPipeline &) = delete;

		void bind(VkCommandBuffer commandBuffer);

		static void defaultPipelineConfigInfo(PipelineConfigInfo &configInfo);
		static void enableAlphaBlending(PipelineConfigInfo &configInfo);
		static void disableDepthStencil(PipelineConfigInfo &configInfo);
		static void enableDepthWriteRead(PipelineConfigInfo &configInfo);
		static void enableConservativeRasterisation(PipelineConfigInfo &configInfo);

	private:
		static std::vector<char> readFile(const std::string &filepath);

		void createGraphicsPipeline(
			const std::string &vertFilepath,
			const std::string &fragFilepath,
			const PipelineConfigInfo &configInfo,
			const VkSpecializationInfo* vertSpecializationInfo = nullptr,
			const VkSpecializationInfo* fragSpecializationInfo = nullptr);

		void createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);

		RengbDevice &rengbDevice;
		VkPipeline graphicsPipeline;
		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;
	};
} // namespace rengb