#ifndef Jate_VulkanDevice_H
#define Jate_VulkanDevice_H

#include <jate/vulkan/vulkan_instance.h>

#include <optional>

namespace jate::vulkan
{
    class VulkanDevice
    {
    public:
        VulkanDevice(const VulkanInstance& instance);
        ~VulkanDevice();

        // No copy allowed
        VulkanDevice(const VulkanDevice&) = delete;
        VulkanDevice& operator=(const VulkanDevice&) = delete;

    private:
        // Init functions
        void init_pickPhysicalDevice();

        struct QueueFamilyIndices
        {
            std::optional<uint32_t> graphicsQueueFamily;

            bool isComplete() const { return graphicsQueueFamily.has_value(); }
        };

        /// @brief Assigns a score to a physical device.
        ///        A negative score means the device is not suitable for our use.
        /// @param device The physical device to rate
        /// @return an integer
        int32_t ratePhysicalDevice(VkPhysicalDevice device) const;

        /// @brief Finds and picks queue families available for the given device
        /// @param device A physical device
        /// @return A struct giving, for each relevant queue family, the index of the queue for the physocal device.
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;

        const VulkanInstance& m_instance;
        VkPhysicalDevice m_physicalDevice;
    };
}

#endif