#define BGS_USE_D3D12_HANDLES 1

#include "D3D12Device.h"

#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Memory.h"
#include "D3D12Common.h"
#include "D3D12Factory.h"
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
                BGS_ASSERT( ppQueue != nullptr, "ppQueue must be a valid address." );
                BGS_ASSERT( *ppQueue == nullptr, "There is a pointer at the given address. *ppQueue must be nullptr." );
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
                BGS_ASSERT( ppQueue != nullptr, "ppQueue must be a valid address." );
                BGS_ASSERT( *ppQueue != nullptr, "*ppQueue must be a valid pointer." );
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