#pragma once

#include "Driver/Frontend/DriverSystemTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            class BGS_API Device final
            {
                friend class DriverSystem;

            public:
                Device()  = default;
                ~Device() = default;

            protected:
                RESULT Create( const DeviceDesc& desc, Backend::IFactory* pFactory, DriverSystem* pDriverSystem );
                void   Destroy();

            private:
                DeviceDesc         m_desc;
                Backend::IFactory* m_pFactory = nullptr;
                DriverSystem*      m_pParent  = nullptr;
                Backend::IDevice*  m_pDevice  = nullptr;
            };

        } // namespace Frontend
    }     // namespace Driver
} // namespace BIGOS