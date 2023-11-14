#pragma once

#include "Driver/Frontend/RenderDevice.h"
#include "Platform/WindowSystem.h"
#include "Sample.h"

using namespace BIGOS::Driver;

constexpr uint32_t FRAME_COUNT = 2;

constexpr float VERTEX_DATA[] = {
    -0.5f, -0.5f, 0.0f, 1.0f /* POSITION */, 1.0f, 0.0f, /* TEXCOORD */
    0.5f,  -0.5f, 0.0f, 1.0f /* POSITION */, 0.0f, 0.0f, /* TEXCOORD */
    0.5f,  0.5f,  0.0f, 1.0f /* POSITION */, 0.0f, 1.0f, /* TEXCOORD */
    -0.5f, 0.5f,  0.0f, 1.0f /* POSITION */, 1.0f, 1.0f, /* TEXCOORD */
};

constexpr uint16_t INDEX_DATA[] = {
    0, 1, 2, 2, 3, 0,
};

class TexturedQuad : public Sample
{
public:
    TexturedQuad( APITypes APIType, uint32_t width, uint32_t height, const char* pName );

    virtual BIGOS::RESULT OnInit() override;
    virtual void          OnUpdate() override;
    virtual void          OnRender() override;
    virtual void          OnDestroy() override;

private:
    BIGOS::RESULT InitDevice();
    BIGOS::RESULT LoadAssets();
    BIGOS::RESULT CreateFrameResources();
    BIGOS::RESULT CreateApplicationResources();
    BIGOS::RESULT UploadResourceData();
    BIGOS::RESULT CreatePipeline();

private:
    // Core objects
    BIGOS::Platform::WindowSystem* m_pWindowSystem;
    BIGOS::Platform::Window*       m_pWindow;
    Frontend::RenderSystem*        m_pRenderSystem;
    Frontend::RenderDevice*        m_pDevice;
    Backend::IDevice*              m_pAPIDevice;
    Backend::IQueue*               m_pQueue;

    // Pipeline objects
    Backend::ISwapchain*             m_pSwapchain;
    Backend::BindingHeapLayoutHandle m_hBindingHeapLayout;
    Backend::BindingHeapHandle       m_hSamplerHeap;
    Backend::BindingHeapHandle       m_hShaderResourceHeap;
    Backend::PipelineLayoutHandle    m_hPipelineLayout;
    Backend::PipelineHandle          m_hPipeline;
    Backend::CommandPoolHandle       m_hCommandPools[ FRAME_COUNT ];
    Backend::ICommandBuffer*         m_pCommandBuffers[ FRAME_COUNT ];
    Backend::ResourceViewHandle      m_hRTVs[ FRAME_COUNT ];
    Backend::BackBufferArray         m_backBuffers;
    uint64_t                         m_bindingOffset;
    uint32_t                         m_frameNdx;

    // App resource
    Backend::ShaderHandle m_hVertexShader;
    Backend::ShaderHandle m_hPixelShader;

    uint32_t m_shaderSourceSize;
    void*    m_pShaderSource;

    Backend::ResourceHandle m_hUploadBuffer;
    Backend::MemoryHandle   m_hUploadBufferMemory;

    Backend::ResourceHandle m_hVertexBuffer;
    Backend::ResourceHandle m_hIndexBuffer;
    Backend::MemoryHandle   m_hBufferMemory;

    Backend::ResourceHandle     m_hTexture;
    Backend::MemoryHandle       m_hTextureMemory;
    Backend::ResourceViewHandle m_hSampledTextureView;
    Backend::SamplerHandle      m_hSampler;

    // Synchronization objects
    Backend::FenceHandle     m_hFences[ FRAME_COUNT ];
    Backend::SemaphoreHandle m_hFrameFinishedSemaphores[ FRAME_COUNT ];
    uint64_t                 m_fenceValues[ FRAME_COUNT ];
};