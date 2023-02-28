#include <jate/application.h>

#include <jate/rendering/vulkan/vulkan_renderer.h>

namespace jate
{
    Application::Application() 
        : m_window("My window", 800, 600)
    {
        m_renderer = std::make_unique<rendering::vulkan::VulkanRenderer>(m_window);
    }

    Application::~Application()
    {
    }


    void Application::run()
    {
        m_running = true;

        // Main loop
        // DEBUG - for now we are only drawing one frame, so to preserve CPU we don't enable the main loop until systems are implemented
        m_renderer->beginFrame();
        m_renderer->endFrame();
        while (!m_window.shouldClose())
        {
            glfwWaitEvents();
        }
    }
}