#pragma once

#include "VulkanTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {

            struct VulkanResourceView
            {
                union
                {
                    VkImageView     imageView;
                    BufferRangeDesc bufferView;
                };
                VULKAN_RESOURCE_TYPE type;
            };

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS