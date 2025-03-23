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

                void BeginFrame();
                void SubmitFrame();

                void BeginRenderPass( RenderPass* pRenderPass );

                void BindPipeline( Pipeline* pPipeline );

                void Draw();

                RESULT Present( Swapchain* pSwapchain, RenderTarget* pRenderTarget );

            private:
                RESULT Create( uint32_t frameCnt, RenderDevice* pDevice );
                void   Destroy();

                struct FrameResources
                {
                    Backend::ICommandBuffer*   pCmdBuffer = nullptr;
                    Backend::CommandPoolHandle hCmdPool   = Backend::CommandPoolHandle();
                    uint64_t                   fenceVal   = 0;
                };

                RESULT CreateFrameResources();
                void DestroyFrameResources();

            private:
                HeapArray<FrameResources> m_frames;
                uint32_t                  m_frameIndex;
                RenderDevice*             m_pParent;
                Backend::IQueue*          m_pQueue;
                Backend::FenceHandle      m_hFence;
                uint64_t                  m_fenceVal;
                uint32_t                  m_frameCount;
                bool                      m_frameStarted;
            };

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS