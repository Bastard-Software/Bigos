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
                BGS_ASSERT( desc.waitFenceCount <= Config::Driver::Synchronization::MAX_FENCES_TO_WAIT_COUNT,
                            "Wait fence count (desc.waitFenceCount) must be less or equal %d.",
                            Config::Driver::Synchronization::MAX_FENCES_TO_WAIT_COUNT );
                BGS_ASSERT( desc.waitSemaphoreCount <= Config::Driver::Synchronization::MAX_SEMAPHORES_TO_WAIT_COUNT,
                            "Wait semaphore count (desc.waitSemaphoreCount) must be less or equal %d.",
                            Config::Driver::Synchronization::MAX_SEMAPHORES_TO_WAIT_COUNT );
                BGS_ASSERT( desc.signalFenceCount <= Config::Driver::Synchronization::MAX_FENCES_TO_SIGNAL_COUNT,
                            "Signal fence count (desc.signalFenceCount) must be less or equal %d.",
                            Config::Driver::Synchronization::MAX_FENCES_TO_SIGNAL_COUNT );
                BGS_ASSERT( desc.signalSemaphoreCount <= Config::Driver::Synchronization::MAX_SEMAPHORES_TO_SIGNAL_COUNT,
                            "Signal semaphore count (desc.signalSemaphoreCount) must be less or equal %d.",
                            Config::Driver::Synchronization::MAX_SEMAPHORES_TO_SIGNAL_COUNT );
                BGS_ASSERT( desc.commandBufferCount <= Config::Driver::Queue::MAX_COMMAND_BUFFER_TO_EXECUTE_COUNT,
                            "Command buffer count must be less or equal %d.", Config::Driver::Queue::MAX_COMMAND_BUFFER_TO_EXECUTE_COUNT );
                BGS_ASSERT( ( desc.ppCommandBuffers != nullptr ) && ( desc.commandBufferCount > 0 ), "You specified no command buffer to execute." );

                static const VkPipelineStageFlags waitStages[ Config::Driver::Synchronization::MAX_FENCES_TO_WAIT_COUNT +
                                                              Config::Driver::Synchronization::MAX_SEMAPHORES_TO_WAIT_COUNT ] = {
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                };

                // Note that our fence is vulkan timeline semaphore. We pass them before binary semaphores.
                VkSemaphore     nativeWaitSemaphores[ Config::Driver::Synchronization::MAX_FENCES_TO_WAIT_COUNT +
                                                  Config::Driver::Synchronization::MAX_SEMAPHORES_TO_WAIT_COUNT ];
                VkSemaphore     nativeSignalSemaphores[ Config::Driver::Synchronization::MAX_FENCES_TO_SIGNAL_COUNT +
                                                    Config::Driver::Synchronization::MAX_SEMAPHORES_TO_SIGNAL_COUNT ];
                VkCommandBuffer nativeCommandBuffers[ Config::Driver::Queue::MAX_COMMAND_BUFFER_TO_EXECUTE_COUNT ];

                for( index_t ndx = 0; ndx < static_cast<index_t>( desc.waitFenceCount ); ++ndx )
                {
                    nativeWaitSemaphores[ ndx ] = desc.phWaitFences[ ndx ].GetNativeHandle();
                }
                for( index_t ndx = 0; ndx < static_cast<index_t>( desc.waitSemaphoreCount ); ++ndx )
                {
                    nativeWaitSemaphores[ ndx + static_cast<index_t>( desc.waitFenceCount ) ] = desc.phWaitSemaphores[ ndx ].GetNativeHandle();
                }

                for( index_t ndx = 0; ndx < static_cast<index_t>( desc.signalFenceCount ); ++ndx )
                {
                    nativeSignalSemaphores[ ndx ] = desc.phSignalFences[ ndx ].GetNativeHandle();
                }
                for( index_t ndx = 0; ndx < static_cast<index_t>( desc.signalSemaphoreCount ); ++ndx )
                {
                    nativeSignalSemaphores[ ndx + static_cast<index_t>( desc.signalFenceCount ) ] = desc.phSignalSemaphores[ ndx ].GetNativeHandle();
                }

                for( index_t ndx = 0; ndx < static_cast<index_t>( desc.commandBufferCount ); ++ndx )
                {
                    nativeCommandBuffers[ ndx ] = desc.ppCommandBuffers[ ndx ]->GetHandle().GetNativeHandle();
                }

                // Passing wait/signal values using timeline semaphore submit info
                VkTimelineSemaphoreSubmitInfo timelineInfo;
                timelineInfo.sType                     = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
                timelineInfo.pNext                     = nullptr;
                timelineInfo.pWaitSemaphoreValues      = desc.pWaitValues;
                timelineInfo.waitSemaphoreValueCount   = desc.waitFenceCount + desc.waitSemaphoreCount; // Wierd doc condition.
                timelineInfo.pSignalSemaphoreValues    = desc.pSignalValues;
                timelineInfo.signalSemaphoreValueCount = desc.signalFenceCount + desc.signalSemaphoreCount; // Wierd doc condition.

                VkSubmitInfo submitInfo;
                submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.pNext                = &timelineInfo;
                submitInfo.pCommandBuffers      = nativeCommandBuffers;
                submitInfo.commandBufferCount   = desc.commandBufferCount;
                submitInfo.pWaitDstStageMask    = waitStages;
                submitInfo.pWaitSemaphores      = nativeWaitSemaphores;
                submitInfo.waitSemaphoreCount   = desc.waitFenceCount + desc.waitSemaphoreCount;
                submitInfo.pSignalSemaphores    = nativeSignalSemaphores;
                submitInfo.signalSemaphoreCount = desc.signalFenceCount + desc.signalSemaphoreCount;

                if( vkQueueSubmit( m_handle.GetNativeHandle(), 1, &submitInfo, VK_NULL_HANDLE ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }

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
                m_handle = QueueHandle( nativeQueue );
                QueryQueueLimits();

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

            RESULT VulkanQueue::QueryQueueLimits()
            {
                // In D3D12 timestamp frequency is queried on queue level (not on physical device as in vulkan). Thats why we store this informations
                // in queue limits structure.
                VkPhysicalDevice           nativeAdapter = m_pParent->GetDesc().pAdapter->GetHandle().GetNativeHandle();
                VkPhysicalDeviceProperties deviceProps;
                vkGetPhysicalDeviceProperties( nativeAdapter, &deviceProps );

                m_limits.timestampPeriod = deviceProps.limits.timestampPeriod;

                return Results::OK;
            }

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS