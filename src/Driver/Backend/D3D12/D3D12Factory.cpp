#define BGS_USE_D3D12_HANDLES 1

#include "D3D12Factory.h"

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

                return Results::OK;
            }

            void D3D12Factory::Destroy()
            {
                m_pParent = nullptr;
            }

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS