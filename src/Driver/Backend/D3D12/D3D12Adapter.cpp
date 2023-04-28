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
                pAdapter;
                pFactory;

                return Results::OK;
            }

            void D3D12Adapter::Destroy()
            {
            }
        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS