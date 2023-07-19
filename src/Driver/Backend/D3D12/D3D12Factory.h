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
                RESULT EnumAdapters();

                RESULT CreateD3D12Factory();

            private:
                BIGOS::Driver::Frontend::DriverSystem* m_pParent;

#if( BGS_RENDER_DEBUG )
                ID3D12Debug1*   m_pNativeDebug;
                IDXGIInfoQueue* m_pNativeInfoQueue;
#endif // ( BGS_RENDER_DEBUG )
            };

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS