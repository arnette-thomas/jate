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
        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence> m_inFlightFences;

        uint32_t m_currentImageIndex;

        const uint8_t MAX_FRAMES_IN_FLIGHT = 2;
        uint8_t m_currentFrameInFlight = 0;

        // TEMPORARY
        std::unique_ptr<VulkanVertexBuffer> m_testingVertexBuffer;
    };
}

#endif