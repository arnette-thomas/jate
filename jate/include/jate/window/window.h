#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace jate
{
    class Window
    {
    public:
        Window(const std::string& name, uint16_t width, uint16_t height);
        ~Window();

        Window(const Window&) = delete;
        void operator=(const Window&) = delete;

        bool shouldClose() const { return glfwWindowShouldClose(m_glfwWindow); }

    private:
        uint16_t m_width, m_height;
        std::string m_name;
        bool m_frameBufferResized = false;

        GLFWwindow* m_glfwWindow;

        static void frameBufferResizedCallback(GLFWwindow* glfwWindow, int width, int height);
    };
    
} // namespace jate
