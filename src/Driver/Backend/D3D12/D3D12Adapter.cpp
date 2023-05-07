#define BGS_USE_D3D12_HANDLES 1

#include "D3D12Adapter.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            RESULT D3D12Adapter::Create( IDXGIAdapter1* pAdapter, D3D12Factory* pFactory )
            {
                BGS_ASSERT( pAdapter != nullptr, "Adapter (pAdapter) must be a valid pointer." );
                BGS_ASSERT( pFactory != nullptr, "Factory (pFactory) must be a valid pointer." );

                m_handle  = AdapterHandle( pAdapter );
                m_pParent = pFactory;

                if( BGS_FAILED( GetInternalInfo() ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void D3D12Adapter::Destroy()
            {
                IDXGIAdapter1* pNativeAdapter = m_handle.GetNativeHandle();
                RELEASE_COM_PTR( pNativeAdapter );

                m_handle  = AdapterHandle();
                m_pParent = nullptr;
            }

            RESULT D3D12Adapter::GetInternalInfo()
            {
                return Results::OK;
            }
        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS