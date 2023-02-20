#include <jate/application.h>

namespace jate
{
    Application::Application() 
        : m_window("My window", 800, 600), 
          m_vulkanInstance("My app"),
          m_vulkanDevice(m_vulkanInstance, m_window)
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