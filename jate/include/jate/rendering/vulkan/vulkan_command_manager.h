#ifndef Jate_VulkanCommandManager_H
#define Jate_VulkanCommandManager_H

#include <jate/rendering/vulkan/vulkan_device.h>
#include <jate/rendering/vulkan/vulkan_swapchain.h>
#include <jate/rendering/vulkan/vulkan_buffers.h>
#include <jate/rendering/vulkan/vulkan_pipeline.h>

#include <functional>

namespace jate::rendering::vulkan
{
    class VulkanCommandBuffer
    {
    public:
        enum Usage
        {
            Main,
            OneShot
        };

        VulkanCommandBuffer(VulkanDevice& device, VkCommandBuffer commandBuffer, Usage commandBufferUsage, std::function<void ()> onDelete = nullptr);
        ~VulkanCommandBuffer();

        void startRecording();
        void endRecording();

        void cmdStartRenderPass(const VulkanSwapChain& swapChain, uint32_t frameBufferIndex);
        void cmdEndRenderPass();

        void cmdBindPipeline(const VulkanPipeline& pipeline);
        void cmdSetViewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f);
        void cmdSetScissor(VkOffset2D offset, VkExtent2D extent);
        
        void cmdDrawVertexBuffer(const VulkanVertexBuffer& vertexBuffer);
        void cmdDrawIndexedVertexBuffer(const VulkanVertexBuffer& vertexBuffer, const VulkanIndexBuffer& indexBuffer);

        void cmdCopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        void submit(VkSemaphore waitSemaphore = nullptr, VkSemaphore signalSemaphore = nullptr, VkFence fence = nullptr);
        void present(const VulkanSwapChain& swapChain, uint32_t* frameBufferIndex, VkSemaphore waitSemaphore = nullptr);

    private:
        VkCommandBuffer m_commandBuffer;

        std::function<void ()> m_onDeleteFn;

        VulkanDevice& m_device;

        Usage m_commandBufferUsage;
    };

    class VulkanCommandManager
    {
    public:
        VulkanCommandManager(VulkanDevice& device, VulkanSwapChain& swapChain, uint8_t commandBuffersCount = 1);
        ~VulkanCommandManager();

        // No copy
        VulkanCommandManager(const VulkanCommandManager&) = delete;
        VulkanCommandManager& operator=(const VulkanCommandManager&) = delete;

        VulkanCommandBuffer* getMainCommandBuffer(size_t index);
        VulkanCommandBuffer createOneShotCommandBuffer();

    private:
        // init functions
        void init_createCommandPools();
        void init_createCommandBuffers(uint8_t amount);

        VulkanDevice& m_device;
        VulkanSwapChain& m_swapChain;

        VkCommandPool m_mainCommandPool;
        std::vector<VulkanCommandBuffer> m_mainCommandBuffers;

        VkCommandPool m_oneShotCommandPool;
    };
}

#endif
