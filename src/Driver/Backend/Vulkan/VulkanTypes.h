#pragma once
#include "Driver/Backend/APIHandles.h"

#include <volk.h>

#if BGS_USE_VULKAN_HANDLES
#    define BGS_DECLARE_VULKAN_HANDLE( name, type )                                                                                                  \
        template<>                                                                                                                                   \
        struct BIGOS::Core::HandleTraits<BIGOS::Driver::Backend::Private::BGS_HANDLE_TAG_NAME( name )>                                               \
        {                                                                                                                                            \
            using NativeHandleTypes = type;                                                                                                          \
        };

BGS_DECLARE_VULKAN_HANDLE( Factory, VkInstance );
BGS_DECLARE_VULKAN_HANDLE( Adapter, VkPhysicalDevice );
BGS_DECLARE_VULKAN_HANDLE( Device, VkDevice );

#endif

#include "Driver/Backend/APITypes.h"

namespace BIGOS::Driver::Backend
{
    class VulkanAdapter;
    class VulkanFactory;
    class VulkanDevice;
} // namespace BIGOS::Driver::Backend