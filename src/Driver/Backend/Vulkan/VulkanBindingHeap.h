#pragma once
#include "VulkanTypes.h"

namespace BIGOS::Driver::Backend
{
    struct VulkanBindingHeap
    {
        VkDeviceMemory     memory;
        VkBuffer           buffer;
        VkDeviceAddress    address;
        VkBufferUsageFlags flags;
    };
} // namespace BIGOS::Driver::Backend