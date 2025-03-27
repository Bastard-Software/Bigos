#pragma once

#include "Driver/Frontend/RenderSystemTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            class BGS_API Swapchain final
            {
                friend class RenderSystem;
                friend class GraphicsContext;

            public:
                Swapchain();
                ~Swapchain() = default;

                RESULT Resize( Platform::Window* pWindow );

            protected:
                RESULT Create( const SwapchainDesc& desc, RenderSystem* pSystem );
                void   Destroy();

                RESULT Present( RenderTarget* pRenderTarget );

            private:
                Backend::SwapchainDesc                m_desc;
                Backend::ISwapchain*                  m_pSwapchain;
                HeapArray<Backend::SemaphoreHandle>   m_hCpySemaphores;
                Backend::FenceHandle                  m_hFence;
                uint64_t                              m_fenceVal;
                HeapArray<Backend::CommandPoolHandle> m_hCmdPools;
                HeapArray<Backend::ICommandBuffer*>   m_pCmdBuffers;
                RenderSystem*                         m_pParent;
                uint32_t                              m_frameNdx;
            };

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS