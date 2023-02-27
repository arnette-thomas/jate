#ifndef Jate_VulkanRenderer_H
#define Jate_VulkanRenderer_H

#include <jate/rendering/renderer.h>

#include <jate/rendering/vulkan/vulkan_instance.h>
#include <jate/rendering/vulkan/vulkan_device.h>
#include <jate/rendering/vulkan/vulkan_swapchain.h>
#include <jate/rendering/vulkan/vulkan_pipeline.h>

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

        VulkanInstance m_vulkanInstance;
        VulkanDevice m_vulkanDevice;
        VulkanSwapChain m_vulkanSwapChain;

        std::unique_ptr<VulkanPipeline> m_vulkanPipeline;
        VkPipelineLayout m_pipelineLayout;
    };
}

#endif