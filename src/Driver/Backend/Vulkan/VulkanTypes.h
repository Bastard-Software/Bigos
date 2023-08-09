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
BGS_DECLARE_VULKAN_HANDLE( Queue, VkQueue );
BGS_DECLARE_VULKAN_HANDLE( CommandPool, VkCommandPool );
BGS_DECLARE_VULKAN_HANDLE( CommandBuffer, VkCommandBuffer );
BGS_DECLARE_VULKAN_HANDLE( Shader, VkShaderModule );
BGS_DECLARE_VULKAN_HANDLE( PipelineLayout, VkPipelineLayout );
BGS_DECLARE_VULKAN_HANDLE( Pipeline, VkPipeline );
BGS_DECLARE_VULKAN_HANDLE( Fence, VkSemaphore );
BGS_DECLARE_VULKAN_HANDLE( Semaphore, VkSemaphore );

#endif

#include "Driver/Backend/APITypes.h"

namespace BIGOS::Driver::Backend
{
    class VulkanAdapter;
    class VulkanFactory;
    class VulkanDevice;

    struct VulkanQueueParams
    {
        uint32_t            familyIndex;
        uint32_t            queueIndex;
        QUEUE_PRIORITY_TYPE priority;
        bool_t              free;
    };

    struct VulkanQueueFamilyParams
    {
        HeapArray<VulkanQueueParams> queueParams;
        VkQueueFamilyProperties      familiyParams;
    };

    using VulkanQueueProperties = HeapArray<VulkanQueueFamilyParams>;

} // namespace BIGOS::Driver::Backend