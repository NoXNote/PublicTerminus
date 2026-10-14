#pragma once
#include <vulkan/vulkan.h>
#include "rengb_window.hpp"
#include <vulkan/vulkan_core.h> 

#define XR_USE_GRAPHICS_API_VULKAN

// #include <openxr/openxr_platform.h>

// std lib headers
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <set>

namespace rengb
{
	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices
	{
		uint32_t graphicsFamily;
		uint32_t presentFamily;
		bool graphicsFamilyHasValue = false;
		bool presentFamilyHasValue = false;
		bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
	};

	class RengbDevice
	{
		public:
			#ifdef NDEBUG
			const bool enableValidationLayers = false;
			#else
			const bool enableValidationLayers = true;
			#endif

			RengbDevice(RengbWindow &window);
			~RengbDevice();

			// Not copyable or movable
			RengbDevice(const RengbDevice &) = delete;
			RengbDevice &operator=(const RengbDevice &) = delete;
			RengbDevice(RengbDevice &&) = delete;
			RengbDevice &operator=(RengbDevice &&) = delete;

			VkCommandPool getCommandPool() { return commandPool; }
			VkDevice device() { return device_; }
			VkSurfaceKHR surface() { return surface_; }
			VkQueue graphicsQueue() { return graphicsQueue_; }
			VkQueue presentQueue() { return presentQueue_; }

			SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
			uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
			QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
			VkFormat findSupportedFormat(
				const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

			// Buffer Helper Functions
			void createBuffer(
				VkDeviceSize size,
				VkBufferUsageFlags usage,
				VkMemoryPropertyFlags properties,
				VkBuffer &buffer,
				VkDeviceMemory &bufferMemory);
			VkCommandBuffer beginSingleTimeCommands();
			void endSingleTimeCommands(VkCommandBuffer commandBuffer);
			void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
			void copyBufferToImage(
				VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

			void createImageWithInfo(
				const VkImageCreateInfo &imageInfo,
				VkMemoryPropertyFlags properties,
				VkImage &image,
				VkDeviceMemory &imageMemory);

			VkPhysicalDeviceProperties properties;
			VkPhysicalDeviceProperties2 properties2;
			VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservativeRasterProperties;



		private:
			void createInstance();
			void setupDebugMessenger();
			void createSurface();
			void pickPhysicalDevice();
			void createLogicalDevice();
			void createCommandPool();

			//  ===========================================================OPEN XR
			// void rEngXRCreateInstance();
			// void rEngXRDestroyInstance();
			// void rEngXRSetSystem();
			// XrSystemId systemID;
			// XrInstance rEngXRInstance;
			// XrGraphicsRequirementsVulkanKHR rEngXRGraphicsRequirements;
			// std::set<std::string> rEngXRInstanceExtensions;
			// std::set<std::string> rEngXRDeviceExtensions;

			// void rEngXRSetVulkanInstanceRequirements();
			// void rEngXRGetVulkanDeviceRequirements();

			// PFN_xrVoidFunction getXRFunction(const char *name)
			// {
			// 	PFN_xrVoidFunction func;

			// 	XrResult result = xrGetInstanceProcAddr(rEngXRInstance, name, &func);

			// 	if (result != XR_SUCCESS)
			// 	{
			// 		std::cerr << "Failed to load OpenXR extension function '" << name << "': " << result << std::endl;
			// 		return nullptr;
			// 	}

			// 	return func;
			// };

			// ===========================================================end OPEN XR
			// helper functions
			bool isDeviceSuitable(VkPhysicalDevice device);
			std::vector<const char *> getRequiredExtensions();
			bool checkValidationLayerSupport();
			QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
			void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
			void hasGflwRequiredInstanceExtensions();
			bool checkDeviceExtensionSupport(VkPhysicalDevice device);
			SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

			VkInstance instance;
			VkDebugUtilsMessengerEXT debugMessenger;
			VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
			RengbWindow &window;
			VkCommandPool commandPool;

			VkDevice device_;
			VkSurfaceKHR surface_;
			VkQueue graphicsQueue_;
			VkQueue presentQueue_;

			const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
			const std::vector<const char *> deviceExtensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME, 
				VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME,
				VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME, // dependancy of shadingrate
				//VK_KHR_MULTIVIEW_EXTENSION_NAME, // dependancy of renderpass2
				//VK_KHR_MAINTENANCE2_EXTENSION_NAME, // dependancy of extension name
				//VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME,
				//VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME // dependancy of shadingrate
				
				};
	};

}  // namespace rengb