#pragma once

#include "Driver/Frontend/RenderDevice.h"
#include "Platform/WindowSystem.h"
#include "Sample.h"

using namespace BIGOS::Driver;

constexpr uint32_t BUFFER_ELEMENT_COUNT = 32;

class Fibonacci : public Sample
{
public:
    Fibonacci( APITypes APIType, uint32_t width, uint32_t height, const char* pName );

    virtual BIGOS::RESULT OnInit() override;
    virtual void          OnUpdate() override;
    virtual void          OnRender() override;
    virtual void          OnDestroy() override;

private:
    BIGOS::RESULT InitDevice();
    BIGOS::RESULT CreateApplicationResources();
    BIGOS::RESULT CreatePipeline();
    BIGOS::RESULT RecordComputeCommandBuffer();
    BIGOS::RESULT PrintResult();

private:
    // Storage buffer struct
    struct StorageBufferStruct
    {
        uint32_t data[ BUFFER_ELEMENT_COUNT ];
    } m_storageBufferData;

    // Core objects
    Frontend::RenderSystem* m_pRenderSystem;
    Frontend::RenderDevice* m_pDevice;
    Backend::IDevice*       m_pAPIDevice;
    Backend::IQueue*        m_pQueue;

    // Pipeline objects
    Backend::ShaderHandle           m_hComputeShader;
    Backend::BindingSetLayoutHandle m_hShaderResourceSetLayout;
    Backend::BindingHeapHandle      m_hShaderResourceHeap;
    Backend::PipelineLayoutHandle   m_hPipelineLayout;
    Backend::PipelineHandle         m_hPipeline;
    uint64_t                        m_sbvOffset;
    uint64_t                        m_cbvOffset;

    // App resource
    Backend::CommandPoolHandle  m_hCommandPool;
    Backend::ICommandBuffer*    m_pCommandBuffer;
    Backend::ResourceHandle     m_hStorageBuffer;
    Backend::MemoryHandle       m_hStorageBufferMemory;
    Backend::ResourceViewHandle m_hStorageBufferView;

    // Synchronization objects
    Backend::FenceHandle m_hComputeFinishedFence;
    uint64_t             m_fenceValue;
};