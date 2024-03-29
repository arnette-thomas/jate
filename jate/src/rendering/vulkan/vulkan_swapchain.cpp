#include <jate/rendering/vulkan/vulkan_swapchain.h>
#include <jate/rendering/vulkan/exceptions.h>

#include <spdlog/spdlog.h>
#include <algorithm>
#include <limits>

namespace jate::rendering::vulkan
{
    VulkanSwapChain::VulkanSwapChain(Window& window, VulkanDevice& device, std::unique_ptr<VulkanSwapChain> previousSwapChain)
        : m_window(window), m_device(device)
    {
        if (previousSwapChain != nullptr)
        {
            m_oldSwapChain = std::move(previousSwapChain);
        }

        // This is called first to ensure swap chain support is available during the initialization process
        m_swapChainSupport = getPhysicalDeviceSwapChainSupport(m_device.getPhysicalDevice(), m_window.getVulkanSurface());

        init_chooseSurfaceFormat();
        init_choosePresentationMode();
        init_chooseSwapExtent();

        init_createSwapChain();
        init_createImageViews();
        init_createDepthResources();
        init_createRenderPass();

        init_createFrameBuffers();

        m_oldSwapChain = nullptr;   // Release old swap chain, since it is only useful at initialization
    }

    VulkanSwapChain::~VulkanSwapChain()
    {
        for (auto framebuffer : m_frameBuffers) {
            vkDestroyFramebuffer(m_device.getVkDevice(), framebuffer, nullptr);
        }

        for (auto imageView : m_swapChainImageViews) {
            vkDestroyImageView(m_device.getVkDevice(), imageView, nullptr);
        }

        for (int i = 0; i < m_depthImages.size(); i++)
        {
            vkDestroyImageView(m_device.getVkDevice(), m_depthImageViews[i], nullptr);
            vkDestroyImage(m_device.getVkDevice(), m_depthImages[i], nullptr);
            vkFreeMemory(m_device.getVkDevice(), m_depthImageMemorys[i], nullptr);
        }

        vkDestroyRenderPass(m_device.getVkDevice(), m_renderPass, nullptr);

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

        createInfo.oldSwapchain = m_oldSwapChain != nullptr ? m_oldSwapChain->getVkSwapchain() : VK_NULL_HANDLE;   // Allows swap chain to keep in memory the previous swap chain, useful when it is rebuilt (e.g. window resize)

        if (vkCreateSwapchainKHR(m_device.getVkDevice(), &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
            spdlog::error("Failed to create swap chain");
            return;
        }

        // Retrieve swap chain images
        vkGetSwapchainImagesKHR(m_device.getVkDevice(), m_swapChain, &imageCount, nullptr);
        m_swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_device.getVkDevice(), m_swapChain, &imageCount, m_swapChainImages.data());
    }

    void VulkanSwapChain::init_createImageViews()
    {
        m_swapChainImageViews.resize(getImageCount());

        for (size_t i = 0; i < m_swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_swapChainImages[i];

            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_surfaceFormat.format;

            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

            // No mipmapping
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;

            // Only one layer
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(m_device.getVkDevice(), &createInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS) {
                spdlog::error("Failed to create image view of index {}", i);
                return;
            }
        }
    }

    void VulkanSwapChain::init_createRenderPass()
    {
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = m_surfaceFormat.format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.srcAccessMask = 0;
        dependency.srcStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstSubpass = 0;
        dependency.dstStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask =
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(m_device.getVkDevice(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
        {
            spdlog::error("Failed to create render pass!");
            return;
        }
    }

    void VulkanSwapChain::init_createDepthResources()
    {
        VkFormat depthFormat = findDepthFormat();
        VkExtent2D swapChainExtent = m_swapExtent;

        m_depthImages.resize(getImageCount());
        m_depthImageMemorys.resize(getImageCount());
        m_depthImageViews.resize(getImageCount());

        for (int i = 0; i < m_depthImages.size(); i++)
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = swapChainExtent.width;
            imageInfo.extent.height = swapChainExtent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = depthFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.flags = 0;

            createImageWithInfo(
                imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_depthImages[i],
                m_depthImageMemorys[i]);

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = m_depthImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = depthFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(m_device.getVkDevice(), &viewInfo, nullptr, &m_depthImageViews[i]) != VK_SUCCESS)
            {
                spdlog::error("failed to create texture image view for depth resources!");
                return;
            }
        }
    }

    void VulkanSwapChain::init_createFrameBuffers()
    {
        m_frameBuffers.resize(getImageCount());

        for (size_t i = 0; i < getImageCount(); i++)
        {
            uint32_t attachmentsCount = 2;
            VkImageView attachments[] = {
                m_swapChainImageViews[i],
                m_depthImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_renderPass;
            framebufferInfo.attachmentCount = attachmentsCount;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = m_swapExtent.width;
            framebufferInfo.height = m_swapExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(m_device.getVkDevice(), &framebufferInfo, nullptr, &m_frameBuffers[i]) != VK_SUCCESS) {
                spdlog::error("Failed to create frame buffer at index {}.", i);
                return;
            }
        }
    }

    VkFormat VulkanSwapChain::findDepthFormat() const
    {
        return m_device.findSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    void VulkanSwapChain::createImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory) const
    {
        if (vkCreateImage(m_device.getVkDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_device.getVkDevice(), image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = m_device.findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_device.getVkDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate image memory!");
        }

        if (vkBindImageMemory(m_device.getVkDevice(), image, imageMemory, 0) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to bind image memory!");
        }
    }

    uint32_t VulkanSwapChain::acquireNextImage(VkSemaphore signalSemaphore)
    {
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_device.getVkDevice(), m_swapChain, UINT64_MAX, signalSemaphore, VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.hasBeenResized())
            throw SwapChainOutOfDateException("Swap chain out of date");
        else if (result != VK_SUCCESS)
            throw std::runtime_error("Could not acquire swap chain image.");

        return imageIndex;
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