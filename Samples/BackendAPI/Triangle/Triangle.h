#pragma once

#include "Driver/Frontend/RenderDevice.h"
#include "Platform/WindowSystem.h"
#include "Sample.h"

using namespace BIGOS::Driver;

constexpr uint32_t FRAME_COUNT = 2;

constexpr uint32_t BACK_BUFFER_COUNT = 3;

class Triangle : public Sample
{
public:
    Triangle( APITypes APIType, uint32_t width, uint32_t height, const char* pName );

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
    // Core objects
    BIGOS::Platform::Event::EventSystem* m_pEventSystem;
    BIGOS::Platform::WindowSystem*       m_pWindowSystem;
    BIGOS::Platform::Window*             m_pWindow;
    Frontend::RenderSystem*              m_pRenderSystem;
    Frontend::RenderDevice*              m_pDevice;
    Backend::IDevice*                    m_pAPIDevice;
    Backend::IQueue*                     m_pQueue;

    // Pipeline objects
    Backend::ShaderHandle           m_hVertexShader, m_hPixelShader;
    Backend::ISwapchain*            m_pSwapchain;
    Backend::BindingSetLayoutHandle m_hBindingSetLayout;
    Backend::PipelineLayoutHandle   m_hPipelineLayout;
    Backend::PipelineHandle         m_hPipeline;
    Backend::CommandPoolHandle      m_hCommandPools[ BACK_BUFFER_COUNT ];
    Backend::ICommandBuffer*        m_pCommandBuffers[ BACK_BUFFER_COUNT ];
    Backend::ResourceViewHandle     m_hRTVs[ BACK_BUFFER_COUNT ];
    Backend::BackBufferArray        m_backBuffers;
    uint32_t                        m_frameNdx;

    // App resource
    Backend::ResourceHandle m_hVertexBuffer;
    Backend::MemoryHandle   m_hVertexBufferMemory;

    // Synchronization objects
    Backend::FenceHandle     m_hFences[ FRAME_COUNT ];
    Backend::SemaphoreHandle m_hFrameFinishedSemaphores[ FRAME_COUNT ];
    uint64_t                 m_fenceValues[ FRAME_COUNT ];
};