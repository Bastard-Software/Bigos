#pragma once
#include "Driver/Backend/APIHandles.h"

#include <volk.h>

namespace BIGOS::Driver::Backend
{
    struct VulkanBindingHeap;
    struct VulkanMemory;
    struct VulkanResource;
    struct VulkanResourceView;
    struct VulkanSampler;
} // namespace BIGOS::Driver::Backend

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
BGS_DECLARE_VULKAN_HANDLE( Swapchain, VkSwapchainKHR );
BGS_DECLARE_VULKAN_HANDLE( CommandPool, VkCommandPool );
BGS_DECLARE_VULKAN_HANDLE( CommandBuffer, VkCommandBuffer );
BGS_DECLARE_VULKAN_HANDLE( Shader, VkShaderModule );
BGS_DECLARE_VULKAN_HANDLE( PipelineLayout, VkPipelineLayout );
BGS_DECLARE_VULKAN_HANDLE( Pipeline, VkPipeline );
BGS_DECLARE_VULKAN_HANDLE( Fence, VkSemaphore );
BGS_DECLARE_VULKAN_HANDLE( Semaphore, VkSemaphore );
BGS_DECLARE_VULKAN_HANDLE( Memory, BIGOS::Driver::Backend::VulkanMemory* );
BGS_DECLARE_VULKAN_HANDLE( Resource, BIGOS::Driver::Backend::VulkanResource* );
BGS_DECLARE_VULKAN_HANDLE( ResourceView, BIGOS::Driver::Backend::VulkanResourceView* );
BGS_DECLARE_VULKAN_HANDLE( Sampler, BIGOS::Driver::Backend::VulkanSampler* );
BGS_DECLARE_VULKAN_HANDLE( BindingSetLayout, VkDescriptorSetLayout );
BGS_DECLARE_VULKAN_HANDLE( BindingHeap, BIGOS::Driver::Backend::VulkanBindingHeap* );
BGS_DECLARE_VULKAN_HANDLE( QueryPool, VkQueryPool );

#endif

#include "Driver/Backend/APITypes.h"

namespace BIGOS::Driver::Backend
{
    class VulkanAdapter;
    class VulkanFactory;
    class VulkanDevice;

    enum class VulkanResourceTypes : uint8_t
    {
        BUFFER,
        IMAGE,
        _MAX_ENUM,
    };
    using VULKAN_RESOURCE_TYPE = VulkanResourceTypes;

    struct VulkanDescriptorInfo
    {
        void*  pData;
        size_t size;
    };

} // namespace BIGOS::Driver::Backend