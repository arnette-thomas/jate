#include <string>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace jate::vulkan
{
    class VulkanInstance
    {
    public:
        VulkanInstance(const std::string& appName);
        ~VulkanInstance();

        // Disable copy
        VulkanInstance(const VulkanInstance&) = delete;
        void operator=(const VulkanInstance&) = delete;

    private:
        void setupValidationLayerDebugMessenger();
        void populateValidationLayerDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const;

        std::vector<const char*> getExtensions() const;
        bool checkValidationLayerSupport() const;

        static VKAPI_ATTR VkBool32 VKAPI_CALL validationLayerDebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);
        
        static VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
        static void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

        VkInstance m_instance;
        VkDebugUtilsMessengerEXT m_debugMessenger;

        const std::vector<const char*> m_validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

        #ifdef NDEBUG
        const bool m_enableValidationLayers = false;
        #else
        const bool m_enableValidationLayers = true;
        #endif

        static const bool ms_ENABLE_VERBOSE_DEBUG_MESSAGES = false;
    };
}