#ifndef Jate_VulkanSwapchain_H
#define Jate_VulkanSwapchain_H

#include <jate/window/window.h>
#include <jate/vulkan/vulkan_device.h>

namespace jate::vulkan
{
    class VulkanSwapChain
    {
    public:
        VulkanSwapChain(Window& window, VulkanDevice& device);
        ~VulkanSwapChain();

        // Disable copy
        VulkanSwapChain(const VulkanSwapChain&) = delete;
        VulkanSwapChain& operator=(const VulkanSwapChain&) = delete;

        struct SwapChainSupportDetails {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        inline SwapChainSupportDetails getSwapChainSupport() const { return m_swapChainSupport; }


        static SwapChainSupportDetails getPhysicalDeviceSwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR windowSurface);

    private:
        // Init method
        void init_chooseSurfaceFormat();
        void init_choosePresentationMode();
        void init_chooseSwapExtent();

        //   must be called after the various "choose" methods
        void init_createSwapChain();
        void init_createImageViews();

        Window& m_window;
        VulkanDevice& m_device;

        SwapChainSupportDetails m_swapChainSupport;

        VkSurfaceFormatKHR m_surfaceFormat;
        VkPresentModeKHR m_presentMode;
        VkExtent2D m_swapExtent;

        VkSwapchainKHR m_swapChain;

        std::vector<VkImage> m_swapChainImages;

        std::vector<VkImageView> m_swapChainImageViews;
    };
}

#endif