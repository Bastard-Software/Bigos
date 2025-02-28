#pragma once

#include "Driver/Frontend/RenderSystemTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            class BGS_API RenderPass final
            {
                friend class RenderDevice;

            public:
                RenderPass();
                ~RenderPass() = default;

            protected:
                RESULT Create( const RenderPassDesc& desc, RenderDevice* pDevice );
                void   Destroy();

            private:
                RenderPassDesc m_desc;
                RenderDevice*  m_pParent;
            };

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS