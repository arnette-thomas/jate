#ifndef Jate_Application_H
#define Jate_Application_H

#include <jate/window/window.h>
#include <jate/vulkan/vulkan_instance.h>
#include <jate/vulkan/vulkan_device.h>

namespace jate
{
    class Application
    {
    public:
        Application();
        ~Application(){};

        void run();
    
    private:
        bool m_running = false;
        Window m_window;
        vulkan::VulkanInstance m_vulkanInstance;
        vulkan::VulkanDevice m_vulkanDevice;
    };
}

#endif