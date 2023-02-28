#ifndef Jate_VulkanRenderer_H
#define Jate_VulkanRenderer_H

#include <jate/rendering/renderer.h>

#include <jate/rendering/vulkan/vulkan_instance.h>
#include <jate/rendering/vulkan/vulkan_device.h>
#include <jate/rendering/vulkan/vulkan_swapchain.h>
#include <jate/rendering/vulkan/vulkan_pipeline.h>
#include <jate/rendering/vulkan/vulkan_command_manager.h>

#include <memory>

namespace jate::rendering::vulkan
{
    class VulkanRenderer : public ARenderer
    {
    public:
        VulkanRenderer(Window& window);
        ~VulkanRenderer();

    private:
        void init_createPipelineLayout();
        void init_createPipeline();
        void init_createSyncObjects();

        virtual void beginFrame() override;
        virtual void endFrame() override;

        VulkanInstance m_vulkanInstance;
        VulkanDevice m_vulkanDevice;
        VulkanSwapChain m_vulkanSwapChain;
        VulkanCommandManager m_vulkanCommandManager;

        std::unique_ptr<VulkanPipeline> m_vulkanPipeline;
        VkPipelineLayout m_pipelineLayout;

        // Sync objects
        VkSemaphore m_imageAvailableSemaphore;
        VkSemaphore m_renderFinishedSemaphore;
        VkFence m_inFlightFence;

        uint32_t m_currentImageIndex;

        // TEMPORARY
        std::unique_ptr<VulkanVertexBuffer> m_testingVertexBuffer;
    };
}

#endif