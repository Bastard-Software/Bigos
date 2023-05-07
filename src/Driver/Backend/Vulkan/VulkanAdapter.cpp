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
                BGS_ASSERT( adapter != VK_NULL_HANDLE, "Adapter (adapter) must be a valid pointer." );
                BGS_ASSERT( pFactory != nullptr, "Factory (pFactory) must be a valid pointer." );

                m_handle  = AdapterHandle( adapter );
                m_pParent = pFactory;

                return Results::OK;
            }

            void VulkanAdapter::Destroy()
            {
                BGS_ASSERT( m_handle != AdapterHandle() );
                m_handle  = AdapterHandle();
                m_pParent = nullptr;
            }

            RESULT VulkanAdapter::GetInternalInfo()
            {
                BGS_ASSERT( m_handle != AdapterHandle() );
                VkPhysicalDevice nativeAdapter = m_handle.GetNativeHandle();
                BGS_ASSERT( nativeAdapter != VK_NULL_HANDLE );

                return Results::OK;
            }

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS