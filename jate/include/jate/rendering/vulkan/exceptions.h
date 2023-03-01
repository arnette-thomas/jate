#ifndef Jate_VulkanExceptions_H
#define Jate_VulkanExceptions_H

#include <stdexcept>

namespace jate::rendering::vulkan
{
    class SwapChainOutOfDateException : public std::runtime_error
    {
        // Inherit all constructors
        using runtime_error::runtime_error;
    };
}

#endif