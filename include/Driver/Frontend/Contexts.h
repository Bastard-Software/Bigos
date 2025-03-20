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
            
            class BGS_API GraphicsContext final
            {
                friend class RenderDevice;

            public:
                GraphicsContext();
                ~GraphicsContext() = default;

                Backend::IQueue* GetQueue() const { return m_pQueue; }

                RESULT Present( Swapchain* pSwapchain, RenderTarget* pRenderTarget );

            private:
                RESULT Create( RenderDevice* pDevice );
                void   Destroy();

            private:
                RenderDevice*        m_pParent;
                Backend::IQueue*     m_pQueue;
                Backend::FenceHandle m_hFence;
                uint64_t             m_fenceVal;
            };

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

            class BGS_API CopyContext final
            {
                friend class RenderDevice;

            public:
                CopyContext();
                ~CopyContext() = default;

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