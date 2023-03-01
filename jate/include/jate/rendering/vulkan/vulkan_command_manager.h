#ifndef Jate_VulkanCommandManager_H
#define Jate_VulkanCommandManager_H

#include <jate/rendering/vulkan/vulkan_device.h>
#include <jate/rendering/vulkan/vulkan_swapchain.h>
#include <jate/rendering/vulkan/vulkan_vertexbuffer.h>
#include <jate/rendering/vulkan/vulkan_pipeline.h>

namespace jate::rendering::vulkan
{
    class VulkanCommandManager
    {
    public:
        VulkanCommandManager(VulkanDevice& device, VulkanSwapChain& swapChain, uint8_t commandBuffersCount = 1);
        ~VulkanCommandManager();

        // No copy
        VulkanCommandManager(const VulkanCommandManager&) = delete;
        VulkanCommandManager& operator=(const VulkanCommandManager&) = delete;

        void changeCommandBufferIndex(size_t index);

        void startRecording();
        void endRecording();

        void cmdStartRenderPass(VkRenderPass renderPass, uint32_t frameBufferIndex);
        void cmdEndRenderPass();

        void cmdBindPipeline(const VulkanPipeline& pipeline);
        void cmdSetViewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f);
        void cmdSetScissor(VkOffset2D offset, VkExtent2D extent);
        
        void cmdDrawVertexBuffer(const VulkanVertexBuffer& vertexBuffer);

        void submit(VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence fence);
        void present(uint32_t frameBufferIndex, VkSemaphore waitSemaphore);

    private:
        // init functions
        void init_createCommandPool();
        void init_createCommandBuffers(uint8_t amount);

        inline VkCommandBuffer getCurrentCommandBuffer() const { return m_commandBuffers[m_currentCommandBufferIndex]; }

        VulkanDevice& m_device;
        VulkanSwapChain& m_swapChain;

        VkCommandPool m_commandPool;
        std::vector<VkCommandBuffer> m_commandBuffers;
        size_t m_currentCommandBufferIndex;
    };
}

#endif
