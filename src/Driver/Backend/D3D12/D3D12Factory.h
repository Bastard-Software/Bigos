#pragma once
#include "D3D12Types.h"

#include "Driver/Backend/API.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {
            class RenderSystem;
        }

        namespace Backend
        {

            class BGS_API D3D12Factory final : public IFactory
            {
                friend class BIGOS::Driver::Frontend::RenderSystem;

            public:
                RESULT CreateDevice( const DeviceDesc& desc, IDevice** ppDevice ) override;
                void   DestroyDevice( IDevice** ppDevice ) override;

                // Funcions needed for D3D12 backend
            public:
                BIGOS::Driver::Frontend::RenderSystem* GetParent() { return m_pParent; }

            protected:
                RESULT Create( const FactoryDesc& desc, BIGOS::Driver::Frontend::RenderSystem* pParent );
                void   Destroy();

            private:
                RESULT EnumAdapters();

                RESULT CreateD3D12Factory();

            private:
                BIGOS::Driver::Frontend::RenderSystem* m_pParent = nullptr;

#if( BGS_RENDER_DEBUG )
                ID3D12Debug1*   m_pNativeDebug;
                IDXGIInfoQueue* m_pNativeInfoQueue;
#endif // ( BGS_RENDER_DEBUG )
            };

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS