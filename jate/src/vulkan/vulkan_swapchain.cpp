#include <jate/vulkan/vulkan_swapchain.h>

#include <spdlog/spdlog.h>
#include <algorithm>
#include <limits>

namespace jate::vulkan
{
    VulkanSwapChain::VulkanSwapChain(Window& window, VulkanDevice& device)
        : m_window(window), m_device(device)
    {
        // This is called first to ensure swap chain support is available during the initialization process
        m_swapChainSupport = getPhysicalDeviceSwapChainSupport(m_device.getPhysicalDevice(), m_window.getVulkanSurface());

        init_chooseSurfaceFormat();
        init_choosePresentationMode();
        init_chooseSwapExtent();
    }

    VulkanSwapChain::~VulkanSwapChain()
    {
        vkDestroySwapchainKHR(m_device.getVkDevice(), m_swapChain, nullptr);
    }

    void VulkanSwapChain::init_chooseSurfaceFormat()
    {
        if (m_swapChainSupport.formats.empty())
        {
            spdlog::error("SwapChain cannot choose surface format : no available format for the current physical device");
            return;
        }

        for (const auto& availableFormat : m_swapChainSupport.formats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                m_surfaceFormat = availableFormat;
                return;
            }
        }

        m_surfaceFormat = m_swapChainSupport.formats[0];
    }

    void VulkanSwapChain::init_choosePresentationMode()
    {
        if (m_swapChainSupport.presentModes.empty())
        {
            spdlog::error("SwapChain cannot choose presentation mode : no available presentMode for the current physical device");
            return;
        }

        if (std::find(m_swapChainSupport.presentModes.begin(), m_swapChainSupport.presentModes.end(), VK_PRESENT_MODE_MAILBOX_KHR) != m_swapChainSupport.presentModes.end())
            m_presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        else
            m_presentMode = VK_PRESENT_MODE_FIFO_KHR;       // Guaranteed to be available
    }

    void VulkanSwapChain::init_chooseSwapExtent()
    {
        if (m_swapChainSupport.capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
        {
            m_swapExtent = m_swapChainSupport.capabilities.currentExtent;
        }
        else
        {
            int width, height;
            glfwGetFramebufferSize(m_window.getWindowPtr(), &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, m_swapChainSupport.capabilities.minImageExtent.width, m_swapChainSupport.capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, m_swapChainSupport.capabilities.minImageExtent.height, m_swapChainSupport.capabilities.maxImageExtent.height);

            m_swapExtent = actualExtent;
        }
    }

    // Must be called after the various "choose" methods
    void VulkanSwapChain::init_createSwapChain()
    {
        // Image count
        uint32_t imageCount = m_swapChainSupport.capabilities.minImageCount + 1;
        if (m_swapChainSupport.capabilities.maxImageCount > 0 && imageCount > m_swapChainSupport.capabilities.maxImageCount) {
            imageCount = m_swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_window.getVulkanSurface();
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = m_surfaceFormat.format;
        createInfo.imageColorSpace = m_surfaceFormat.colorSpace;
        createInfo.imageExtent = m_swapExtent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        VulkanDevice::QueueFamilyIndices deviceQueueFamilyIndices = m_device.getQueueFamilyIndices();
        uint32_t queueFamilyIndices[] = {deviceQueueFamilyIndices.graphicsQueueFamily.value(), deviceQueueFamilyIndices.presentQueueFamily.value()};

        if (deviceQueueFamilyIndices.graphicsQueueFamily != deviceQueueFamilyIndices.presentQueueFamily)
        {
            // If queue families are different, use concurent mode to avoid dealing with image ownership 
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = m_swapChainSupport.capabilities.currentTransform;

        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;  // Ignore alpha channel for the window (we don't carre about window transparency)

        createInfo.presentMode = m_presentMode;
        createInfo.clipped = VK_TRUE;   // The values of pixels hidden by another window are ignored.

        createInfo.oldSwapchain = VK_NULL_HANDLE;   // TEMPORARY. Allows swap chain to keep in memory the previous swap chain, useful when it is rebuilt (e.g. window resize)

        if (vkCreateSwapchainKHR(m_device.getVkDevice(), &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
            spdlog::error("Failed to create swap chain");
            return;
        }
    }

    // STATIC METHOD
    VulkanSwapChain::SwapChainSupportDetails VulkanSwapChain::getPhysicalDeviceSwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR windowSurface)
    {
        SwapChainSupportDetails details {};

        // Surface capabilities
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, windowSurface, &details.capabilities);

        // Surface formats
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, windowSurface, &formatCount, nullptr);
        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, windowSurface, &formatCount, details.formats.data());
        }

        // Presentation modes
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, windowSurface, &presentModeCount, nullptr);
        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, windowSurface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

}