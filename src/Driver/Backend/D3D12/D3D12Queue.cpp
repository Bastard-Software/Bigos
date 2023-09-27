#define BGS_USE_D3D12_HANDLES 1

#include "D3D12Queue.h"

#include "D3D12Common.h"
#include "D3D12Device.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {

            RESULT D3D12Queue::Submit( const QueueSubmitDesc& desc )
            {
                desc;

                return Results::OK;
            }

            RESULT D3D12Queue::Wait( const QueueWaitDesc& desc )
            {
                desc;

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