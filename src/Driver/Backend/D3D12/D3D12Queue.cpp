#define BGS_USE_D3D12_HANDLES 1

#include "D3D12Queue.h"

#include "D3D12Common.h"
#include "D3D12Device.h"
#include "D3D12Fence.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {

            RESULT D3D12Queue::Submit( const QueueSubmitDesc& desc )
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

                ID3D12CommandQueue* pNativeQueue = m_handle.GetNativeHandle();
                ID3D12Fence*        pNativeFence = nullptr;

                // Wait for binary semaphores
                for( index_t ndx = 0; ndx < static_cast<index_t>( desc.waitSemaphoreCount ); ++ndx )
                {
                    pNativeFence = desc.phWaitSemaphores[ ndx ].GetNativeHandle();
                    if( FAILED( pNativeQueue->Wait( pNativeFence, static_cast<uint64_t>( D3D12SemaphoreStates::SIGNALED ) ) ) )
                    {
                        return Results::FAIL;
                    }
                }
                // Wait for fences
                for( index_t ndx = 0; ndx < static_cast<index_t>( desc.waitFenceCount ); ++ndx )
                {
                    pNativeFence = desc.phWaitFences[ ndx ].GetNativeHandle()->pFence;
                    BGS_ASSERT( pNativeFence->GetCompletedValue() < desc.pWaitValues[ ndx ] );
                    if( FAILED( pNativeQueue->Wait( pNativeFence, static_cast<uint64_t>( D3D12SemaphoreStates::SIGNALED ) ) ) )
                    {
                        return Results::FAIL;
                    }
                }

                // Executing command lists
                ID3D12CommandList* pNativeLists[ Config::Driver::Queue::MAX_COMMAND_BUFFER_TO_EXECUTE_COUNT ];
                for( index_t ndx = 0; ndx < static_cast<index_t>( desc.commandBufferCount ); ++ndx )
                {
                    pNativeLists[ ndx ] = desc.ppCommandBuffers[ ndx ]->GetHandle().GetNativeHandle();
                }
                pNativeQueue->ExecuteCommandLists( desc.commandBufferCount, pNativeLists );

                // Reseting wait semaphores
                for( index_t ndx = 0; ndx < static_cast<index_t>( desc.waitSemaphoreCount ); ++ndx )
                {
                    pNativeFence = desc.phWaitSemaphores[ ndx ].GetNativeHandle();
                    pNativeQueue->Signal( pNativeFence, static_cast<uint64_t>( D3D12SemaphoreStates::NOT_SIGNALED ) );
                }
                // Setting binary semaphores to signaled
                for( index_t ndx = 0; ndx < static_cast<index_t>( desc.signalSemaphoreCount ); ++ndx )
                {
                    pNativeFence = desc.phSignalSemaphores[ ndx ].GetNativeHandle();
                    pNativeQueue->Signal( pNativeFence, static_cast<uint64_t>( D3D12SemaphoreStates::SIGNALED ) );
                }
                // Setting fences to proper values
                for( index_t ndx = 0; ndx < static_cast<index_t>( desc.signalFenceCount ); ++ndx )
                {
                    pNativeFence = desc.phSignalFences[ ndx ].GetNativeHandle()->pFence;
                    pNativeQueue->Signal( pNativeFence, desc.pSignalValues[ ndx ] );
                }

                return Results::OK;
            }

            RESULT D3D12Queue::Create( const QueueDesc& desc, D3D12Device* pDevice )
            {
                BGS_ASSERT( pDevice != nullptr, "Device (pDevice) must be a valid pointer." );
                if( pDevice == nullptr )
                {
                    return Results::FAIL;
                }

                m_pParent = pDevice;
                m_desc    = desc;

                D3D12_COMMAND_QUEUE_DESC nativeDesc;
                nativeDesc.Type     = MapBigosQueueTypeToD3D12CommandListType( m_desc.type );
                nativeDesc.Priority = MapBigosQueuePriorityToD3D12CommandQueuePriority( m_desc.priority );
                nativeDesc.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE;
                nativeDesc.NodeMask = 0;

                ID3D12Device*       pNativeDevice = m_pParent->GetHandle().GetNativeHandle();
                ID3D12CommandQueue* pNativeQueue  = nullptr;

                if( FAILED( pNativeDevice->CreateCommandQueue( &nativeDesc, IID_PPV_ARGS( &pNativeQueue ) ) ) )
                {
                    return Results::FAIL;
                }

                m_handle = QueueHandle( pNativeQueue );

                if( BGS_FAILED( QueryQueueLimits() ) )
                {
                    RELEASE_COM_PTR( pNativeQueue );
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void D3D12Queue::Destroy()
            {
                if( m_handle != QueueHandle() )
                {
                    ID3D12CommandQueue* pNativeQueue = m_handle.GetNativeHandle();
                    RELEASE_COM_PTR( pNativeQueue );
                }

                m_pParent = nullptr;
                m_handle  = QueueHandle();
            }

            RESULT D3D12Queue::QueryQueueLimits()
            {
                ID3D12CommandQueue* pNativeQueue = m_handle.GetNativeHandle();
                uint64_t            frequency    = 0;
                if( FAILED( pNativeQueue->GetTimestampFrequency( &frequency ) ) )
                {
                    return Results::FAIL;
                }
                // Dividing 1 000 000 000 t get period in nanoseconds
                m_limits.timestampPeriod = 1000000000.0f / static_cast<float>( frequency );

                return Results::OK;
            }

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS