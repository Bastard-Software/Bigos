#define BGS_USE_D3D12_HANDLES 1

#include "D3D12Device.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {

            RESULT D3D12Device::Create( const DeviceDesc& desc, D3D12Factory* pFactory )
            {
                BGS_ASSERT( pFactory != nullptr, "Factory (pFactory) must be a valid pointer." );
                BGS_ASSERT( desc.h_adapter != AdapterHandle(), "Adapter handle (h_adapter) must be a valid adapter handle." );

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
                ID3D12Device* pNativeDevice = m_handle.GetNativeHandle();
                RELEASE_COM_PTR( pNativeDevice );

                m_handle  = DeviceHandle();
                m_pParent = nullptr;
            }

            RESULT D3D12Device::CreateD3D12Device()
            {
                IDXGIAdapter1* pNativeAdapter = m_desc.h_adapter.GetNativeHandle();
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