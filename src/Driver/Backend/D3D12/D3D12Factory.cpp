#define BGS_USE_D3D12_HANDLES 1

#include "D3D12Factory.h"

#include "Core/CoreTypes.h"

#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Memory.h"
#include "D3D12Adapter.h"
#include "Driver/Frontend/DriverSystem.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {

            RESULT D3D12Factory::CreateDevice( const DeviceDesc& desc, IDevice** ppDevice )
            {
                BGS_ASSERT( ( ppDevice != nullptr ) && ( *ppDevice == nullptr ) );
                desc;

                return Results::OK;
            }

            void D3D12Factory::DestroyDevice( IDevice** ppDevice )
            {
                BGS_ASSERT( ( ppDevice != nullptr ) && ( *ppDevice != nullptr ) );
            }

            RESULT D3D12Factory::Create( const FactoryDesc& desc, BIGOS::Driver::Frontend::DriverSystem* pParent )
            {
                BGS_ASSERT( pParent != nullptr );

                m_desc    = desc;
                m_pParent = pParent;

                if( BGS_FAILED( CreateD3D12Factory() ) )
                {
                    return Results::FAIL;
                }

                if( BGS_FAILED( EnumAdapters() ) )
                {
                    Destroy();
                    return Results::NOT_FOUND;
                }

                return Results::OK;
            }

            void D3D12Factory::Destroy()
            {
#if( BGS_RENDER_DEBUG )
                RELEASE_COM_PTR( m_pNativeInfoQueue );
                RELEASE_COM_PTR( m_pNativeDebug );
#endif // ( BGS_RENDER_DEBUG )

                for( index_t i = 0; i < m_adapters.size(); ++i )
                {
                    static_cast<D3D12Adapter*>( m_adapters[ i ] )->Destroy();
                    Core::Memory::FreeObject( m_pParent->GetDefaultAllocatorPtr(), &m_adapters[ i ] );
                }
                m_adapters.clear();

                BGS_ASSERT( m_handle != FactoryHandle() );
                IDXGIFactory4* pNativeFactory = m_handle.GetNativeHandle();
                RELEASE_COM_PTR( pNativeFactory );

                m_handle  = FactoryHandle();
                m_pParent = nullptr;
            }

            RESULT D3D12Factory::EnumAdapters()
            {
                IDXGIFactory4* pNativeFactory = m_handle.GetNativeHandle();

                for( index_t i = 0;; ++i )
                {
                    IDXGIAdapter1* pAdapter   = nullptr;
                    const UINT     adapterNdx = static_cast<UINT>( i );

                    if( DXGI_ERROR_NOT_FOUND == pNativeFactory->EnumAdapters1( adapterNdx, &pAdapter ) )
                    {
                        // Iterating through all adapters until none are available.
                        break;
                    }

                    if( SUCCEEDED( D3D12CreateDevice( pAdapter, D3D_FEATURE_LEVEL_11_0, __uuidof( ID3D12Device ), nullptr ) ) )
                    {
                        D3D12Adapter* pD3D12Adapter = nullptr;
                        if( BGS_FAILED( Core::Memory::AllocateObject( m_pParent->GetDefaultAllocatorPtr(), &pD3D12Adapter ) ) )
                        {
                            return Results::NO_MEMORY;
                        }
                        if( BGS_FAILED( pD3D12Adapter->Create( pAdapter, this ) ) )
                        {
                            return Results::FAIL;
                        }
                        m_adapters.push_back( pD3D12Adapter );
                    }
                }

                return Results::OK;
            }

            RESULT D3D12Factory::CreateD3D12Factory()
            {
                HRESULT        hr;
                IDXGIFactory4* pNativeFactory = nullptr;

                if( m_desc.flags > 0 )
                {
#if( BGS_RENDER_DEBUG )
                    if( FAILED( D3D12GetDebugInterface( IID_PPV_ARGS( &m_pNativeDebug ) ) ) )
                    {
                        if( m_desc.flags & static_cast<uint32_t>( FactoryFlagBits::ENABLE_DEBUG_LAYERS ) )
                        {
                            return Results::FAIL;
                        }
                    }
                    else
                    {
                        m_pNativeDebug->EnableDebugLayer();

                        if( m_desc.flags & static_cast<uint32_t>( FactoryFlagBits::ENABLE_GPU_BASED_VALIDATION ) )
                        {
                            m_pNativeDebug->SetEnableGPUBasedValidation( TRUE );
                        }
                        if( m_desc.flags & static_cast<uint32_t>( FactoryFlagBits::ENABLE_QUEUE_SYNCHRONIZATION_VALIDATION ) )
                        {
                            m_pNativeDebug->SetEnableSynchronizedCommandQueueValidation( TRUE );
                        }
                    }

                    if( FAILED( DXGIGetDebugInterface1( 0, IID_PPV_ARGS( &m_pNativeInfoQueue ) ) ) )
                    {
                        RELEASE_COM_PTR( m_pNativeDebug );
                        return Results::FAIL;
                    }
                    else
                    {
                        if( FAILED( CreateDXGIFactory2( DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS( &pNativeFactory ) ) ) )
                        {
                            RELEASE_COM_PTR( m_pNativeInfoQueue );
                            RELEASE_COM_PTR( m_pNativeDebug );
                            return Results::FAIL;
                        }
                        else
                        {
                            m_pNativeInfoQueue->SetBreakOnSeverity( DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true );
                            m_pNativeInfoQueue->SetBreakOnSeverity( DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true );
                            m_pNativeInfoQueue->SetBreakOnSeverity( DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING, true );
                        }
                    }
#endif // ( BGS_RENDER_DEBUG )
                }
                else // No debug features
                {
                    hr = CreateDXGIFactory1( IID_PPV_ARGS( &pNativeFactory ) );
                    if( FAILED( hr ) )
                    {
                        return Results::FAIL;
                    }
                }

                m_handle = FactoryHandle( pNativeFactory );

                return Results::OK;
            }

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS