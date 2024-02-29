#pragma once

#include "Driver/Frontend/RenderDevice.h"
#include "Platform/WindowSystem.h"
#include "Sample.h"

using namespace BIGOS::Driver;

constexpr uint32_t FRAME_COUNT = 2;

constexpr float VERTEX_DATA[] = {
    // Front face
    -0.5f, -0.5f, -0.5f, 1.0f /* POSITION */, 0.0f, 0.0f, -1.0f, /* NORMAL0 */ 1.0f, 0.0f, 1.0f, /* COLOR0 */
    -0.5f, 0.5f, -0.5f, 1.0f /* POSITION */, 0.0f, 0.0f, -1.0f, /* NORMAL0 */ 1.0f, 0.0f, 1.0f,  /* COLOR0 */
    0.5f, 0.5f, -0.5f, 1.0f /* POSITION */, 0.0f, 0.0f, -1.0f, /* NORMAL0 */ 1.0f, 0.0f, 1.0f,   /* COLOR0 */
    0.5f, -0.5f, -0.5f, 1.0f /* POSITION */, 0.0f, 0.0f, -1.0f, /* NORMAL0 */ 1.0f, 0.0f, 1.0f,  /* COLOR0 */

    // Back face
    -0.5f, -0.5f, 0.5f, 1.0f /* POSITION */, 0.0f, 0.0f, 1.0f, /* NORMAL0 */ 1.0f, 0.0f, 1.0f, /* COLOR0 */
    0.5f, -0.5f, 0.5f, 1.0f /* POSITION */, 0.0f, 0.0f, 1.0f, /* NORMAL0 */ 1.0f, 0.0f, 1.0f,  /* COLOR0 */
    0.5f, 0.5f, 0.5f, 1.0f /* POSITION */, 0.0f, 0.0f, 1.0f, /* NORMAL0 */ 1.0f, 0.0f, 1.0f,   /* COLOR0 */
    -0.5f, 0.5f, 0.5f, 1.0f /* POSITION */, 0.0f, 0.0f, 1.0f, /* NORMAL0 */ 1.0f, 0.0f, 1.0f,  /* COLOR0 */

    // Top face
    -0.5f, 0.5f, -0.5f, 1.0f /* POSITION */, 0.0f, 1.0f, 0.0f, /* NORMAL0 */ 1.0f, 0.0f, 1.0f, /* COLOR0 */
    -0.5f, 0.5f, 0.5f, 1.0f /* POSITION */, 0.0f, 1.0f, 0.0f, /* NORMAL0 */ 1.0f, 0.0f, 1.0f,  /* COLOR0 */
    0.5f, 0.5f, 0.5f, 1.0f /* POSITION */, 0.0f, 1.0f, 0.0f, /* NORMAL0 */ 1.0f, 0.0f, 1.0f,   /* COLOR0 */
    0.5f, 0.5f, -0.5f, 1.0f /* POSITION */, 0.0f, 1.0f, 0.0f, /* NORMAL0 */ 1.0f, 0.0f, 1.0f,  /* COLOR0 */

    // Bottom face
    -0.5f, -0.5f, -0.5f, 1.0f /* POSITION */, 0.0f, -1.0f, 0.0f, /* NORMAL0 */ 1.0f, 0.0f, 1.0f, /* COLOR0 */
    0.5f, -0.5f, -0.5f, 1.0f /* POSITION */, 0.0f, -1.0f, 0.0f, /* NORMAL0 */ 1.0f, 0.0f, 1.0f,  /* COLOR0 */
    0.5f, -0.5f, 0.5f, 1.0f /* POSITION */, 0.0f, -1.0f, 0.0f, /* NORMAL0 */ 1.0f, 0.0f, 1.0f,   /* COLOR0 */
    -0.5f, -0.5f, 0.5f, 1.0f /* POSITION */, 0.0f, -1.0f, 0.0f, /* NORMAL0 */ 1.0f, 0.0f, 1.0f,  /* COLOR0 */

    // Left face
    -0.5f, -0.5f, 0.5f, 1.0f /* POSITION */, -1.0f, 0.0f, 0.0f, /* NORMAL0 */ 1.0f, 0.0f, 1.0f,  /* COLOR0 */
    -0.5f, 0.5f, 0.5f, 1.0f /* POSITION */, -1.0f, 0.0f, 0.0f, /* NORMAL0 */ 1.0f, 0.0f, 1.0f,   /* COLOR0 */
    -0.5f, 0.5f, -0.5f, 1.0f /* POSITION */, -1.0f, 0.0f, 0.0f, /* NORMAL0 */ 1.0f, 0.0f, 1.0f,  /* COLOR0 */
    -0.5f, -0.5f, -0.5f, 1.0f /* POSITION */, -1.0f, 0.0f, 0.0f, /* NORMAL0 */ 1.0f, 0.0f, 1.0f, /* COLOR0 */

    // Right face
    0.5f, -0.5f, -0.5f, 1.0f /* POSITION */, 1.0f, 0.0f, 0.0f, /* NORMAL0 */ 1.0f, 0.0f, 1.0f, /* COLOR0 */
    0.5f, 0.5f, -0.5f, 1.0f /* POSITION */, 1.0f, 0.0f, 0.0f, /* NORMAL0 */ 1.0f, 0.0f, 1.0f,  /* COLOR0 */
    0.5f, 0.5f, 0.5f, 1.0f /* POSITION */, 1.0f, 0.0f, 0.0f, /* NORMAL0 */ 1.0f, 0.0f, 1.0f,   /* COLOR0 */
    0.5f, -0.5f, 0.5f, 1.0f /* POSITION */, 1.0f, 0.0f, 0.0f, /* NORMAL0 */ 1.0f, 0.0f, 1.0f,  /* COLOR0 */
};

constexpr uint16_t INDEX_DATA[] = {
    // Front face
    0,
    1,
    2,
    0,
    2,
    3,

    // Back face
    4,
    5,
    6,
    4,
    6,
    7,

    // Top face
    8,
    9,
    10,
    8,
    10,
    11,

    // Bottom face
    12,
    13,
    14,
    12,
    14,
    15,

    // Left face
    16,
    17,
    18,
    16,
    18,
    19,

    // Right face
    20,
    21,
    22,
    20,
    22,
    23,
};

class DebugNormals : public Sample
{
public:
    DebugNormals( APITypes APIType, uint32_t width, uint32_t height, const char* pName );

    virtual BIGOS::RESULT OnInit() override;
    virtual void          OnUpdate() override;
    virtual void          OnRender() override;
    virtual void          OnDestroy() override;

private:
    BIGOS::RESULT InitDevice();
    BIGOS::RESULT CreateFrameResources();
    BIGOS::RESULT CreateApplicationResources();
    BIGOS::RESULT UploadResourceData();
    BIGOS::RESULT CreatePipeline();

private:
    // Constant buffer struct
    struct ConstantBufferStruct
    {
        float projection[ 4 ][ 4 ];
        float model[ 4 ][ 4 ];
        float padding[ 32 ]; // Constant buffers needs to be 256 aligned
    } m_constantBufferData;

    // Core objects
    BIGOS::Platform::WindowSystem* m_pWindowSystem;
    BIGOS::Platform::Window*       m_pWindow;
    Frontend::RenderSystem*        m_pRenderSystem;
    Frontend::RenderDevice*        m_pDevice;
    Backend::IDevice*              m_pAPIDevice;
    Backend::IQueue*               m_pQueue;

    // Pipeline objects
    // Common
    Backend::ISwapchain*            m_pSwapchain;
    Backend::BindingSetLayoutHandle m_hShaderResourceSetLayout;
    Backend::BindingHeapHandle      m_hShaderResourceHeap;
    Backend::PipelineLayoutHandle   m_hPipelineLayout;
    Backend::CommandPoolHandle      m_hCommandPools[ FRAME_COUNT ];
    Backend::ICommandBuffer*        m_pCommandBuffers[ FRAME_COUNT ];
    Backend::ResourceViewHandle     m_hRTVs[ FRAME_COUNT ];
    Backend::BackBufferArray        m_backBuffers;
    uint32_t                        m_frameNdx;
    uint64_t                        m_bindingsOffset;

    // Solid pipeline
    Backend::PipelineHandle m_hSolidPipeline;

    // Debug normals pipeline
    // Backend::PipelineHandle m_hNormalsPipeline;

    // App resource
    Backend::ShaderHandle m_hSolidVertexShader;
    Backend::ShaderHandle m_hSolidPixelShader;

    /*
    Backend::ShaderHandle m_hNormalsVertexShader;
    Backend::ShaderHandle m_hNormalsPixelShader;
    Backend::ShaderHandle m_hNormalsGeometryShader;
    */

    Backend::ResourceHandle m_hVertexBuffer;
    Backend::ResourceHandle m_hIndexBuffer;
    Backend::ResourceHandle m_hUploadBuffer;
    Backend::MemoryHandle   m_hVertexIndexBufferMemory;
    Backend::MemoryHandle   m_hUploadBufferMemory;

    Backend::ResourceHandle     m_hDepthStencilBuffer;
    Backend::MemoryHandle       m_hDepthStencilBufferMemory;
    Backend::ResourceViewHandle m_hDepthStencilBufferView;

    Backend::ResourceHandle     m_hConstantBuffer;
    Backend::MemoryHandle       m_hConstantBufferMemory;
    void*                       m_pConstantBufferHost;
    Backend::ResourceViewHandle m_hConstantBufferView;

    // Synchronization objects
    Backend::FenceHandle     m_hFences[ FRAME_COUNT ];
    Backend::SemaphoreHandle m_hFrameFinishedSemaphores[ FRAME_COUNT ];
    uint64_t                 m_fenceValues[ FRAME_COUNT ];
};