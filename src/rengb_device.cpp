#include "rengb_device.hpp"

// vulkan header


// std headers
#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>

// namespace
// {
// 	constexpr XrViewConfigurationType viewType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
// 	constexpr XrEnvironmentBlendMode environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;

// 	const std::string applicationName = "rEngXR";
// } // namespace

namespace rengb
{

	// local callback functions
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
		void *pUserData)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
		const VkAllocationCallbacks *pAllocator,
		VkDebugUtilsMessengerEXT *pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance,
			"vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks *pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance,
			"vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

	// class member functions
	RengbDevice::RengbDevice(RengbWindow &window) : window{window}
	{
		// xr

		// rEngXRCreateInstance();
		// rEngXRSetSystem();
		// rEngXRSetVulkanInstanceRequirements();
		// // rEngXRGetVulkanDeviceRequirements();
		// std::cout<<std::endl<<"all good for XR!"<<std::endl<<std::endl;
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createCommandPool();
	}

	RengbDevice::~RengbDevice()
	{
		//rEngXRDestroyInstance();

		vkDestroyCommandPool(device_, commandPool, nullptr);
		vkDestroyDevice(device_, nullptr);

		if (enableValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(instance, surface_, nullptr);
		vkDestroyInstance(instance, nullptr);
	}

	

	void RengbDevice::createInstance()
	{
		if (enableValidationLayers && !checkValidationLayerSupport())
		{
			throw std::runtime_error("validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "RotaryEngineBeta App";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create instance!");
		}

		hasGflwRequiredInstanceExtensions();
	}

	void RengbDevice::pickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0)
		{
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}
		std::cout << "Device count: " << deviceCount << std::endl;
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto &device : devices)
		{
			if (isDeviceSuitable(device))
			{
				physicalDevice = device;
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
		}

		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		std::cout << "physical device: " << properties.deviceName << std::endl;



		// vkGetPhysicalDeviceProperties(physicalDevice, &properties);

		// VkPhysicalDeviceFragmentShadingRatePropertiesKHR shadingRateProperties = {};
		// shadingRateProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR;

		// conservativeRasterProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT;

		// properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		// properties2.pNext = &shadingRateProperties;
		// shadingRateProperties.pNext = &conservativeRasterProperties;

		// vkGetPhysicalDeviceProperties2(physicalDevice, &properties2);


		// std::cout << "physical device: " << properties.deviceName << std::endl;
		// std::cout << "max fragment width: " << shadingRateProperties.maxFragmentSize.width << std::endl;

		// if (!conservativeRasterProperties.fullyCoveredFragmentShaderInputVariable)
		// {
		// 	std::cerr << "Conservative rasterisation is not fully supported on this device." << std::endl;
		// } else {
		// 	std::cout << "Conservative rasterisation supported!" << std::endl << conservativeRasterProperties.maxExtraPrimitiveOverestimationSize << std::endl;
		// }
    		
	}

	void RengbDevice::createLogicalDevice()
	{
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragmentShadingRateFeatures = {};
		fragmentShadingRateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR;
		fragmentShadingRateFeatures.pipelineFragmentShadingRate = VK_TRUE;	  // Enable the feature
		fragmentShadingRateFeatures.attachmentFragmentShadingRate = VK_FALSE; // Optional
		fragmentShadingRateFeatures.primitiveFragmentShadingRate = VK_FALSE;  // Optional


		// VkPhysicalDeviceFeatures deviceFeatures = {};
		// deviceFeatures.samplerAnisotropy = VK_TRUE;

		// Chain the feature structure to VkDeviceCreateInfo
		VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
		deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		deviceFeatures2.pNext = &fragmentShadingRateFeatures;
		deviceFeatures2.features.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		//createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		createInfo.pNext = &deviceFeatures2; // mine

		// might not really be necessary anymore because device specific validation layer have been deprecated
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device_) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create logical device!");
		}

		//vkCmdSetFragmentShadingRateKHR = (PFN_vkCmdSetFragmentShadingRateKHR)vkGetDeviceProcAddr(device_, "vkCmdSetFragmentShadingRateKHR");

		// Load vkCmdSetFragmentShadingRateKHR function
		// PFN_vkCmdSetFragmentShadingRateKHR vkCmdSetFragmentShadingRateKHR =
		// 	(PFN_vkCmdSetFragmentShadingRateKHR)vkGetDeviceProcAddr(device_, "vkCmdSetFragmentShadingRateKHR");

		// if (!vkCmdSetFragmentShadingRateKHR)
		// {
		// 	throw std::runtime_error("Failed to load vkCmdSetFragmentShadingRateKHR");
		// }

		vkGetDeviceQueue(device_, indices.graphicsFamily, 0, &graphicsQueue_);
		vkGetDeviceQueue(device_, indices.presentFamily, 0, &presentQueue_);


	}

	void RengbDevice::createCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
		poolInfo.flags =
			VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create command pool!");
		}
	}

	void RengbDevice::createSurface() { window.createWindowSurface(instance, &surface_); }

	bool RengbDevice::isDeviceSuitable(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices = findQueueFamilies(device);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return indices.isComplete() && extensionsSupported && swapChainAdequate &&
			   supportedFeatures.samplerAnisotropy;
	}

	void RengbDevice::populateDebugMessengerCreateInfo(
		VkDebugUtilsMessengerCreateInfoEXT &createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
									 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
								 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
								 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr; // Optional
	}

	void RengbDevice::setupDebugMessenger()
	{
		if (!enableValidationLayers)
			return;
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);
		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	bool RengbDevice::checkValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char *layerName : validationLayers)
		{
			bool layerFound = false;

			for (const auto &layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	std::vector<const char *> RengbDevice::getRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char **glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		//extensions.push_back(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
		//extensions.push_back(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME); // add fdm

		return extensions;
	}

	void RengbDevice::hasGflwRequiredInstanceExtensions()
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		std::cout << "available extensions:" << std::endl;
		std::unordered_set<std::string> available;
		for (const auto &extension : extensions)
		{
			std::cout << "\t" << extension.extensionName << std::endl;
			available.insert(extension.extensionName);
		}

		std::cout << "required extensions:" << std::endl;
		auto requiredExtensions = getRequiredExtensions();
		for (const auto &required : requiredExtensions)
		{
			std::cout << "\t" << required << std::endl;
			if (available.find(required) == available.end())
			{
				throw std::runtime_error("Missing required glfw extension");
			}
		}
	}

	bool RengbDevice::checkDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(
			device,
			nullptr,
			&extensionCount,
			availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto &extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	QueueFamilyIndices RengbDevice::findQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto &queueFamily : queueFamilies)
		{
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
				indices.graphicsFamilyHasValue = true;
			}
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
			if (queueFamily.queueCount > 0 && presentSupport)
			{
				indices.presentFamily = i;
				indices.presentFamilyHasValue = true;
			}
			if (indices.isComplete())
			{
				break;
			}

			++i;
		}

		return indices;
	}

	SwapChainSupportDetails RengbDevice::querySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(
				device,
				surface_,
				&presentModeCount,
				details.presentModes.data());
		}
		return details;
	}

	VkFormat RengbDevice::findSupportedFormat(
		const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (
				tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}
		throw std::runtime_error("failed to find supported format!");
	}

	uint32_t RengbDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
		{
			if ((typeFilter & (1 << i)) &&
				(memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	void RengbDevice::createBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer &buffer,
		VkDeviceMemory &bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create vertex buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device_, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device_, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}

		vkBindBufferMemory(device_, buffer, bufferMemory, 0);
	}

	VkCommandBuffer RengbDevice::beginSingleTimeCommands()
	{
		
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		return commandBuffer;
	}

	void RengbDevice::endSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue_);

		vkFreeCommandBuffers(device_, commandPool, 1, &commandBuffer);
	}

	void RengbDevice::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		endSingleTimeCommands(commandBuffer);
	}

	void RengbDevice::copyBufferToImage(
		VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = layerCount;

		region.imageOffset = {0, 0, 0};
		region.imageExtent = {width, height, 1};

		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region);
		endSingleTimeCommands(commandBuffer);
	}

	void RengbDevice::createImageWithInfo(
		const VkImageCreateInfo &imageInfo,
		VkMemoryPropertyFlags properties,
		VkImage &image,
		VkDeviceMemory &imageMemory)
	{
		if (vkCreateImage(device_, &imageInfo, nullptr, &image) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device_, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device_, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate image memory!");
		}

		if (vkBindImageMemory(device_, image, imageMemory, 0) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to bind image memory!");
		}
	}

	// void RengbDevice::rEngXRCreateInstance()
	// {
	// 	static const char* const applicationName = "OpenXR Example";
	// 	static const unsigned int majorVersion = 0;
	// 	static const unsigned int minorVersion = 1;
	// 	static const unsigned int patchVersion = 0;
	// 	static const char* const extensionNames[] = {
	// 		"XR_KHR_vulkan_enable",
	// 		"XR_KHR_vulkan_enable2"
	// 	};

	// 	XrInstanceCreateInfo instanceCreateInfo{};
	// 	instanceCreateInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
	// 	instanceCreateInfo.createFlags = 0;
	// 	strcpy(instanceCreateInfo.applicationInfo.applicationName, applicationName);
	// 	// instanceCreateInfo.applicationInfo.applicationVersion = XR_MAKE_VERSION(majorVersion, minorVersion, patchVersion);
	// 	strcpy(instanceCreateInfo.applicationInfo.engineName, applicationName);
	// 	// instanceCreateInfo.applicationInfo.engineVersion = XR_MAKE_VERSION(majorVersion, minorVersion, patchVersion);
	// 	instanceCreateInfo.applicationInfo.apiVersion = XR_MAKE_VERSION(1, 0, 34);
	// 	instanceCreateInfo.enabledApiLayerCount = 0;
	// 	instanceCreateInfo.enabledApiLayerNames = nullptr;
	// 	instanceCreateInfo.enabledExtensionCount = sizeof(extensionNames) / sizeof(const char*);
	// 	instanceCreateInfo.enabledExtensionNames = extensionNames;

	// 	XrResult result = xrCreateInstance(&instanceCreateInfo, &rEngXRInstance);

	// 	if (result != XR_SUCCESS)
	// 	{
	// 		std::cerr << "Failed to create OpenXR instance: " << result << std::endl;
	// 		// return XR_NULL_HANDLE;
	// 	}
	// }

	// void RengbDevice::rEngXRSetSystem()
	// {
	// 	XrSystemGetInfo systemGetInfo{};
	// 	systemGetInfo.type = XR_TYPE_SYSTEM_GET_INFO;
	// 	systemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

	// 	XrResult result = xrGetSystem(rEngXRInstance, &systemGetInfo, &systemID);

	// 	if (result != XR_SUCCESS)
	// 	{
	// 		std::cerr << "Failed to get system: " << result << std::endl;
	// 	}
	// }

	// void RengbDevice::rEngXRSetVulkanInstanceRequirements()
	// {
	// 	auto xrGetVulkanGraphicsRequirementsKHR = (PFN_xrGetVulkanGraphicsRequirementsKHR)getXRFunction("xrGetVulkanGraphicsRequirementsKHR");
	// 	auto xrGetVulkanInstanceExtensionsKHR = (PFN_xrGetVulkanInstanceExtensionsKHR)getXRFunction("xrGetVulkanInstanceExtensionsKHR");

	// 	//XrGraphicsRequirementsVulkanKHR graphicsRequirements{};
	// 	rEngXRGraphicsRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR;

	// 	XrResult result = xrGetVulkanGraphicsRequirementsKHR(rEngXRInstance, systemID, &rEngXRGraphicsRequirements);

	// 	if (result != XR_SUCCESS)
	// 	{
	// 		std::cerr << "Failed to get Vulkan graphics requirements: " << result << std::endl;
	// 		//return { rEngXRGraphicsRequirements, {} };
	// 	}

	// 	uint32_t instanceExtensionsSize;

	// 	result = xrGetVulkanInstanceExtensionsKHR(rEngXRInstance, systemID, 0, &instanceExtensionsSize, nullptr);

	// 	if (result != XR_SUCCESS)
	// 	{
	// 		std::cerr << "Failed to get Vulkan rEngXRInstance extensions: " << result << std::endl;
	// 		//return { rEngXRGraphicsRequirements, {} };
	// 	}

	// 	char* instanceExtensionsData = new char[instanceExtensionsSize];

	// 	result = xrGetVulkanInstanceExtensionsKHR(rEngXRInstance, systemID, instanceExtensionsSize, &instanceExtensionsSize, instanceExtensionsData);

	// 	if (result != XR_SUCCESS)
	// 	{
	// 		std::cerr << "Failed to get Vulkan rEngXRInstance extensions: " << result << std::endl;
	// 		//return { rEngXRGraphicsRequirements, {} };
	// 	}

	// 	//std::set<std::string> instanceExtensions;

	// 	uint32_t last = 0;
	// 	for (uint32_t i = 0; i <= instanceExtensionsSize; i++)
	// 	{
	// 		if (i == instanceExtensionsSize || instanceExtensionsData[i] == ' ')
	// 		{
	// 			rEngXRInstanceExtensions.insert(std::string(instanceExtensionsData + last, i - last));
	// 			last = i + 1;
	// 		}
	// 	}

	// 	delete[] instanceExtensionsData;

	// 	//return { rEngXRGraphicsRequirements, instanceExtensions };
	// }
	
	// void RengbDevice::rEngXRGetVulkanDeviceRequirements()
	// {
	// 	auto xrGetVulkanGraphicsDeviceKHR = (PFN_xrGetVulkanGraphicsDeviceKHR)getXRFunction("xrGetVulkanGraphicsDeviceKHR");
	// 	auto xrGetVulkanDeviceExtensionsKHR = (PFN_xrGetVulkanDeviceExtensionsKHR)getXRFunction("xrGetVulkanDeviceExtensionsKHR");

	// 	VkPhysicalDevice physicalDevice;

	// 	XrResult result = xrGetVulkanGraphicsDeviceKHR(rEngXRInstance, systemID, instance, &physicalDevice);

	// 	if (result != XR_SUCCESS)
	// 	{
	// 		std::cerr << "Failed to get Vulkan graphics device: " << result << std::endl;
	// 	}

	// 	uint32_t deviceExtensionsSize;

	// 	result = xrGetVulkanDeviceExtensionsKHR(rEngXRInstance, systemID, 0, &deviceExtensionsSize, nullptr);

	// 	if (result != XR_SUCCESS)
	// 	{
	// 		std::cerr << "Failed to get Vulkan device extensions: " << result << std::endl;
	// 	}

	// 	char* deviceExtensionsData = new char[deviceExtensionsSize];

	// 	result = xrGetVulkanDeviceExtensionsKHR(rEngXRInstance, systemID, deviceExtensionsSize, &deviceExtensionsSize, deviceExtensionsData);

	// 	if (result != XR_SUCCESS)
	// 	{
	// 		std::cerr << "Failed to get Vulkan device extensions: " << result << std::endl;
	// 	}


	// 	uint32_t last = 0;
	// 	for (uint32_t i = 0; i <= deviceExtensionsSize; i++)
	// 	{
	// 		if (i == deviceExtensionsSize || deviceExtensionsData[i] == ' ')
	// 		{
	// 			rEngXRDeviceExtensions.insert(std::string(deviceExtensionsData + last, i - last));
	// 			last = i + 1;
	// 		}
	// 	}

	// 	delete[] deviceExtensionsData;

	// 	//return { physicalDevice, deviceExtensions };
	// }

	// void RengbDevice::rEngXRDestroyInstance()
	// {
	// 	xrDestroyInstance(rEngXRInstance);
	// }

} // namespace rengb