#pragma once

#include "VulkanTypes.h"

namespace BIGOS::Driver::Backend
{
    struct VulkanSampler
    {
        VkSampler    sampler;
        void*        pDescriptorData;
        SAMPLER_TYPE type;
    };
} // namespace BIGOS::Driver::Backend