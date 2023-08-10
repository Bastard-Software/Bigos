#pragma once

#include "VulkanTypes.h"

namespace BIGOS::Driver::Backend
{
    struct VulkanMemory
    {
        VkDeviceMemory nativeMemory;
        void*          pHostMemory;
    };
} // namespace BIGOS::Driver::Backend