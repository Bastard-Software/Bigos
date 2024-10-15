#pragma once
#include "VulkanTypes.h"

namespace BIGOS::Driver::Backend
{
    struct VulkanCommandLayout
    {
        uint32_t              stride;
        INDIRECT_COMMAND_TYPE type;
    };
} // namespace BIGOS::Driver::Backend