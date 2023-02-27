#ifndef Jate_Application_H
#define Jate_Application_H

#include <jate/window/window.h>
#include <jate/vulkan/vulkan_instance.h>
#include <jate/vulkan/vulkan_device.h>
#include <jate/vulkan/vulkan_swapchain.h>
#include <jate/vulkan/vulkan_pipeline.h>

#include <memory>

namespace jate
{
    class Application
    {
    public:
        Application();
        ~Application(){};

        void run();
    
    private:
        void init_createPipelineLayout();
        void init_createPipeline();

        bool m_running = false;
        Window m_window;
        vulkan::VulkanInstance m_vulkanInstance;
        vulkan::VulkanDevice m_vulkanDevice;
        vulkan::VulkanSwapChain m_vulkanSwapChain;

        std::unique_ptr<vulkan::VulkanPipeline> m_vulkanPipeline;
        VkPipelineLayout m_pipelineLayout;
    };
}

#endif