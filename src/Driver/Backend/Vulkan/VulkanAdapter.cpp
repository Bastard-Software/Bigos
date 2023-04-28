#define BGS_USE_VULKAN_HANDLES 1

#include "VulkanAdapter.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {

            RESULT VulkanAdapter::Create( VkPhysicalDevice adapter, VulkanFactory* pFactory )
            {
                adapter;
                pFactory;

                return Results::OK;
            }

            void VulkanAdapter::Destroy()
            {
            }
        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS