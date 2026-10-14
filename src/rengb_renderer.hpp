#pragma once

#include "rengb_device.hpp"
#include "rengb_swap_chain.hpp"
#include "rengb_window.hpp"

// std
#include <cassert>
#include <memory>
#include <vector>

namespace rengb
{
	class RengbRenderer
	{
	public:
		RengbRenderer(RengbWindow &window, RengbDevice &device);
		~RengbRenderer();

		RengbRenderer(const RengbRenderer &) = delete;
		RengbRenderer &operator=(const RengbRenderer &) = delete;

		VkRenderPass getSwapChainRenderPass() const { return rengbSwapChain->getRenderPass(); }
		float getAspectRatio() const { return rengbSwapChain->extentAspectRatio(); }
		bool isFrameInProgress() const { return isFrameStarted; }

		// for fencing
		VkFence getInFlightFence(size_t index) const { return rengbSwapChain->getInFlightFence(index); }


		VkCommandBuffer getCurrentCommandBuffer() const
		{
			assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
			return commandBuffers[currentFrameIndex];
		}

		int getFrameIndex() const
		{
			assert(isFrameStarted && "Cannot get frame index when frame not in progress");
			return currentFrameIndex;
		}

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		void setFSR(VkExtent2D fragmentSize);

	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

		RengbWindow &rengbWindow;
		RengbDevice &rengbDevice;
		std::unique_ptr<RengbSwapChain> rengbSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex{0};
		bool isFrameStarted{false};
	};
} // namespace rengb