#include "rengb_renderer.hpp"

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace rengb
{

	RengbRenderer::RengbRenderer(RengbWindow &window, RengbDevice &device)
		: rengbWindow{window}, rengbDevice{device}
	{
		recreateSwapChain();
		createCommandBuffers();
	}

	RengbRenderer::~RengbRenderer()
	{
		freeCommandBuffers();
	}

	void RengbRenderer::recreateSwapChain()
	{
		auto extent = rengbWindow.getExtent();
		while (extent.width == 0 || extent.height == 0)
		{
			extent = rengbWindow.getExtent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(rengbDevice.device());

		if (rengbSwapChain == nullptr)
		{
			rengbSwapChain = std::make_unique<RengbSwapChain>(rengbDevice, extent);
		}
		else
		{
			std::shared_ptr<RengbSwapChain> oldSwapChain = std::move(rengbSwapChain);
			rengbSwapChain = std::make_unique<RengbSwapChain>(rengbDevice, extent, oldSwapChain);

			if (!oldSwapChain->compareSwapFormats(*rengbSwapChain.get()))
			{
				throw std::runtime_error("Swap chain image(or depth) format has changed!");
			}
		}
	}

	void RengbRenderer::createCommandBuffers()
	{
		commandBuffers.resize(RengbSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = rengbDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(rengbDevice.device(), &allocInfo, commandBuffers.data()) !=
			VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void RengbRenderer::freeCommandBuffers()
	{
		vkFreeCommandBuffers(
			rengbDevice.device(),
			rengbDevice.getCommandPool(),
			static_cast<uint32_t>(commandBuffers.size()),
			commandBuffers.data());
		commandBuffers.clear();
	}

	void RengbRenderer::setFSR(VkExtent2D fragmentSize)
	{
		auto vkCmdSetFragmentShadingRateKHR =
			(PFN_vkCmdSetFragmentShadingRateKHR)vkGetDeviceProcAddr(rengbDevice.device(), "vkCmdSetFragmentShadingRateKHR");

		VkFragmentShadingRateCombinerOpKHR combinerOps[2] = {
			VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR, // keep the original pipeline shading rate
			VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR  // should replace with the specified shading rate
		};
		auto commandBuffer = getCurrentCommandBuffer();
		vkCmdSetFragmentShadingRateKHR(commandBuffer, &fragmentSize, combinerOps);
	}

	VkCommandBuffer RengbRenderer::beginFrame()
	{
		assert(!isFrameStarted && "Can't call beginFrame while already in progress");

		auto result = rengbSwapChain->acquireNextImage(&currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}


		return commandBuffer;
	}

	void RengbRenderer::endFrame()
	{
		assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
		auto commandBuffer = getCurrentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}

		auto result = rengbSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || rengbWindow.wasWindowResized())
		{
			rengbWindow.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % RengbSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void RengbRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
		assert(
			commandBuffer == getCurrentCommandBuffer() &&
			"Can't begin render pass on command buffer from a different frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = rengbSwapChain->getRenderPass();
		renderPassInfo.framebuffer = rengbSwapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = rengbSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = {.0f, .0f, .0f, .0f};
		clearValues[1].depthStencil = {1.0f, 0};
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		//-------------------fsr-------------------
		// VkExtent2D fragmentSize = {16, 16}; // Example fragment size
		// VkFragmentShadingRateCombinerOpKHR combinerOps[2] = {
		//   VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
		//   VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR
		//   };
		// vkCmdSetFragmentShadingRateKHR(commandBuffer, &fragmentSize, combinerOps);
		//-------------------fsr-------------------

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(rengbSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(rengbSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{{0, 0}, rengbSwapChain->getSwapChainExtent()};
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void RengbRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
		assert(
			commandBuffer == getCurrentCommandBuffer() &&
			"Can't end render pass on command buffer from a different frame");
		vkCmdEndRenderPass(commandBuffer);
	}

} // namespace rengb