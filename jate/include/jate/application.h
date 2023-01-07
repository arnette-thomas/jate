#include <jate/window/window.h>
#include <jate/vulkan/vulkan_instance.h>

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
    };
}