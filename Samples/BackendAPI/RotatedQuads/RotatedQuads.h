#pragma once

#include "Driver/Frontend/RenderDevice.h"
#include "Platform/WindowSystem.h"
#include "Sample.h"
#include "glm/glm.hpp"

using namespace BIGOS::Driver;

constexpr uint32_t FRAME_COUNT = 2;

constexpr uint32_t BACK_BUFFER_COUNT = 3;

constexpr float VERTEX_DATA[] = {
    -0.5f, -0.5f, 0.0f,  /* POSITION */ 1.0f, 0.0f, 0.0f, /* COLOR */
    0.5f,  -0.5f, 0.0f,  /* POSITION */ 0.0f, 1.0f, 0.0f, /* COLOR */
    0.5f,  0.5f,  0.0f,  /* POSITION */ 0.0f, 0.0f, 1.0f, /* COLOR */
    -0.5f, 0.5f,  0.0f,  /* POSITION */ 1.0f, 1.0f, 1.0f, /* COLOR */

    -0.5f, -0.5f, -0.5f, /* POSITION */ 1.0f, 0.0f, 0.0f, /* COLOR */
    0.5f,  -0.5f, -0.5f, /* POSITION */ 0.0f, 1.0f, 0.0f, /* COLOR */
    0.5f,  0.5f,  -0.5f, /* POSITION */ 0.0f, 0.0f, 1.0f, /* COLOR */
    -0.5f, 0.5f,  -0.5f, /* POSITION */ 1.0f, 1.0f, 1.0f, /* COLOR */
};

constexpr uint16_t INDEX_DATA[] = {
    // first quad
    0, 1, 2, 2, 3, 0,
    // second quad
    4, 5, 6, 6, 7, 4
};

class RotatedQuads : public Sample
{
public:
    RotatedQuads( APITypes APIType, uint32_t width, uint32_t height, const char* pName );

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
    BIGOS::Platform::WindowSystem* m_pWindowSystem;
    BIGOS::Platform::Window*       m_pWindow;
    Frontend::RenderSystem*        m_pRenderSystem;
    Frontend::RenderDevice*        m_pDevice;
    Backend::IDevice*              m_pAPIDevice;
    Backend::IQueue*               m_pQueue;

    // Pipeline objects
    Backend::ISwapchain*            m_pSwapchain;
    Backend::BindingSetLayoutHandle m_hShaderResourceSetLayout;
    Backend::BindingHeapHandle      m_hShaderResourceHeap;
    Backend::PipelineLayoutHandle   m_hPipelineLayout;
    Backend::PipelineHandle         m_hPipeline;
    Backend::CommandPoolHandle      m_hCommandPools[ BACK_BUFFER_COUNT ];
    Backend::ICommandBuffer*        m_pCommandBuffers[ BACK_BUFFER_COUNT ];
    Backend::ResourceViewHandle     m_hRTVs[ BACK_BUFFER_COUNT ];
    Backend::BackBufferArray        m_backBuffers;
    uint64_t                        m_cbvOffset;
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
    Backend::MemoryHandle   m_hBufferMemory;

    Backend::ResourceHandle     m_hDephtStencilTarget;
    Backend::MemoryHandle       m_hDepthStencilMemory;
    Backend::ResourceViewHandle m_hDepthStencilView;

    Backend::ResourceHandle     m_hConstantBuffer;
    Backend::MemoryHandle       m_hConstantBufferMemory;
    void*                       m_pConstantBufferHost;
    Backend::ResourceViewHandle m_hConstantBufferView;

    // Synchronization objects
    Backend::FenceHandle     m_hFences[ FRAME_COUNT ];
    Backend::SemaphoreHandle m_hFrameFinishedSemaphores[ FRAME_COUNT ];
    uint64_t                 m_fenceValues[ FRAME_COUNT ];
};