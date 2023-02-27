#include <jate/application.h>

#include <jate/rendering/vulkan/vulkan_renderer.h>

namespace jate
{
    Application::Application() 
        : m_window("My window", 800, 600)
    {
        m_renderer = std::make_unique<rendering::vulkan::VulkanRenderer>();
    }

    Application::~Application()
    {
    }


    void Application::run()
    {
        m_running = true;

        // Main loop
        while (!m_window.shouldClose())
        {
            glfwWaitEvents();   // TODO use pollevents instead for main loop once we'll have actual frames, for now it used 100% CPU for nothing
        }
    }
}