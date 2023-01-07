#include <jate/application.h>

namespace jate
{
    Application::Application() : m_window("My window", 800, 600), m_vulkanInstance("My app")
    {

    }

    void Application::run()
    {
        m_running = true;

        // Main loop
        while (!m_window.shouldClose())
        {
            glfwPollEvents();
        }
    }
}