#include <jate/application.h>

#include <jate/rendering/vulkan/vulkan_renderer.h>
#include <jate/systems/render_system.h>

#include <spdlog/spdlog.h>

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

    models::World* Application::createWorld()
    {
        m_world = std::make_unique<models::World>(*this, m_renderer.get());
        return m_world.get();
    }

    void Application::run()
    {
        if (m_world == nullptr)
        {
            spdlog::error("Cannot run app : world is null");
            return;
        }

        m_running = true;

        // Main loop
        while (!m_window.shouldClose())
        {
            glfwPollEvents();
            m_renderer->beginFrame();

            // Tick systems
            m_world->tickSystems();

            m_renderer->endFrame();
        }
    }
}