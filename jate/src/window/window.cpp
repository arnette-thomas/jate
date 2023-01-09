#include <jate/window/window.h>

#include <spdlog/spdlog.h>

namespace jate
{
    Window::Window(const std::string& name, uint16_t width, uint16_t height)
        : m_name(name), m_width(width), m_height(height)
    {
        init_createWindow();
    }

    void Window::init_createWindow()
    {
        if (!glfwInit())
        {
            spdlog::critical("Could not initialize GLFW.");
            return;
        }

        // Window Hints
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        m_glfwWindow = glfwCreateWindow(m_width, m_height, m_name.c_str(), nullptr, nullptr);
        if (m_glfwWindow == nullptr)
        {
            spdlog::critical("Could not create a GLFW window.");
            return;
        }

        // Set callbacks
        glfwSetWindowUserPointer(m_glfwWindow, this);
		glfwSetFramebufferSizeCallback(m_glfwWindow, frameBufferResizedCallback);
    }

    void Window::createWindowSurface()
    {
        assert(m_vkInstance != nullptr && "Window::createWindowSurface should not be called before Window::attachVulkanInstance");

        if (glfwCreateWindowSurface(m_vkInstance, m_glfwWindow, nullptr, &m_vkSurface) != VK_SUCCESS) {
            spdlog::error("Failed to create window surface!");
        }
    }

    void Window::freeWindowSurface()
    {
        vkDestroySurfaceKHR(m_vkInstance, m_vkSurface, nullptr);
    }

    Window::~Window()
    {
        glfwDestroyWindow(m_glfwWindow);
        glfwTerminate();
    }

    void Window::frameBufferResizedCallback(GLFWwindow* glfwWindow, int width, int height)
    {
        Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
		window->m_frameBufferResized = true;
		window->m_width = width;
		window->m_height = height;
	}
}