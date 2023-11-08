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
                void*                pDescriptorData;
                VkImageView          imageView;
                VULKAN_RESOURCE_TYPE type;
            };

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS