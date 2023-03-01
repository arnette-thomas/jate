#include <jate/rendering/vulkan/vulkan_command_manager.h>

#include <spdlog/spdlog.h>

namespace jate::rendering::vulkan
{
    VulkanCommandManager::VulkanCommandManager(VulkanDevice& device, VulkanSwapChain& swapChain, uint8_t commandBuffersCount) :
        m_device(device),
        m_swapChain(swapChain)
    {
        init_createCommandPool();
        init_createCommandBuffers(commandBuffersCount);
    }

    VulkanCommandManager::~VulkanCommandManager()
    {
        vkDestroyCommandPool(m_device.getVkDevice(), m_commandPool, nullptr);   // This will destroy command buffers as well
    }

    void VulkanCommandManager::init_createCommandPool()
    {
        auto queueFamilyIndices = m_device.getQueueFamilyIndices();

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

        poolInfo.flags = 
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |   // Buffers can be re-recorder individually, without having to reset them together
            VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;               // Indicates that buffer are ofter re-recorded (this is our case since command buffers are recorded each frame)

        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsQueueFamily.value();

        if (vkCreateCommandPool(m_device.getVkDevice(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
            spdlog::error("Failed to create command pool");
            return;
        }
    }

    void VulkanCommandManager::init_createCommandBuffers(uint8_t amount)
    {
        m_commandBuffers.resize(amount);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;      // Primary = can be sumbitted ; Secondary = can be called by another command buffer
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

        if (vkAllocateCommandBuffers(m_device.getVkDevice(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
            spdlog::error("Failed to allocate command buffers!");
            return;
        }
    }

    void VulkanCommandManager::changeCommandBufferIndex(size_t index)
    {
        if (index >= m_commandBuffers.size())
        {
            spdlog::error("bad index at changeCommandBufferIndex : index = {}, but command manager only has {} buffers", index, m_commandBuffers.size());
            return;
        }
        m_currentCommandBufferIndex = index;
    }

    void VulkanCommandManager::startRecording()
    {
        vkResetCommandBuffer(getCurrentCommandBuffer(), 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(getCurrentCommandBuffer(), &beginInfo) != VK_SUCCESS) {
            spdlog::error("failed to begin recording command buffer!");
            return;
        }
    }

    void VulkanCommandManager::endRecording()
    {
        if (vkEndCommandBuffer(getCurrentCommandBuffer()) != VK_SUCCESS) {
            spdlog::error("failed to record command buffer!");
            return;
        }
    }

    void VulkanCommandManager::cmdStartRenderPass(VkRenderPass renderPass, uint32_t frameBufferIndex)
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = m_swapChain.getFrameBuffer(frameBufferIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapChain.getExtent();

        // TODO this should be a parameter or smth
        std::array<VkClearValue, 2> clearValues;
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(getCurrentCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VulkanCommandManager::cmdEndRenderPass()
    {
        vkCmdEndRenderPass(getCurrentCommandBuffer());
    }

    void VulkanCommandManager::cmdBindPipeline(const VulkanPipeline& pipeline)
    {
        vkCmdBindPipeline(getCurrentCommandBuffer(), VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getVkPipeline());
    }

    void VulkanCommandManager::cmdSetViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
    {
        VkViewport viewport {};
        viewport.x = x; viewport.y = y;
        viewport.width = width; viewport.height = height;
        viewport.minDepth = minDepth; viewport.maxDepth = maxDepth;

        vkCmdSetViewport(getCurrentCommandBuffer(), 0, 1, &viewport);
    }

    void VulkanCommandManager::cmdSetScissor(VkOffset2D offset, VkExtent2D extent)
    {
        VkRect2D scissor {};
        scissor.offset = offset;
        scissor.extent = extent;

        vkCmdSetScissor(getCurrentCommandBuffer(), 0, 1, &scissor);
    }

    void VulkanCommandManager::cmdDrawVertexBuffer(const VulkanVertexBuffer& vertexBuffer)
    {

        VkBuffer buffers[] = { vertexBuffer.getVkBuffer() };
		VkDeviceSize bufferOffsets[] = { vertexBuffer.getBufferOffset() };
		vkCmdBindVertexBuffers(getCurrentCommandBuffer(), 0, 1, buffers, bufferOffsets);

        vkCmdDraw(getCurrentCommandBuffer(), vertexBuffer.getVertexCount(), 1, 0, 0);
    }

    void VulkanCommandManager::submit(VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence fence)
    {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {waitSemaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffers[m_currentCommandBufferIndex];

        VkSemaphore signalSemaphores[] = {signalSemaphore};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(m_device.getGraphicsQueue(), 1, &submitInfo, fence) != VK_SUCCESS) {
            spdlog::error("failed to submit draw command buffer!");
            return;
        }
    }

    void VulkanCommandManager::present(uint32_t frameBufferIndex, VkSemaphore waitSemaphore)
    {
        VkSemaphore waitSemaphores[] = {waitSemaphore};

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = waitSemaphores;

        VkSwapchainKHR swapChains[] = {m_swapChain.getVkSwapchain()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &frameBufferIndex;

        presentInfo.pResults = nullptr; // Optional

        vkQueuePresentKHR(m_device.getPresentQueue(), &presentInfo);
    }
}