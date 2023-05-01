#pragma once
#include "D3D12Types.h"

#include "Driver/Backend/API.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {
            class DriverSystem;
        }

        namespace Backend
        {

            class BGS_API D3D12Factory final : public IFactory
            {
                friend class BIGOS::Driver::Frontend::DriverSystem;

            public:
                RESULT CreateDevice( const DeviceDesc& desc, IDevice** ppDevice ) override;
                void   DestroyDevice( IDevice** ppDevice ) override;

            protected:
                RESULT Create( const FactoryDesc& desc, BIGOS::Driver::Frontend::DriverSystem* pParent );
                void   Destroy();

            private:
                BIGOS::Driver::Frontend::DriverSystem* m_pParent;
            };

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS