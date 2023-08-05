#define BGS_USE_D3D12_HANDLES 1

#include "D3D12Device.h"

#include "BigosFramework/Config.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Memory.h"
#include "D3D12CommandBuffer.h"
#include "D3D12Common.h"
#include "D3D12Factory.h"
#include "D3D12Fence.h"
#include "D3D12Queue.h"
#include "Driver/Frontend/RenderSystem.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            RESULT D3D12Device::CreateQueue( const QueueDesc& desc, IQueue** ppQueue )
            {
                BGS_ASSERT( ppQueue != nullptr, "Queue (ppQueue) must be a valid address." );
                BGS_ASSERT( *ppQueue == nullptr, "There is a pointer at the given address. Queue (*ppQueue) must be nullptr." );
                if( ( ppQueue == nullptr ) || ( *ppQueue != nullptr ) )
                {
                    return Results::FAIL;
                }

                D3D12Queue* pQueue = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( m_pParent->GetParentPtr()->GetDefaultAllocatorPtr(), &pQueue ) ) )
                {
                    return Results::NO_MEMORY;
                }
                BGS_ASSERT( pQueue != nullptr );

                if( BGS_FAILED( pQueue->Create( desc, this ) ) )
                {
                    Memory::FreeObject( m_pParent->GetParentPtr()->GetDefaultAllocatorPtr(), &pQueue );
                    return Results::FAIL;
                }

                ( *ppQueue ) = pQueue;

                return Results::OK;
            }

            void D3D12Device::DestroyQueue( IQueue** ppQueue )
            {
                BGS_ASSERT( ppQueue != nullptr, "Queue (ppQueue) must be a valid address." );
                BGS_ASSERT( *ppQueue != nullptr, "Queue (*ppQueue) must be a valid pointer." );
                if( ( ppQueue != nullptr ) && ( *ppQueue != nullptr ) )
                {
                    static_cast<D3D12Queue*>( *ppQueue )->Destroy();
                    Memory::FreeObject( m_pParent->GetParentPtr()->GetDefaultAllocatorPtr(), ppQueue );
                }
            }

            RESULT D3D12Device::CreateCommandPool( const CommandPoolDesc& desc, CommandPoolHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Command pool (pHandle) must be a valid address." );
                BGS_ASSERT( desc.pQueue != nullptr, "Queue (desc.pQueue) must be a valid pointer." );
                if( desc.pQueue == nullptr )
                {
                    return Results::FAIL;
                }

                ID3D12Device*           pNativeDevice      = m_handle.GetNativeHandle();
                ID3D12CommandAllocator* pNativeCommandPool = nullptr;

                if( FAILED( pNativeDevice->CreateCommandAllocator( MapBigosQueueTypeToD3D12CommandListType( desc.pQueue->GetDesc().type ),
                                                                   IID_PPV_ARGS( &pNativeCommandPool ) ) ) )
                {
                    return Results::FAIL;
                }

                *pHandle = CommandPoolHandle( pNativeCommandPool );

                return Results::OK;
            }

            void D3D12Device::DestroyCommandPool( CommandPoolHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Command pool handle (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != CommandPoolHandle(), "Command pool handle (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != CommandPoolHandle() ) )
                {
                    ID3D12CommandAllocator* pNativeCommandPool = pHandle->GetNativeHandle();

                    RELEASE_COM_PTR( pNativeCommandPool );

                    *pHandle = CommandPoolHandle();
                }
            }

            RESULT D3D12Device::ResetCommandPool( CommandPoolHandle handle )
            {
                BGS_ASSERT( handle != CommandPoolHandle(), "Command pool handle (handle) must be a valid handle." );
                if( handle == CommandPoolHandle() )
                {
                    return Results::FAIL;
                }

                ID3D12CommandAllocator* pNativeCommandPool = handle.GetNativeHandle();
                if( FAILED( pNativeCommandPool->Reset() ) )
                {
                    return Results::FAIL;
                }

                return Results::OK;
            }

            RESULT D3D12Device::CreateCommandBuffer( const CommandBufferDesc& desc, ICommandBuffer** ppCommandBuffer )
            {
                BGS_ASSERT( ppCommandBuffer != nullptr, "Command buffer (ppCommandBuffer) must be a valid address." );
                BGS_ASSERT( *ppCommandBuffer == nullptr,
                            "There is a pointer at the given address. Command buffer (*ppCommandBuffer) must be nullptr." );
                if( ( ppCommandBuffer == nullptr ) || ( *ppCommandBuffer != nullptr ) )
                {
                    return Results::FAIL;
                }

                D3D12CommandBuffer* pCommandBuffer = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( m_pParent->GetParentPtr()->GetDefaultAllocatorPtr(), &pCommandBuffer ) ) )
                {
                    return Results::NO_MEMORY;
                }
                BGS_ASSERT( pCommandBuffer != nullptr );

                if( BGS_FAILED( pCommandBuffer->Create( desc, this ) ) )
                {
                    Memory::FreeObject( m_pParent->GetParentPtr()->GetDefaultAllocatorPtr(), &pCommandBuffer );
                    return Results::FAIL;
                }

                *ppCommandBuffer = pCommandBuffer;

                return Results::OK;
            }

            void D3D12Device::DestroyCommandBuffer( ICommandBuffer** ppCommandBuffer )
            {
                BGS_ASSERT( ppCommandBuffer != nullptr, "Command buffer (ppCommandBuffer) must be a valid address." );
                BGS_ASSERT( *ppCommandBuffer != nullptr, "Command buffer (*ppCommandBuffer) must be a valid pointer." );
                if( ( ppCommandBuffer != nullptr ) && ( *ppCommandBuffer != nullptr ) )
                {
                    static_cast<D3D12CommandBuffer*>( *ppCommandBuffer )->Destroy();
                    Memory::FreeObject( m_pParent->GetParentPtr()->GetDefaultAllocatorPtr(), ppCommandBuffer );
                }
            }

            RESULT D3D12Device::CreateFence( const FenceDesc& desc, FenceHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Fence (pHandle) must be a valid address." );

                ID3D12Device* pNativeDevice = m_handle.GetNativeHandle();
                D3D12Fence*   pNativeFence  = nullptr;

                if( BGS_FAILED( Memory::AllocateObject( m_pParent->GetParentPtr()->GetDefaultAllocatorPtr(), &pNativeFence ) ) )
                {
                    return Results::NO_MEMORY;
                }

                /*
                Params:
                  - LPSECURITY_ATTRIBUTES - ptr to struct containing security description, if null there is default
                    initialization which meets our needs
                  - bool - indicate if event needs manual reset. In our case we do not need manual reset (autoreset
                    after trigger is fine)
                  - bool - initial state, false is not signaled
                  - LPCSTR - event name
                */
                pNativeFence->hEvent = CreateEvent( nullptr, FALSE, FALSE, nullptr );

                if( FAILED( pNativeDevice->CreateFence( desc.initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &pNativeFence->pFence ) ) ) )
                {
                    CloseHandle( pNativeFence->hEvent );
                    Memory::FreeObject( m_pParent->GetParentPtr()->GetDefaultAllocatorPtr(), &pNativeFence );
                    return Results::FAIL;
                }

                *pHandle = FenceHandle( pNativeFence );

                return Results::OK;
            }

            void D3D12Device::DestroyFence( FenceHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Fence (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != FenceHandle(), "Fence (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != FenceHandle() ) )
                {
                    D3D12Fence* pNativeFence = pHandle->GetNativeHandle();

                    CloseHandle( pNativeFence->hEvent );
                    RELEASE_COM_PTR( pNativeFence->pFence );
                    Memory::FreeObject( m_pParent->GetParentPtr()->GetDefaultAllocatorPtr(), &pNativeFence );

                    *pHandle = FenceHandle();
                }
            }

            RESULT D3D12Device::WaitForFences( const WaitForFencesDesc& desc, uint64_t timeout )
            {
                BGS_ASSERT( desc.pFences != nullptr, "Fence (desc.pFences) must be a valid pointer." );
                BGS_ASSERT( desc.pWaitValues != nullptr, "Uint array (desc.pWaitValues) must be a valid pointer." );
                BGS_ASSERT( desc.fenceCount < Config::Driver::Synchronization::MAX_FENCES_TO_WAIT_COUNT,
                            "Fence count (desc.fenceCount) must be less than %d", Config::Driver::Synchronization::MAX_FENCES_TO_WAIT_COUNT );
                if( ( desc.pFences == nullptr ) || ( desc.pWaitValues == nullptr ) ||
                    ( desc.fenceCount >= Config::Driver::Synchronization::MAX_FENCES_TO_WAIT_COUNT ) )
                {
                    return Results::FAIL;
                }

                D3D12Fence* pNativeFence = nullptr;
                uint32_t    eventCnt     = 0;
                HANDLE      events[ Config::Driver::Synchronization::MAX_FENCES_TO_WAIT_COUNT ];
                for( index_t ndx = 0; ndx < static_cast<index_t>( desc.fenceCount ); ++ndx )
                {
                    if( desc.pFences[ ndx ] == FenceHandle() )
                    {
                        return Results::FAIL;
                    }
                    // Setting event only for fences which has lower internal value than wait value
                    pNativeFence = desc.pFences[ ndx ].GetNativeHandle();
                    if( pNativeFence->pFence->GetCompletedValue() < desc.pWaitValues[ ndx ] )
                    {
                        pNativeFence->pFence->SetEventOnCompletion( desc.pWaitValues[ ndx ], pNativeFence->hEvent );
                        events[ eventCnt++ ] = pNativeFence->hEvent;
                    }
                }
                // Waiting for necessary events
                if( WaitForMultipleObjects( eventCnt, events, desc.waitAll, static_cast<DWORD>( timeout ) ) == WAIT_TIMEOUT )
                {
                    return Results::TIMEOUT;
                }

                return Results::OK;
            }

            RESULT D3D12Device::SignalFence( uint64_t value, FenceHandle handle )
            {
                BGS_ASSERT( handle != FenceHandle(), "Fence (handle) must be a valid handle." );
                if( handle == FenceHandle() )
                {
                    return Results::FAIL;
                }

                D3D12Fence* pNativeFence = handle.GetNativeHandle();
                if( FAILED( pNativeFence->pFence->Signal( value ) ) )
                {
                    return Results::FAIL;
                }

                return Results::OK;
            }

            RESULT D3D12Device::GetFenceValue( FenceHandle handle, uint64_t* pValue )
            {
                BGS_ASSERT( handle != FenceHandle(), "Fence (handle) must be a valid handle." );
                if( handle == FenceHandle() )
                {
                    return Results::FAIL;
                }

                D3D12Fence* pNativeFence = handle.GetNativeHandle();
                *pValue                  = pNativeFence->pFence->GetCompletedValue();

                return Results::OK;
            }

            RESULT D3D12Device::CreateSemaphore( const SemaphoreDesc& desc, SemaphoreHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Semaphore (pHandle) must be a valid address." );
                desc;

                ID3D12Device* pNativeDevice    = m_handle.GetNativeHandle();
                ID3D12Fence*  pNativeSemaphore = nullptr;

                if( FAILED( pNativeDevice->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &pNativeSemaphore ) ) ) )
                {
                    return Results::FAIL;
                }

                *pHandle = SemaphoreHandle( pNativeSemaphore );

                return Results::OK;
            }

            void D3D12Device::DestroySemaphore( SemaphoreHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Semaphore handle (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != SemaphoreHandle(), "Semaphore handle (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != SemaphoreHandle() ) )
                {
                    ID3D12Fence* pNativeSemaphore = pHandle->GetNativeHandle();

                    RELEASE_COM_PTR( pNativeSemaphore );

                    *pHandle = SemaphoreHandle();
                }
            }

            RESULT D3D12Device::Create( const DeviceDesc& desc, D3D12Factory* pFactory )
            {
                BGS_ASSERT( pFactory != nullptr, "Factory (pFactory) must be a valid pointer." );
                BGS_ASSERT( desc.pAdapter != nullptr, "Adapter (pAdapter) must be a valid pointer." );
                if( ( pFactory == nullptr ) || ( desc.pAdapter == nullptr ) )
                {
                    return Results::FAIL;
                }

                m_desc    = desc;
                m_pParent = pFactory;

                if( BGS_FAILED( CreateD3D12Device() ) )
                {
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void D3D12Device::Destroy()
            {
                BGS_ASSERT( m_handle != DeviceHandle() );

                if( m_handle != DeviceHandle() )
                {
                    ID3D12Device* pNativeDevice = m_handle.GetNativeHandle();
                    RELEASE_COM_PTR( pNativeDevice );
                }

                m_handle  = DeviceHandle();
                m_pParent = nullptr;
            }

            RESULT D3D12Device::CreateD3D12Device()
            {
                IDXGIAdapter1* pNativeAdapter = m_desc.pAdapter->GetHandle().GetNativeHandle();
                ID3D12Device*  pNativeDevice  = nullptr;

                if( FAILED( D3D12CreateDevice( pNativeAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &pNativeDevice ) ) ) )
                {
                    return Results::FAIL;
                }

                m_handle = DeviceHandle( pNativeDevice );

                return Results::OK;
            }

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS