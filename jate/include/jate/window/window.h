#ifndef Jate_Window_H
#define Jate_Window_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <jate/rendering/vulkan/vulkan_instance.h>

#include <string>

namespace jate
{
    class Window
    {
    public:
        Window(const std::string& name, uint16_t width, uint16_t height);
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        bool shouldClose() const { return glfwWindowShouldClose(m_glfwWindow); }
        VkSurfaceKHR getVulkanSurface() const { return m_vkSurface; }
        GLFWwindow* getWindowPtr() const { return m_glfwWindow; }

        void attachVulkanInstance(const rendering::vulkan::VulkanInstance& vulkanInstance) { m_vkInstance = vulkanInstance.getVkInstance(); }
        void createWindowSurface();
        void freeWindowSurface();

    private:
        uint16_t m_width, m_height;
        std::string m_name;
        bool m_frameBufferResized = false;

        GLFWwindow* m_glfwWindow;

        VkInstance m_vkInstance;
        VkSurfaceKHR m_vkSurface;

        // Init functions
        void init_createWindow();

        static void frameBufferResizedCallback(GLFWwindow* glfwWindow, int width, int height);
    };
    
} // namespace jate

#endif