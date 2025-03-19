#pragma once

#include "VulkanTypes.h"

namespace BIGOS::Driver::Backend
{
    struct VulkanResource
    {
        VkExtent3D extent;
        union
        {
            VkImage  image;
            VkBuffer buffer;
        };
        VulkanMemory*        pMemory;
        VkDeviceSize         memoryOffset;
        ResourceUsageFlags   usage;
        VULKAN_RESOURCE_TYPE type;
    };
} // namespace BIGOS::Driver::Backend