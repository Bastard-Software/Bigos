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
                friend class RenderSystem;

            public:
                RenderPass();
                ~RenderPass() = default;

            protected:
                RESULT Create( const RenderPassDesc& desc, RenderSystem* pSystem );
                void   Destroy();

            private:
                RenderPassDesc m_desc;
                RenderSystem*  m_pParent;
            };

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS