#pragma once
#include "VulkanTypes.h"

namespace BIGOS::Driver::Backend
{
    struct VulkanBindingHeap
    {
        void*              pHost;
        VkDeviceMemory     memory;
        VkBuffer           buffer;
        VkDeviceAddress    address;
        VkBufferUsageFlags flags;
    };
} // namespace BIGOS::Driver::Backend