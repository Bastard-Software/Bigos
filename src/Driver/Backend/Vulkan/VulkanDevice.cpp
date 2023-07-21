#define BGS_USE_VULKAN_HANDLES 1

#include "VulkanDevice.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            class VkDeviceFeaturesHelper final
            {
            public:
                VkDeviceFeaturesHelper( const DeviceDesc& desc )
                {
                    m_devDesc = desc;

                    // Vulkan 1.0 features (core)
                    m_devCoreFeatures.pipelineStatisticsQuery = VK_TRUE;
                    m_devCoreFeatures.samplerAnisotropy       = VK_TRUE;
                    m_devCoreFeatures.multiDrawIndirect       = VK_TRUE;

                    // Vulkan 1.1 features
                    m_dev11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
                    m_dev11Features.pNext = &m_dev12Features;

                    // Vulkan 1.2 features
                    m_dev12Features.sType             = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
                    m_dev12Features.pNext             = &m_dev13Features;
                    m_dev12Features.drawIndirectCount = VK_TRUE;
                    // m_dev12Features.samplerMirrorClampToEdge = VK_TRUE;
                    m_dev12Features.timelineSemaphore = VK_TRUE; // Crucial for synchronization

                    // Vulkan 1.3 features
                    m_dev13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
                    // m_dev13Features.pNext = next p chain
                    m_dev13Features.synchronization2 = VK_TRUE; // Crucial for synchronization

                    m_pNext = &m_dev11Features;

                    // Extensions
                    m_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
                    // TODO: Add logic
                }

                VkDeviceFeaturesHelper() = default;

                void*                     GetExtChain() const { return m_pNext; }
                VkPhysicalDeviceFeatures* GetDeviceFeatures() { return &m_devCoreFeatures; }
                const char* const*        GetExtNames() const { return m_extensions.data(); }
                uint32_t                  GetExtCount() const { return static_cast<uint32_t>( m_extensions.size() ); }

            private:
                DeviceDesc m_devDesc;

                VkPhysicalDeviceFeatures         m_devCoreFeatures{};
                VkPhysicalDeviceVulkan11Features m_dev11Features{};
                VkPhysicalDeviceVulkan12Features m_dev12Features{};
                VkPhysicalDeviceVulkan13Features m_dev13Features{};

                HeapArray<const char*> m_extensions;

                // TODO: pNext chain structs
                void* m_pNext = nullptr;
            };

            RESULT VulkanDevice::Create( const DeviceDesc& desc, VulkanFactory* pFactory )
            {
                BGS_ASSERT( pFactory != nullptr, "Factory (pFactory) must be a valid pointer." );
                BGS_ASSERT( desc.h_adapter != AdapterHandle(), "Adapter handle (h_adapter) must be a valid adapter handle." );

                m_desc    = desc;
                m_pParent = pFactory;

                return Results::OK;
            }

            void VulkanDevice::Destroy()
            {
                m_pParent = nullptr;
                m_handle  = DeviceHandle();
            }

            RESULT VulkanDevice::CreateVkDevice()
            {
                VkPhysicalDevice       nativeAdapter = m_desc.h_adapter.GetNativeHandle();
                VkDevice               nativeDevice  = VK_NULL_HANDLE;
                VkDeviceFeaturesHelper deviceFeaturesHelper( m_desc );

                VkDeviceCreateInfo devInfo;
                devInfo.pNext                   = deviceFeaturesHelper.GetExtChain();
                devInfo.enabledExtensionCount   = deviceFeaturesHelper.GetExtCount();
                devInfo.ppEnabledExtensionNames = deviceFeaturesHelper.GetExtNames();
                devInfo.pEnabledFeatures        = deviceFeaturesHelper.GetDeviceFeatures();

                if( vkCreateDevice( nativeAdapter, &devInfo, nullptr, &nativeDevice ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }
                BGS_ASSERT( m_pDeviceAPI == nullptr );
                volkLoadDeviceTable( m_pDeviceAPI, nativeDevice );
                BGS_ASSERT( m_pDeviceAPI != nullptr );
                if( m_pDeviceAPI == nullptr )
                {
                    vkDestroyDevice( nativeDevice, nullptr );
                    return Results::FAIL;
                }

                return Results::OK;
            }

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS