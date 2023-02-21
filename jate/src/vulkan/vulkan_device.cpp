#include <jate/vulkan/vulkan_device.h>

#include <jate/vulkan/vulkan_swapchain.h>

#include <spdlog/spdlog.h>
#include <algorithm>
#include <set>

#include <cassert>

namespace jate::vulkan
{
    VulkanDevice::VulkanDevice(const VulkanInstance& instance, Window& window) : m_instance(instance), m_window(window)
    {
        m_window.attachVulkanInstance(instance);
        m_window.createWindowSurface();

        init_pickPhysicalDevice();
        if (m_physicalDevice != nullptr)
        {
            init_createLogicalDevice();
        }
    }

    VulkanDevice::~VulkanDevice()
    {
        vkDestroyDevice(m_device, nullptr);

        m_window.freeWindowSurface();
    }

    // Init functions
    void VulkanDevice::init_pickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_instance.getVkInstance(), &deviceCount, nullptr);

        if (deviceCount == 0) {
            spdlog::error("No GPU found with Vulkan support.");
            return;
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_instance.getVkInstance(), &deviceCount, devices.data());
        assert(devices.size() == deviceCount && "vkEnumeratePhysicalDevices did not returned as many elements as expected");

        std::vector<std::pair<VkPhysicalDevice, int32_t>> deviceScores(deviceCount);
        std::transform(devices.begin(), devices.end(), deviceScores.begin(), [this](VkPhysicalDevice d)
        {
            return std::make_pair(d, this->ratePhysicalDevice(d));
        });

        std::pair<VkPhysicalDevice, int32_t> pickedDevice = *std::max_element(deviceScores.begin(), deviceScores.end(), [this](std::pair<VkPhysicalDevice, int32_t> a, std::pair<VkPhysicalDevice, int32_t> b)
        {
            return a.second < b.second;
        });

        if (pickedDevice.second < 0)
        {
            spdlog::error("No suitable GPU found.");
            return;
        }

        m_physicalDevice = pickedDevice.first;
    }

    void VulkanDevice::init_createLogicalDevice()
    {
        m_queueFamilyIndices = findQueueFamilies(m_physicalDevice);

        // Device queue
        std::set<uint32_t> uniqueQueueFamilies = {m_queueFamilyIndices.graphicsQueueFamily.value(), m_queueFamilyIndices.presentQueueFamily.value()};
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // Device features
        VkPhysicalDeviceFeatures deviceFeatures{};

        // Creating the logical device itself
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();

        // Device validation layers are not relevant for modern Vulkan implementations

        if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
            spdlog::error("Error while creating logical device.");
            return;
        }

        // Retrieve queues
        vkGetDeviceQueue(m_device, m_queueFamilyIndices.graphicsQueueFamily.value(), 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, m_queueFamilyIndices.presentQueueFamily.value(), 0, &m_presentQueue);
    }

    int32_t VulkanDevice::ratePhysicalDevice(VkPhysicalDevice device) const
    {
        if (device == nullptr) return -1;   // Just for safety

        int32_t score = 0;
        
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        QueueFamilyIndices queueFamilies = findQueueFamilies(device);

        // Check if required device extensions are supported
        if (!checkDeviceExtensionSupport(device))
            return -1;

        // We need a graphics queue
        if (!queueFamilies.isComplete())
            return -1;

        // Check swap chain support
        VulkanSwapChain::SwapChainSupportDetails swapChainSupportDetails = VulkanSwapChain::getPhysicalDeviceSwapChainSupport(device, m_window.getVulkanSurface());
        if (swapChainSupportDetails.formats.empty() || swapChainSupportDetails.presentModes.empty())
            return -1;

        // Favour dedicated graphics cards
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            score += 100000;

        // Maximum possible size of textures affects graphics quality
        score += deviceProperties.limits.maxImageDimension2D;

        spdlog::info("Valid GPU detected. Name : {} ; score : {}", deviceProperties.deviceName, score);

        return score;
    }

    VulkanDevice::QueueFamilyIndices VulkanDevice::findQueueFamilies(VkPhysicalDevice device) const
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        assert(queueFamilies.size() == queueFamilyCount && "vkGetPhysicalDeviceQueueFamilyProperties did not returned as many elements as expected");

        for (uint32_t i = 0; i < queueFamilyCount; i++)
        {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.graphicsQueueFamily = i;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_window.getVulkanSurface(), &presentSupport);
            if (presentSupport)
                indices.presentQueueFamily = i;
        }

        return indices;
    }

    bool VulkanDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) const
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }
}