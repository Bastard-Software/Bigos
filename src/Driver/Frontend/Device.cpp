#include "Driver/Frontend/Device.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {
            RESULT Device::Create( const DeviceDesc& desc, Backend::IFactory* pFactory, DriverSystem* pDriverSystem )
            {
                BGS_ASSERT( pDriverSystem != nullptr, "Driver system (pDriverSystem) must be a valid pointer." );
                BGS_ASSERT( pFactory != nullptr, "Factory (pFactory) must be a valid pointer." );

                m_desc     = desc;
                m_pFactory = pFactory;
                m_pParent  = pDriverSystem;

                Backend::DeviceDesc backendDesc;
                backendDesc.h_adapter = Backend::AdapterHandle( m_pFactory->GetAdapters()[m_desc.adapter.index] );
            

            Backend::IDevice* pDevice = nullptr;
            if( BGS_FAILED( m_pFactory->CreateDevice( backendDesc, &pDevice ) ) )
            {
                Results::FAIL;
            }

            m_pDevice = pDevice;

            return Results::OK;
        }

        void Device::Destroy()
        {
            m_pFactory = nullptr;
            m_pParent  = nullptr;
        }
    } // namespace Frontend
} // namespace Driver
} // namespace BIGOS