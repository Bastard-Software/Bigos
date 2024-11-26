#pragma once

#include "Driver/Frontend/RenderDevice.h"
#include "Platform/WindowSystem.h"
#include "Sample.h"
#include "glm/glm.hpp"

using namespace BIGOS::Driver;

constexpr uint32_t FRAME_COUNT = 2;

constexpr uint32_t BACK_BUFFER_COUNT = 3;

constexpr uint32_t TEXTURE_WIDTH  = 256;
constexpr uint32_t TEXTURE_HEIGHT = 256;

constexpr float VERTEX_DATA[] = {
    -0.5f, -0.5f, -0.5f, /* POSITION */ 0.0f, 0.0f, /* TEXCOORD */
    0.5f,  -0.5f, -0.5f, /* POSITION */ 1.0f, 0.0f, /* TEXCOORD */
    0.5f,  0.5f,  -0.5f, /* POSITION */ 1.0f, 1.0f, /* TEXCOORD */
    -0.5f, 0.5f,  -0.5f, /* POSITION */ 0.0f, 1.0f, /* TEXCOORD */
    -0.5f, -0.5f, 0.5f,  /* POSITION */ 0.0f, 0.0f, /* TEXCOORD */
    0.5f,  -0.5f, 0.5f,  /* POSITION */ 1.0f, 0.0f, /* TEXCOORD */
    0.5f,  0.5f,  0.5f,  /* POSITION */ 1.0f, 1.0f, /* TEXCOORD */
    -0.5f, 0.5f,  0.5f,  /* POSITION */ 0.0f, 1.0f, /* TEXCOORD */

    -0.5f, 0.5f,  -0.5f, /* POSITION */ 0.0f, 0.0f, /* TEXCOORD */
    -0.5f, -0.5f, -0.5f, /* POSITION */ 1.0f, 0.0f, /* TEXCOORD */
    -0.5f, -0.5f, 0.5f,  /* POSITION */ 1.0f, 1.0f, /* TEXCOORD */
    -0.5f, 0.5f,  0.5f,  /* POSITION */ 0.0f, 1.0f, /* TEXCOORD */
    0.5f,  -0.5f, -0.5f, /* POSITION */ 0.0f, 0.0f, /* TEXCOORD */
    0.5f,  0.5f,  -0.5f, /* POSITION */ 1.0f, 0.0f, /* TEXCOORD */
    0.5f,  0.5f,  0.5f,  /* POSITION */ 1.0f, 1.0f, /* TEXCOORD */
    0.5f,  -0.5f, 0.5f,  /* POSITION */ 0.0f, 1.0f, /* TEXCOORD */

    -0.5f, -0.5f, -0.5f, /* POSITION */ 0.0f, 0.0f, /* TEXCOORD */
    0.5f,  -0.5f, -0.5f, /* POSITION */ 1.0f, 0.0f, /* TEXCOORD */
    0.5f,  -0.5f, 0.5f,  /* POSITION */ 1.0f, 1.0f, /* TEXCOORD */
    -0.5f, -0.5f, 0.5f,  /* POSITION */ 0.0f, 1.0f, /* TEXCOORD */
    0.5f,  0.5f,  -0.5f, /* POSITION */ 0.0f, 0.0f, /* TEXCOORD */
    -0.5f, 0.5f,  -0.5f, /* POSITION */ 1.0f, 0.0f, /* TEXCOORD */
    -0.5f, 0.5f,  0.5f,  /* POSITION */ 1.0f, 1.0f, /* TEXCOORD */
    0.5f,  0.5f,  0.5f,  /* POSITION */ 0.0f, 1.0f, /* TEXCOORD */
};

constexpr uint16_t INDEX_DATA[] = {
    // front and back
    0, 3, 2, 2, 1, 0, 4, 5, 6, 6, 7, 4,
    // left and right
    11, 8, 9, 9, 10, 11, 12, 13, 14, 14, 15, 12,
    // bottom and top
    16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20
};

constexpr BIGOS::Driver::Backend::DrawIndexedIndirectArguments INDIRECT_DATA = { 36, 1, 0, 0, 0 };

class IndirectCube : public Sample
{
public:
    IndirectCube( APITypes APIType, uint32_t width, uint32_t height, const char* pName );

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
    // Constant buffer struct
    struct ConstantBufferStruct
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
        float     padding[ 16 ]; // Constant buffers needs to be 256 aligned
    } m_constantBufferData;

    // Core objects
    BIGOS::Platform::Event::EventSystem* m_pEventSystem;
    BIGOS::Platform::WindowSystem*       m_pWindowSystem;
    BIGOS::Platform::Window*             m_pWindow;
    Frontend::RenderSystem*              m_pRenderSystem;
    Frontend::RenderDevice*              m_pDevice;
    Backend::IDevice*                    m_pAPIDevice;
    Backend::IQueue*                     m_pQueue;

    // Pipeline objects
    Backend::ISwapchain*            m_pSwapchain;
    Backend::BindingSetLayoutHandle m_hShaderResourceSetLayout;
    Backend::BindingSetLayoutHandle m_hSamplerSetLayout;
    Backend::BindingHeapHandle      m_hShaderResourceHeap;
    Backend::BindingHeapHandle      m_hSamplerHeap;
    Backend::PipelineLayoutHandle   m_hPipelineLayout;
    Backend::PipelineHandle         m_hPipeline;
    Backend::CommandLayoutHandle    m_hCommandLayout;
    Backend::CommandPoolHandle      m_hCommandPools[ BACK_BUFFER_COUNT ];
    Backend::ICommandBuffer*        m_pCommandBuffers[ BACK_BUFFER_COUNT ];
    Backend::ResourceViewHandle     m_hRTVs[ BACK_BUFFER_COUNT ];
    Backend::BackBufferArray        m_backBuffers;
    uint64_t                        m_cbvOffset;
    uint64_t                        m_texOffset;
    uint64_t                        m_samplerOffset;
    uint32_t                        m_frameNdx;

    // App resource
    Backend::ShaderHandle m_hVertexShader;
    Backend::ShaderHandle m_hPixelShader;

    uint32_t m_shaderSourceSize;
    void*    m_pShaderSource;

    Backend::ResourceHandle m_hUploadBuffer;
    Backend::MemoryHandle   m_hUploadBufferMemory;

    Backend::ResourceHandle m_hVertexBuffer;
    Backend::ResourceHandle m_hIndexBuffer;
    Backend::ResourceHandle m_hIndirectBuffer;
    Backend::MemoryHandle   m_hBufferMemory;

    Backend::ResourceHandle     m_hDephtStencilTarget;
    Backend::MemoryHandle       m_hDepthStencilMemory;
    Backend::ResourceViewHandle m_hDepthStencilView;

    Backend::ResourceHandle     m_hConstantBuffer;
    Backend::MemoryHandle       m_hConstantBufferMemory;
    void*                       m_pConstantBufferHost;
    Backend::ResourceViewHandle m_hConstantBufferView;

    Backend::ResourceHandle     m_hTexture;
    Backend::MemoryHandle       m_hTextureMemory;
    Backend::ResourceViewHandle m_hSampledTextureView;
    Backend::SamplerHandle      m_hSampler;

    // Synchronization objects
    Backend::FenceHandle     m_hFences[ FRAME_COUNT ];
    Backend::SemaphoreHandle m_hFrameFinishedSemaphores[ FRAME_COUNT ];
    uint64_t                 m_fenceValues[ FRAME_COUNT ];
};