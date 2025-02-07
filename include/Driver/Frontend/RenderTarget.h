#pragma once

#include "Driver/Frontend/RenderSystemTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            class BGS_API RenderTarget final
            {
                friend class RenderDevice;

            public:
                RenderTarget();
                ~RenderTarget() = default;

            protected:
                RESULT Create( const RenderTargetDesc& desc, RenderDevice* pDevice );
                void   Destroy();

            private:
                RenderTargetDesc            m_desc;
                Backend::ResourceDesc       m_targetDesc;
                RenderDevice*               m_pParent;
                Backend::MemoryHandle       m_hMemory;
                Backend::ResourceHandle     m_hResource;
                Backend::ResourceViewHandle m_hView;
                Backend::ResourceViewHandle m_hSampleView;
                // TODO: Should Render Target store MS resource?
            };
        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS