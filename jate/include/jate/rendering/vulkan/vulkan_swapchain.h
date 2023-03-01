#ifndef Jate_VulkanSwapchain_H
#define Jate_VulkanSwapchain_H

#include <jate/window/window.h>
#include <jate/rendering/vulkan/vulkan_device.h>

#include <spdlog/spdlog.h>
#include <memory>

namespace jate::rendering::vulkan
{
    class VulkanSwapChain
    {
    public:
        VulkanSwapChain(Window& window, VulkanDevice& device, std::unique_ptr<VulkanSwapChain> previousSwapChain = nullptr);
        ~VulkanSwapChain();

        // Disable copy
        VulkanSwapChain(const VulkanSwapChain&) = delete;
        VulkanSwapChain& operator=(const VulkanSwapChain&) = delete;

        struct SwapChainSupportDetails {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        inline VkSwapchainKHR getVkSwapchain() const { return m_swapChain; }
        inline SwapChainSupportDetails getSwapChainSupport() const { return m_swapChainSupport; }
        inline size_t getImageCount() const { return m_swapChainImages.size(); }
        inline VkRenderPass getRenderPass() const { return m_renderPass; }
        inline VkExtent2D getExtent() const { return m_swapExtent; }
        inline VkFramebuffer getFrameBuffer(uint32_t frameBufferIndex) const
        {
            if (frameBufferIndex >= m_frameBuffers.size())
            {
                spdlog::error("Invalid index, trying to access framebuffer {}, but swap chain only has {} framebuffers.", frameBufferIndex, m_frameBuffers.size());
                return nullptr;
            }

            return m_frameBuffers[frameBufferIndex];
        }

        uint32_t acquireNextImage(VkSemaphore signalSemaphore);

        static SwapChainSupportDetails getPhysicalDeviceSwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR windowSurface);

    private:
        // Init method
        void init_chooseSurfaceFormat();
        void init_choosePresentationMode();
        void init_chooseSwapExtent();

        //   must be called after the various "choose" methods
        void init_createSwapChain();
        void init_createImageViews();
        void init_createRenderPass();
        void init_createDepthResources();
        void init_createFrameBuffers();

        VkFormat findDepthFormat() const;
        void createImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory) const;

        Window& m_window;
        VulkanDevice& m_device;

        SwapChainSupportDetails m_swapChainSupport;

        VkSurfaceFormatKHR m_surfaceFormat;
        VkPresentModeKHR m_presentMode;
        VkExtent2D m_swapExtent;

        VkSwapchainKHR m_swapChain;

        std::vector<VkImage> m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;

        std::vector<VkImage> m_depthImages;
        std::vector<VkDeviceMemory> m_depthImageMemorys;
        std::vector<VkImageView> m_depthImageViews;

        VkRenderPass m_renderPass;

        std::vector<VkFramebuffer> m_frameBuffers;

        std::unique_ptr<VulkanSwapChain> m_oldSwapChain;    // Only available at initialisation
    };
}

#endif