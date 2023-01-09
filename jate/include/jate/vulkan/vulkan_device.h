#ifndef Jate_VulkanDevice_H
#define Jate_VulkanDevice_H

#include <jate/vulkan/vulkan_instance.h>
#include <jate/window/window.h>

#include <optional>

namespace jate::vulkan
{
    class VulkanDevice
    {
    public:
        VulkanDevice(const VulkanInstance& instance, Window& window);
        ~VulkanDevice();

        // No copy allowed
        VulkanDevice(const VulkanDevice&) = delete;
        VulkanDevice& operator=(const VulkanDevice&) = delete;

    private:
        // Init functions
        void init_pickPhysicalDevice();
        void init_createLogicalDevice();

        struct QueueFamilyIndices
        {
            std::optional<uint32_t> graphicsQueueFamily;
            std::optional<uint32_t> presentQueueFamily;

            bool isComplete() const { return graphicsQueueFamily.has_value() && presentQueueFamily.has_value(); }
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
        Window& m_window;

        VkPhysicalDevice m_physicalDevice;

        VkDevice m_device;

        // Queues
        VkQueue m_graphicsQueue;
        VkQueue m_presentQueue;
    };
}

#endif