#include <jate/vulkan/vulkan_device.h>

#include <spdlog/spdlog.h>
#include <algorithm>

#include <cassert>

namespace jate::vulkan
{
    VulkanDevice::VulkanDevice(const VulkanInstance& instance) : m_instance(instance)
    {
        init_pickPhysicalDevice();
        if (m_physicalDevice != nullptr)
        {
            init_createLogicalDevice();
        }
    }

    VulkanDevice::~VulkanDevice()
    {
        vkDestroyDevice(m_device, nullptr);
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
        QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

        // Device queue
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.graphicsQueueFamily.value();
        queueCreateInfo.queueCount = 1;
        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        // Device features
        VkPhysicalDeviceFeatures deviceFeatures{};

        // Creating the logical device itself
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pEnabledFeatures = &deviceFeatures;

        // Device validation layers are not relevant for modern Vulkan implementations

        if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
            spdlog::error("Error while creating logical device.");
            return;
        }

        // Retrieve queues
        vkGetDeviceQueue(m_device, indices.graphicsQueueFamily.value(), 0, &m_graphicsQueue);
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

        // We need a graphics queue
        if (!queueFamilies.isComplete())
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

        for (size_t i = 0; i < queueFamilyCount; i++)
        {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.graphicsQueueFamily = static_cast<uint32_t>(i);
        }

        return indices;
    }
}