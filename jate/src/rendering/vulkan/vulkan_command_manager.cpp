#include <jate/rendering/vulkan/vulkan_command_manager.h>

#include <spdlog/spdlog.h>

namespace jate::rendering::vulkan
{
    VulkanCommandManager::VulkanCommandManager(VulkanDevice& device, VulkanSwapChain& swapChain, uint8_t commandBuffersCount) :
        m_device(device),
        m_swapChain(swapChain)
    {
        init_createCommandPools();
        init_createCommandBuffers(commandBuffersCount);
    }

    VulkanCommandManager::~VulkanCommandManager()
    {
        m_mainCommandBuffers.clear();
        vkDestroyCommandPool(m_device.getVkDevice(), m_mainCommandPool, nullptr);   // This will destroy command buffers as well
        vkDestroyCommandPool(m_device.getVkDevice(), m_oneShotCommandPool, nullptr);
    }

    void VulkanCommandManager::init_createCommandPools()
    {
        auto queueFamilyIndices = m_device.getQueueFamilyIndices();

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

        poolInfo.flags = 
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |   // Buffers can be re-recorded individually, without having to reset them together
            VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;               // Indicates that buffer are ofter re-recorded (this is our case since command buffers are recorded each frame)

        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsQueueFamily.value();

        if (vkCreateCommandPool(m_device.getVkDevice(), &poolInfo, nullptr, &m_mainCommandPool) != VK_SUCCESS) {
            spdlog::error("Failed to create main command pool");
            return;
        }

        if (vkCreateCommandPool(m_device.getVkDevice(), &poolInfo, nullptr, &m_oneShotCommandPool) != VK_SUCCESS) {
            spdlog::error("Failed to create one-shot command pool");
            return;
        }
    }

    void VulkanCommandManager::init_createCommandBuffers(uint8_t amount)
    {
        std::vector<VkCommandBuffer> cmdBuffers(amount);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_mainCommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;      // Primary = can be sumbitted ; Secondary = can be called by another command buffer
        allocInfo.commandBufferCount = static_cast<uint32_t>(cmdBuffers.size());

        if (vkAllocateCommandBuffers(m_device.getVkDevice(), &allocInfo, cmdBuffers.data()) != VK_SUCCESS) {
            spdlog::error("Failed to allocate command buffers!");
            return;
        }

        m_mainCommandBuffers.reserve(amount);
        for (auto cmdBuffer : cmdBuffers)
        {
            m_mainCommandBuffers.emplace_back(m_device, cmdBuffer, VulkanCommandBuffer::Usage::Main);
        }
    }

    VulkanCommandBuffer* VulkanCommandManager::getMainCommandBuffer(size_t index)
    {
        if (index >= m_mainCommandBuffers.size())
        {
            spdlog::error("bad index at changeCommandBufferIndex : index = {}, but command manager only has {} buffers", index, m_mainCommandBuffers.size());
            return nullptr; // TODO
        }
        return &m_mainCommandBuffers[index];
    }

    VulkanCommandBuffer VulkanCommandManager::createOneShotCommandBuffer()
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_oneShotCommandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(m_device.getVkDevice(), &allocInfo, &commandBuffer);

        return VulkanCommandBuffer(m_device, commandBuffer, VulkanCommandBuffer::Usage::OneShot, [this, commandBuffer]()
        {
            vkFreeCommandBuffers(this->m_device.getVkDevice(), this->m_oneShotCommandPool, 1, &commandBuffer);
        });
    }

    VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice& device, VkCommandBuffer commandBuffer, Usage commandBufferUsage, std::function<void()> onDelete)
        : m_device(device), m_commandBuffer(commandBuffer), m_onDeleteFn(onDelete)
    {
        m_commandBufferUsage = commandBufferUsage;
    }

    VulkanCommandBuffer::~VulkanCommandBuffer()
    {
        if (m_onDeleteFn != nullptr)
        {
            m_onDeleteFn();
        }
    }

    void VulkanCommandBuffer::startRecording()
    {
        vkResetCommandBuffer(m_commandBuffer, 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;
        if (m_commandBufferUsage == Usage::OneShot)
        {
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        }

        if (vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS) {
            spdlog::error("failed to begin recording command buffer!");
            return;
        }
    }

    void VulkanCommandBuffer::endRecording()
    {
        if (vkEndCommandBuffer(m_commandBuffer) != VK_SUCCESS) {
            spdlog::error("failed to record command buffer!");
            return;
        }
    }

    void VulkanCommandBuffer::cmdStartRenderPass(const VulkanSwapChain& swapChain, uint32_t frameBufferIndex)
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = swapChain.getRenderPass();
        renderPassInfo.framebuffer = swapChain.getFrameBuffer(frameBufferIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChain.getExtent();

        // TODO this should be a parameter or smth
        std::array<VkClearValue, 2> clearValues;
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VulkanCommandBuffer::cmdEndRenderPass()
    {
        vkCmdEndRenderPass(m_commandBuffer);
    }

    void VulkanCommandBuffer::cmdBindPipeline(const VulkanPipeline& pipeline)
    {
        vkCmdBindPipeline(m_commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getVkPipeline());
    }

    void VulkanCommandBuffer::cmdSetViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
    {
        VkViewport viewport {};
        viewport.x = x; viewport.y = y;
        viewport.width = width; viewport.height = height;
        viewport.minDepth = minDepth; viewport.maxDepth = maxDepth;

        vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);
    }

    void VulkanCommandBuffer::cmdSetScissor(VkOffset2D offset, VkExtent2D extent)
    {
        VkRect2D scissor {};
        scissor.offset = offset;
        scissor.extent = extent;

        vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
    }

    void VulkanCommandBuffer::cmdDrawVertexBuffer(const VulkanVertexBuffer& vertexBuffer)
    {

        VkBuffer buffers[] = { vertexBuffer.getVkBuffer() };
		VkDeviceSize bufferOffsets[] = { vertexBuffer.getBufferOffset() };
		vkCmdBindVertexBuffers(m_commandBuffer, 0, 1, buffers, bufferOffsets);

        vkCmdDraw(m_commandBuffer, vertexBuffer.getVertexCount(), 1, 0, 0);
    }

    void VulkanCommandBuffer::cmdCopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        VkBufferCopy copyRegion {};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;

        vkCmdCopyBuffer(m_commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    }

    void VulkanCommandBuffer::submit(VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence fence)
    {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {waitSemaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = waitSemaphore != nullptr ? 1 : 0;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffer;

        VkSemaphore signalSemaphores[] = {signalSemaphore};
        submitInfo.signalSemaphoreCount = signalSemaphore != nullptr ? 1 : 0;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(m_device.getGraphicsQueue(), 1, &submitInfo, fence) != VK_SUCCESS) {
            spdlog::error("failed to submit draw command buffer!");
            return;
        }
    }

    void VulkanCommandBuffer::present(const VulkanSwapChain& swapChain, uint32_t* frameBufferIndex, VkSemaphore waitSemaphore)
    {
        VkSemaphore waitSemaphores[] = {waitSemaphore};

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = waitSemaphore != nullptr ? 1 : 0;
        presentInfo.pWaitSemaphores = waitSemaphores;

        VkSwapchainKHR swapChains[] = {swapChain.getVkSwapchain()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = frameBufferIndex;

        presentInfo.pResults = nullptr; // Optional

        vkQueuePresentKHR(m_device.getPresentQueue(), &presentInfo);
    }
}