#pragma once

#include "Driver/Frontend/RenderSystemTypes.h"

namespace BIGOS
{
    class BGS_API Renderer
    {
        friend class IApplication;
        friend class WindowedApplication;

    public:
        Renderer();
        ~Renderer() = default;

        void Render();

    protected:
        RESULT Create( Driver::Frontend::RenderSystem* pSystem, Platform::Window* pWindow = nullptr );
        void   Destroy();

        RESULT Resize( uint32_t width, uint32_t height );

    private:
        RESULT CreateResizableResources();
        void   DestroyResizableResources();

    private:
        Driver::Frontend::RenderSystem*            m_pRenderSystem;
        Driver::Frontend::RenderDevice*            m_pDevice;
        Driver::Frontend::GraphicsContext*         m_pGraphicsContext;
        Platform::Window*                          m_pWindow;
        Driver::Frontend::Swapchain*               m_pSwapchain;
        uint32_t                                   m_frameCount;
        uint32_t                                   m_width;
        uint32_t                                   m_height;
        Driver::Backend::FORMAT                    m_rtFormat;
        Driver::Backend::FORMAT                    m_dsFormat;
        uint32_t                                   m_frameNdx;
        HeapArray<Driver::Frontend::RenderTarget*> m_pColorRTs; // Resizable
        Driver::Frontend::RenderTarget*            m_pDepthRT;  // Resizable
        Driver::Frontend::Buffer*                  m_pVertexBuffer;
        Driver::Frontend::Buffer*                  m_pIndexBuffer;
        Driver::Frontend::Buffer*                  m_pConstantBuffer;
        Driver::Frontend::Buffer*                  m_pStorageBuffer;
        Driver::Frontend::Shader*                  m_pVS;
        Driver::Frontend::Shader*                  m_pPS;
        Driver::Frontend::Pipeline*                m_pGraphicsPipeline;
        Driver::Frontend::Shader*                  m_pCS;
        Driver::Frontend::Pipeline*                m_pComputePipeline;
        Driver::Frontend::Texture*                 m_pSampledTexture;
        Driver::Frontend::Texture*                 m_pStorageTexture;
        Driver::Frontend::Texture*                 m_pSkyTexture;
    };
} // namespace BIGOS