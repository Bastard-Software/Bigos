#pragma once

#include "Platform/Window/WindowSystem.h"
#include "Sample.h"

using namespace BIGOS::Driver;

constexpr uint32_t FRAME_COUNT = 2;

constexpr uint32_t BACK_BUFFER_COUNT = 3;

class AnimatedTriangle : public Sample
{
public:
    AnimatedTriangle( APITypes APIType, uint32_t width, uint32_t height, const char* pName );

    virtual BIGOS::RESULT OnInit() override;
    virtual void          OnUpdate() override;
    virtual void          OnRender() override;
    virtual void          OnDestroy() override;

private:
    BIGOS::RESULT InitDevice();
    BIGOS::RESULT CreateFrameResources();
    BIGOS::RESULT CreateApplicationResources();
    BIGOS::RESULT CreatePipeline();

private:
    // Constant buffer struct
    struct ConstantBufferStruct
    {
        float offset[ 4 ];
        float padding[ 60 ]; // Constant buffers needs to be 256 aligned
    } m_constantBufferData;

    // Core objects
    BIGOS::Platform::Event::EventSystem* m_pEventSystem;
    BIGOS::Platform::Window*             m_pWindow;
    Backend::IDevice*                    m_pAPIDevice;
    Backend::IQueue*                     m_pQueue;

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

    Backend::ResourceHandle m_hVertexBuffer;
    Backend::MemoryHandle   m_hVertexBufferMemory;

    Backend::ResourceHandle     m_hConstantBuffer;
    Backend::MemoryHandle       m_hConstantBufferMemory;
    void*                       m_pConstantBufferHost;
    Backend::ResourceViewHandle m_hConstantBufferView;

    // Synchronization objects
    Backend::FenceHandle     m_hFences[ FRAME_COUNT ];
    Backend::SemaphoreHandle m_hFrameFinishedSemaphores[ FRAME_COUNT ];
    uint64_t                 m_fenceValues[ FRAME_COUNT ];
};