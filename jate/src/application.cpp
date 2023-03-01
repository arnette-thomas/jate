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
        while (!m_window.shouldClose())
        {
            glfwPollEvents();
            m_renderer->beginFrame();
            m_renderer->endFrame();
        }
    }
}