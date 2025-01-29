#pragma once

#include "Driver/Frontend/RenderSystemTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            class BGS_API RenderDevice final
            {
                friend class RenderSystem;

            public:
                Backend::IDevice* GetNativeAPIDevice() { return m_pAPIDevice; }

            protected:
                RESULT Create( const RenderDeviceDesc& desc, Backend::IFactory* pFactory, RenderSystem* pDriverSystem );
                void   Destroy();

            private:
                RenderDeviceDesc   m_desc;
                Backend::IFactory* m_pFactory   = nullptr;
                RenderSystem*      m_pParent    = nullptr;
                Backend::IDevice*  m_pAPIDevice = nullptr;
            };

        } // namespace Frontend
    }     // namespace Driver
} // namespace BIGOS