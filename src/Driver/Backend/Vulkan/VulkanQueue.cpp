#define BGS_USE_VULKAN_HANDLES 1

#include "VulkanQueue.h"

#include "VulkanDevice.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {

            RESULT VulkanQueue::Submit( const QueueSubmitDesc& desc )
            {
                desc;

                return Results::OK;
            }

            RESULT VulkanQueue::Wait( const QueueWaitDesc& desc )
            {
                desc;

                return Results::OK;
            }

            RESULT VulkanQueue::Create( const QueueDesc& desc, VulkanDevice* pDevice )
            {
                BGS_ASSERT( pDevice != nullptr, "Device (pDevice) must be a valid pointer." );
                if( pDevice == nullptr )
                {
                    return Results::FAIL;
                }

                m_pParent = pDevice;
                m_desc    = desc;

                if( BGS_FAILED( m_pParent->FindSuitableQueue( m_desc.type, m_desc.priority, &m_nativeQueueFamilyIndex, &m_nativeQueueIndex ) ) )
                {
                    return Results::NOT_FOUND;
                }

                VkDevice nativeDevice = m_pParent->GetHandle().GetNativeHandle();
                VkQueue  nativeQueue  = VK_NULL_HANDLE;
                vkGetDeviceQueue( nativeDevice, m_nativeQueueFamilyIndex, m_nativeQueueIndex, &nativeQueue );

                return Results::OK;
            }

            void VulkanQueue::Destroy()
            {
                BGS_ASSERT( m_pParent != nullptr );

                if( ( m_handle != QueueHandle() ) )
                {
                    m_pParent->FreeNativeQueue( m_nativeQueueFamilyIndex, m_nativeQueueIndex );
                    m_nativeQueueFamilyIndex = MAX_UINT32; // Equivalent of INVALID_POSITION for 32 bit ints.
                    m_nativeQueueIndex       = MAX_UINT32; // Equivalent of INVALID_POSITION for 32 bit ints.
                }

                m_pParent = nullptr;
                m_handle  = QueueHandle();
            }

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS