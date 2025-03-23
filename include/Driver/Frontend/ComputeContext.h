#pragma once
#include "Core/CoreTypes.h"
#include "Driver/Backend/APITypes.h"
#include "Driver/Frontend/RenderSystemTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            class BGS_API ComputeContext final
            {
                friend class RenderDevice;

            public:
                ComputeContext();
                ~ComputeContext() = default;

            private:
                RESULT Create( RenderDevice* pDevice );
                void   Destroy();

            private:
                RenderDevice*              m_pParent;
                Backend::IQueue*           m_pQueue;
                Backend::CommandPoolHandle m_hCmdPool;
                Backend::ICommandBuffer*   m_pCmdBuffer;
            };

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS