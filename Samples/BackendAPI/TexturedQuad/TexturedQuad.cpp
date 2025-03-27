#include "TexturedQuad.h"

#include "Application.h"
#include "Driver/Frontend/RenderSystem.h"
#include "Driver/Frontend/Shader/IShaderCompiler.h"
#include "Platform/Event/EventSystem.h"
#include "Platform/Window/Window.h"

TexturedQuad::TexturedQuad( APITypes APIType, uint32_t width, uint32_t height, const char* pName )
    : Sample( APIType, width, height, pName )
    , m_pWindow( nullptr )
    , m_pAPIDevice( nullptr )
    , m_pQueue( nullptr )
    , m_pSwapchain( nullptr )
    , m_hSamplerSetLayout()
    , m_hShaderResourceSetLayout()
    , m_hSamplerHeap()
    , m_hShaderResourceHeap()
    , m_hPipelineLayout()
    , m_hPipeline()
    , m_hCommandPools()
    , m_pCommandBuffers()
    , m_hRTVs()
    , m_backBuffers()
    , m_samplerOffset( BIGOS::MAX_UINT64 )
    , m_textureOffset( BIGOS::MAX_UINT64 )
    , m_frameNdx( 0 )
    , m_shaderSourceSize( 0 )
    , m_pShaderSource( nullptr )
    , m_hUploadBuffer()
    , m_hUploadBufferMemory()
    , m_hVertexShader()
    , m_hPixelShader()
    , m_hVertexBuffer()
    , m_hIndexBuffer()
    , m_hBufferMemory()
    , m_hTexture()
    , m_hTextureMemory()
    , m_hSampledTextureView()
{
    BIGOS::Memory::Set( &m_fenceValues, 0, sizeof( m_fenceValues ) );
}

BIGOS::RESULT TexturedQuad::OnInit()
{
    if( BGS_FAILED( InitDevice() ) )
    {
        return BIGOS::Results::FAIL;
    }

    if( BGS_FAILED( LoadAssets() ) )
    {
        return BIGOS::Results::FAIL;
    }

    if( BGS_FAILED( CreateApplicationResources() ) )
    {
        return BIGOS::Results::FAIL;
    }

    if( BGS_FAILED( CreateFrameResources() ) )
    {
        return BIGOS::Results::FAIL;
    }

    if( BGS_FAILED( CreatePipeline() ) )
    {
        return BIGOS::Results::FAIL;
    }

    if( BGS_FAILED( UploadResourceData() ) )
    {
        return BIGOS::Results::FAIL;
    }

    return BIGOS::Results::OK;
}

void TexturedQuad::OnUpdate()
{
    m_pWindow->Update();
}

void TexturedQuad::OnRender()
{
    BIGOS::Driver::Backend::FrameInfo frameInfo;
    m_pSwapchain->GetNextFrame( &frameInfo );
    const uint32_t bufferNdx = frameInfo.swapChainBackBufferIndex;

    BIGOS::Driver::Backend::WaitForFencesDesc waitDesc;
    waitDesc.fenceCount  = 1;
    waitDesc.waitAll     = BIGOS::Core::BGS_TRUE;
    waitDesc.pFences     = &m_hFences[ m_frameNdx ];
    waitDesc.pWaitValues = &m_fenceValues[ m_frameNdx ];
    if( BGS_FAILED( m_pAPIDevice->WaitForFences( waitDesc, UINT64_MAX ) ) )
    {
        return;
    }
    m_fenceValues[ m_frameNdx ]++;

    if( BGS_FAILED( m_pAPIDevice->ResetCommandPool( m_hCommandPools[ bufferNdx ] ) ) )
    {
        return;
    }

    // Render commands
    BIGOS::Driver::Backend::BeginCommandBufferDesc beginDesc;
    m_pCommandBuffers[ bufferNdx ]->Begin( beginDesc );

    BIGOS::Driver::Backend::TextureBarrierDesc preTexBarrier;
    preTexBarrier.srcStage     = BGS_FLAG( BIGOS::Driver::Backend::PipelineStageFlagBits::RENDER_TARGET );
    preTexBarrier.srcAccess    = BGS_FLAG( BIGOS::Driver::Backend::AccessFlagBits::GENERAL );
    preTexBarrier.srcLayout    = BIGOS::Driver::Backend::TextureLayouts::PRESENT;
    preTexBarrier.dstStage     = BGS_FLAG( BIGOS::Driver::Backend::PipelineStageFlagBits::RENDER_TARGET );
    preTexBarrier.dstAccess    = BGS_FLAG( BIGOS::Driver::Backend::AccessFlagBits::RENDER_TARGET );
    preTexBarrier.dstLayout    = BIGOS::Driver::Backend::TextureLayouts::RENDER_TARGET;
    preTexBarrier.hResouce     = m_backBuffers[ bufferNdx ].hBackBuffer;
    preTexBarrier.textureRange = { BGS_FLAG( TextureComponentFlagBits::COLOR ), 0, 1, 0, 1 };

    BIGOS::Driver::Backend::BarierDesc preBarrier;
    preBarrier.textureBarrierCount = 1;
    preBarrier.pTextureBarriers    = &preTexBarrier;
    preBarrier.globalBarrierCount  = 0;
    preBarrier.pGlobalBarriers     = nullptr;
    preBarrier.bufferBarrierCount  = 0;
    preBarrier.pBufferBarriers     = nullptr;

    m_pCommandBuffers[ bufferNdx ]->Barrier( preBarrier );

    BIGOS::Driver::Backend::BeginRenderingDesc renderingDesc;
    renderingDesc.colorRenderTargetCount   = 1;
    renderingDesc.phColorRenderTargetViews = &m_hRTVs[ bufferNdx ];
    renderingDesc.hDepthStencilTargetView  = BIGOS::Driver::Backend::ResourceViewHandle();
    renderingDesc.renderArea.offset        = { 0, 0 };
    renderingDesc.renderArea.size          = { m_width, m_height };
    m_pCommandBuffers[ bufferNdx ]->BeginRendering( renderingDesc );

    m_pCommandBuffers[ bufferNdx ]->SetPipeline( m_hPipeline, BIGOS::Driver::Backend::PipelineTypes::GRAPHICS );

    m_pCommandBuffers[ bufferNdx ]->SetPrimitiveTopology( BIGOS::Driver::Backend::PrimitiveTopologies::TRIANGLE_LIST );

    BIGOS::Driver::Backend::BindingHeapHandle bindingHeaps[] = {
        m_hShaderResourceHeap,
        m_hSamplerHeap,
    };
    m_pCommandBuffers[ bufferNdx ]->SetBindingHeaps( 2, bindingHeaps );

    BIGOS::Driver::Backend::SetBindingsDesc setBindingDesc;
    setBindingDesc.baseBindingOffset = m_textureOffset;
    setBindingDesc.heapNdx           = 0;
    setBindingDesc.setSpaceNdx       = 0;
    setBindingDesc.hPipelineLayout   = m_hPipelineLayout;
    setBindingDesc.type              = BIGOS::Driver::Backend::PipelineTypes::GRAPHICS;
    m_pCommandBuffers[ bufferNdx ]->SetBindings( setBindingDesc );

    setBindingDesc.baseBindingOffset = m_samplerOffset;
    setBindingDesc.heapNdx           = 1;
    setBindingDesc.setSpaceNdx       = 1;
    setBindingDesc.hPipelineLayout   = m_hPipelineLayout;
    setBindingDesc.type              = BIGOS::Driver::Backend::PipelineTypes::GRAPHICS;
    m_pCommandBuffers[ bufferNdx ]->SetBindings( setBindingDesc );

    BIGOS::Driver::Backend::ViewportDesc vp;
    vp.upperLeftX = 0.0f;
    vp.upperLeftY = 0.0f;
    vp.width      = static_cast<float>( m_width );
    vp.height     = static_cast<float>( m_height );
    vp.minDepth   = 0.0f;
    vp.maxDepth   = 1.0f;
    m_pCommandBuffers[ bufferNdx ]->SetViewports( 1, &vp );

    BIGOS::Driver::Backend::ScissorDesc sr;
    sr.upperLeftX = 0;
    sr.upperLeftY = 0;
    sr.width      = m_width;
    sr.height     = m_height;
    m_pCommandBuffers[ bufferNdx ]->SetScissors( 1, &sr );

    BIGOS::Driver::Backend::ColorValue clrColor;
    clrColor.r = 0.0f;
    clrColor.g = 0.0f;
    clrColor.b = 0.0f;
    clrColor.a = 0.0f;

    BIGOS::Core::Rect2D clrRect;
    clrRect.offset = { 0, 0 };
    clrRect.size   = { m_width, m_height };
    m_pCommandBuffers[ bufferNdx ]->ClearBoundColorRenderTarget( 0, clrColor, 1, &clrRect );

    BIGOS::Driver::Backend::VertexBufferDesc vbBindDesc;
    vbBindDesc.hVertexBuffer = m_hVertexBuffer;
    vbBindDesc.offset        = 0;
    vbBindDesc.size          = 4 * 6 * sizeof( float );
    vbBindDesc.elementStride = 6 * sizeof( float );
    m_pCommandBuffers[ bufferNdx ]->SetVertexBuffers( 0, 1, &vbBindDesc );

    BIGOS::Driver::Backend::IndexBufferDesc ibBindDesc;
    ibBindDesc.hIndexBuffer = m_hIndexBuffer;
    ibBindDesc.indexType    = BIGOS::Driver::Backend::IndexTypes::UINT16;
    ibBindDesc.offset       = 0;
    m_pCommandBuffers[ bufferNdx ]->SetIndexBuffer( ibBindDesc );

    BIGOS::Driver::Backend::DrawDesc drawDesc;
    drawDesc.indexCount    = 6;
    drawDesc.instanceCount = 1;
    drawDesc.firstIndex    = 0;
    drawDesc.firstInstance = 0;
    drawDesc.vertexOffset  = 0;

    m_pCommandBuffers[ bufferNdx ]->DrawIndexed( drawDesc );

    m_pCommandBuffers[ bufferNdx ]->EndRendering();

    BIGOS::Driver::Backend::TextureBarrierDesc postTexBarrier;
    postTexBarrier.srcStage     = BGS_FLAG( BIGOS::Driver::Backend::PipelineStageFlagBits::RENDER_TARGET );
    postTexBarrier.srcAccess    = BGS_FLAG( BIGOS::Driver::Backend::AccessFlagBits::RENDER_TARGET );
    postTexBarrier.srcLayout    = BIGOS::Driver::Backend::TextureLayouts::RENDER_TARGET;
    postTexBarrier.dstStage     = BGS_FLAG( BIGOS::Driver::Backend::PipelineStageFlagBits::RENDER_TARGET );
    postTexBarrier.dstAccess    = BGS_FLAG( BIGOS::Driver::Backend::AccessFlagBits::GENERAL );
    postTexBarrier.dstLayout    = BIGOS::Driver::Backend::TextureLayouts::PRESENT;
    postTexBarrier.hResouce     = m_backBuffers[ bufferNdx ].hBackBuffer;
    postTexBarrier.textureRange = { BGS_FLAG( TextureComponentFlagBits::COLOR ), 0, 1, 0, 1 };

    BIGOS::Driver::Backend::BarierDesc postBarrier;
    postBarrier.textureBarrierCount = 1;
    postBarrier.pTextureBarriers    = &postTexBarrier;
    postBarrier.globalBarrierCount  = 0;
    postBarrier.pGlobalBarriers     = nullptr;
    postBarrier.bufferBarrierCount  = 0;
    postBarrier.pBufferBarriers     = nullptr;

    m_pCommandBuffers[ bufferNdx ]->Barrier( postBarrier );

    m_pCommandBuffers[ bufferNdx ]->End();

    // Execute commands
    BIGOS::Driver::Backend::QueueSubmitDesc submitDesc;
    submitDesc.waitSemaphoreCount   = 1;
    submitDesc.phWaitSemaphores     = &frameInfo.hBackBufferAvailableSemaphore;
    submitDesc.waitFenceCount       = 0;
    submitDesc.phWaitFences         = nullptr;
    submitDesc.pWaitValues          = nullptr;
    submitDesc.commandBufferCount   = 1;
    submitDesc.ppCommandBuffers     = &m_pCommandBuffers[ bufferNdx ];
    submitDesc.signalSemaphoreCount = 1;
    submitDesc.phSignalSemaphores   = &m_hFrameFinishedSemaphores[ m_frameNdx ];
    submitDesc.signalFenceCount     = 1;
    submitDesc.phSignalFences       = &m_hFences[ m_frameNdx ];
    submitDesc.pSignalValues        = &m_fenceValues[ m_frameNdx ];
    if( BGS_FAILED( m_pQueue->Submit( submitDesc ) ) )
    {
        return;
    }

    // Presents frame
    BIGOS::Driver::Backend::SwapchainPresentDesc presentDesc;
    presentDesc.waitSemaphoreCount = 1;
    presentDesc.pWaitSemaphores    = &m_hFrameFinishedSemaphores[ m_frameNdx ];
    presentDesc.frameNdx           = bufferNdx;
    if( BGS_FAILED( m_pSwapchain->Present( presentDesc ) ) )
    {
        return;
    }

    m_frameNdx = ( m_frameNdx + 1 ) % FRAME_COUNT;
}

void TexturedQuad::OnDestroy()
{
    BIGOS::Driver::Backend::WaitForFencesDesc waitDesc;
    waitDesc.fenceCount  = FRAME_COUNT;
    waitDesc.waitAll     = BIGOS::Core::BGS_TRUE;
    waitDesc.pFences     = m_hFences;
    waitDesc.pWaitValues = m_fenceValues;
    if( BGS_FAILED( m_pAPIDevice->WaitForFences( waitDesc, UINT64_MAX ) ) )
    {
        return;
    }
    m_pAPIDevice->DestroyResource( &m_hUploadBuffer );
    m_pAPIDevice->FreeMemory( &m_hUploadBufferMemory );
    m_pAPIDevice->DestroySampler( &m_hSampler );
    m_pAPIDevice->DestroyResourceView( &m_hSampledTextureView );
    m_pAPIDevice->DestroyResource( &m_hTexture );
    m_pAPIDevice->FreeMemory( &m_hTextureMemory );
    m_pAPIDevice->DestroyResource( &m_hVertexBuffer );
    m_pAPIDevice->DestroyResource( &m_hIndexBuffer );
    m_pAPIDevice->FreeMemory( &m_hBufferMemory );
    m_pAPIDevice->DestroyShader( &m_hVertexShader );
    m_pAPIDevice->DestroyShader( &m_hPixelShader );
    m_pAPIDevice->DestroyBindingHeap( &m_hSamplerHeap );
    m_pAPIDevice->DestroyBindingHeap( &m_hShaderResourceHeap );
    m_pAPIDevice->DestroyPipeline( &m_hPipeline );
    m_pAPIDevice->DestroyPipelineLayout( &m_hPipelineLayout );
    m_pAPIDevice->DestroyBindingSetLayout( &m_hSamplerSetLayout );
    m_pAPIDevice->DestroyBindingSetLayout( &m_hShaderResourceSetLayout );
    for( uint32_t ndx = 0; ndx < BACK_BUFFER_COUNT; ++ndx )
    {
        m_pAPIDevice->DestroyCommandBuffer( &m_pCommandBuffers[ ndx ] );
        m_pAPIDevice->DestroyCommandPool( &m_hCommandPools[ ndx ] );
        m_pAPIDevice->DestroyResourceView( &m_hRTVs[ ndx ] );
    }
    for( uint32_t ndx = 0; ndx < FRAME_COUNT; ++ndx )
    {
        m_pAPIDevice->DestroyFence( &m_hFences[ ndx ] );
        m_pAPIDevice->DestroySemaphore( &m_hFrameFinishedSemaphores[ ndx ] );
    }
    m_pAPIDevice->DestroySwapchain( &m_pSwapchain );
    m_pAPIDevice->DestroyQueue( &m_pQueue );
}

BIGOS::RESULT TexturedQuad::InitDevice()
{
    Application::GetFramework()->GetEventSystem().Subscribe( &m_windowCloseHandler );

    BIGOS::Platform::WindowDesc wndDesc;
    wndDesc.pTitle    = m_pName;
    wndDesc.mode      = BIGOS::Platform::WindowModes::WINDOW;
    wndDesc.xPosition = 100;
    wndDesc.yPosition = 100;
    wndDesc.width     = m_width;
    wndDesc.height    = m_height;
    if( BGS_FAILED( Application::GetFramework()->GetWindowSystem().CreateWindow( wndDesc, &m_pWindow ) ) )
    {
        return BIGOS::Results::FAIL;
    }
    m_pWindow->Show();

    // Creating device
    BIGOS::Driver::Frontend::DriverDesc driverDesc;
    driverDesc.apiType = m_APIType;
    driverDesc.debug   = true;
    if( BGS_FAILED( Application::GetFramework()->GetRenderSystem().InitializeDriver( driverDesc ) ) )
    {
        return BIGOS::Results::FAIL;
    }
    m_pAPIDevice = Application::GetFramework()->GetRenderSystem().GetDevice();

    // Synchronization objects
    for( uint32_t ndx = 0; ndx < FRAME_COUNT; ++ndx )
    {
        BIGOS::Driver::Backend::SemaphoreDesc semaphoreDesc;
        if( BGS_FAILED( m_pAPIDevice->CreateSemaphore( semaphoreDesc, &m_hFrameFinishedSemaphores[ ndx ] ) ) )
        {
            return BIGOS::Results::FAIL;
        }

        BIGOS::Driver::Backend::FenceDesc fenceDesc;
        fenceDesc.initialValue = m_fenceValues[ ndx ];
        if( BGS_FAILED( m_pAPIDevice->CreateFence( fenceDesc, &m_hFences[ ndx ] ) ) )
        {
            return BIGOS::Results::FAIL;
        }
    }

    // Queue and swapchain
    BIGOS::Driver::Backend::QueueDesc queueDesc;
    queueDesc.type     = BIGOS::Driver::Backend::QueueTypes::GRAPHICS;
    queueDesc.priority = BIGOS::Driver::Backend::QueuePriorityTypes::NORMAL;
    if( BGS_FAILED( m_pAPIDevice->CreateQueue( queueDesc, &m_pQueue ) ) )
    {
        return BIGOS::Results::OK;
    }

    BIGOS::Driver::Backend::SwapchainDesc swapDesc;
    swapDesc.pQueue          = m_pQueue;
    swapDesc.pWindow         = m_pWindow;
    swapDesc.vSync           = BIGOS::Core::BGS_TRUE;
    swapDesc.backBufferCount = BACK_BUFFER_COUNT;
    swapDesc.format          = BIGOS::Driver::Backend::Formats::B8G8R8A8_UNORM;

    if( BGS_FAILED( m_pAPIDevice->CreateSwapchain( swapDesc, &m_pSwapchain ) ) )
    {
        return BIGOS::Results::OK;
    }

    return BIGOS::Results::OK;
}

BIGOS::RESULT TexturedQuad::LoadAssets()
{
    // Load shaders
    if( BGS_FAILED( readDataFromFile( "../Samples/BackendAPI/TexturedQuad/shader.hlsl", &m_pShaderSource, &m_shaderSourceSize ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Generating texture data
    void*    pTextureData = nullptr;
    uint32_t textureSize  = 0;
    if( BGS_FAILED( GenerateCheckerboardData( 256, 256, &pTextureData, &textureSize ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Creating upload buffer
    // 512 is copy alignment for textures (native API limitation)
    const uint32_t                       uploadBufferSize = 512 + 256 * 256 * 4;
    BIGOS::Driver::Backend::ResourceDesc uploadBufferDesc;
    uploadBufferDesc.size.width      = uploadBufferSize;
    uploadBufferDesc.size.height     = 1;
    uploadBufferDesc.size.depth      = 1;
    uploadBufferDesc.format          = BIGOS::Driver::Backend::Formats::UNKNOWN;
    uploadBufferDesc.mipLevelCount   = 1;
    uploadBufferDesc.arrayLayerCount = 1;
    uploadBufferDesc.resourceLayout  = BIGOS::Driver::Backend::ResourceLayouts::LINEAR;
    uploadBufferDesc.resourceType    = BIGOS::Driver::Backend::ResourceTypes::BUFFER;
    uploadBufferDesc.sampleCount     = BIGOS::Driver::Backend::SampleCount::COUNT_1;
    uploadBufferDesc.sharingMode     = BIGOS::Driver::Backend::ResourceSharingModes::EXCLUSIVE_ACCESS;
    uploadBufferDesc.resourceUsage   = BGS_FLAG( BIGOS::Driver::Backend::ResourceUsageFlagBits::TRANSFER_SRC );
    if( BGS_FAILED( m_pAPIDevice->CreateResource( uploadBufferDesc, &m_hUploadBuffer ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Allocating memory for upload buffer
    BIGOS::Driver::Backend::ResourceAllocationInfo uploadBufferAllocInfo;
    m_pAPIDevice->GetResourceAllocationInfo( m_hUploadBuffer, &uploadBufferAllocInfo );

    BIGOS::Driver::Backend::AllocateMemoryDesc uploadAllocDesc;
    uploadAllocDesc.size      = uploadBufferAllocInfo.size;
    uploadAllocDesc.alignment = uploadBufferAllocInfo.alignment;
    uploadAllocDesc.heapType  = BIGOS::Driver::Backend::MemoryHeapTypes::UPLOAD;
    uploadAllocDesc.heapUsage = BIGOS::Driver::Backend::MemoryHeapUsages::BUFFERS;
    if( BGS_FAILED( m_pAPIDevice->AllocateMemory( uploadAllocDesc, &m_hUploadBufferMemory ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Binding upload buffer
    BIGOS::Driver::Backend::BindResourceMemoryDesc bindMemDesc;
    bindMemDesc.hMemory      = m_hUploadBufferMemory;
    bindMemDesc.hResource    = m_hUploadBuffer;
    bindMemDesc.memoryOffset = 0;
    if( BGS_FAILED( m_pAPIDevice->BindResourceMemory( bindMemDesc ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    void*                                   pHostAccess = nullptr;
    BIGOS::Driver::Backend::MapResourceDesc mapUpload;
    mapUpload.hResource          = m_hUploadBuffer;
    mapUpload.bufferRange.size   = uploadBufferSize;
    mapUpload.bufferRange.offset = 0;
    if( BGS_FAILED( m_pAPIDevice->MapResource( mapUpload, &pHostAccess ) ) )
    {
        return BIGOS::Results::FAIL;
    }
    BIGOS::byte_t* pCurr = static_cast<BIGOS::byte_t*>( pHostAccess );

    BIGOS::Memory::Copy( VERTEX_DATA, sizeof( VERTEX_DATA ), pCurr, sizeof( VERTEX_DATA ) );
    pCurr += sizeof( VERTEX_DATA );
    BIGOS::Memory::Copy( INDEX_DATA, sizeof( INDEX_DATA ), pCurr, sizeof( INDEX_DATA ) );
    pCurr = static_cast<BIGOS::byte_t*>( pHostAccess ) + 512;
    BIGOS::Memory::Copy( pTextureData, textureSize, pCurr, textureSize );

    if( BGS_FAILED( m_pAPIDevice->UnmapResource( mapUpload ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    delete[] pTextureData;

    return BIGOS::RESULT();
}

BIGOS::RESULT TexturedQuad::CreateFrameResources()
{
    m_backBuffers = m_pSwapchain->GetBackBuffers();

    BIGOS::Driver::Backend::TextureRangeDesc rtRange;
    rtRange.components      = BGS_FLAG( BIGOS::Driver::Backend::TextureComponentFlagBits::COLOR );
    rtRange.mipLevel        = 0;
    rtRange.mipLevelCount   = 1;
    rtRange.arrayLayer      = 0;
    rtRange.arrayLayerCount = 1;

    for( uint32_t ndx = 0; ndx < BACK_BUFFER_COUNT; ++ndx )
    {
        BIGOS::Driver::Backend::TextureViewDesc rtvDesc;
        rtvDesc.usage                 = BGS_FLAG( BIGOS::Driver::Backend::ResourceViewUsageFlagBits::COLOR_RENDER_TARGET );
        rtvDesc.format                = BIGOS::Driver::Backend::Formats::B8G8R8A8_UNORM;
        rtvDesc.textureType           = BIGOS::Driver::Backend::TextureTypes::TEXTURE_2D;
        rtvDesc.range.components      = BGS_FLAG( BIGOS::Driver::Backend::TextureComponentFlagBits::COLOR );
        rtvDesc.range.mipLevel        = 0;
        rtvDesc.range.mipLevelCount   = 1;
        rtvDesc.range.arrayLayer      = 0;
        rtvDesc.range.arrayLayerCount = 1;
        rtvDesc.hResource             = m_backBuffers[ ndx ].hBackBuffer;

        if( BGS_FAILED( m_pAPIDevice->CreateResourceView( rtvDesc, &m_hRTVs[ ndx ] ) ) )
        {
            return BIGOS::Results::FAIL;
        }

        BIGOS::Driver::Backend::CommandPoolDesc cmdPoolDesc;
        cmdPoolDesc.pQueue = m_pQueue;
        if( BGS_FAILED( m_pAPIDevice->CreateCommandPool( cmdPoolDesc, &m_hCommandPools[ ndx ] ) ) )
        {
            return BIGOS::Results::FAIL;
        }

        BIGOS::Driver::Backend::CommandBufferDesc cmdBufferDesc;
        cmdBufferDesc.hCommandPool = m_hCommandPools[ ndx ];
        cmdBufferDesc.level        = BIGOS::Driver::Backend::CommandBufferLevels::PRIMARY;
        cmdBufferDesc.pQueue       = m_pQueue;
        if( BGS_FAILED( m_pAPIDevice->CreateCommandBuffer( cmdBufferDesc, &m_pCommandBuffers[ ndx ] ) ) )
        {
            return BIGOS::Results::FAIL;
        }
    }

    return BIGOS::Results::OK;
}

BIGOS::RESULT TexturedQuad::CreateApplicationResources()
{
    // Creating vertex index buffer
    BIGOS::Driver::Backend::ResourceDesc vbDesc;
    vbDesc.size.width      = 4 /* quad */ * 6 /* position (4) + texcoord (2) */ * sizeof( float );
    vbDesc.size.height     = 1;
    vbDesc.size.depth      = 1;
    vbDesc.mipLevelCount   = 1;
    vbDesc.arrayLayerCount = 1;
    vbDesc.format          = BIGOS::Driver::Backend::Formats::UNKNOWN;
    vbDesc.resourceLayout  = BIGOS::Driver::Backend::ResourceLayouts::LINEAR;
    vbDesc.resourceType    = BIGOS::Driver::Backend::ResourceTypes::BUFFER;
    vbDesc.sampleCount     = BIGOS::Driver::Backend::SampleCount::COUNT_1;
    vbDesc.sharingMode     = BIGOS::Driver::Backend::ResourceSharingModes::EXCLUSIVE_ACCESS;
    vbDesc.resourceUsage   = BGS_FLAG( BIGOS::Driver::Backend::ResourceUsageFlagBits::VERTEX_BUFFER ) |
                           BGS_FLAG( BIGOS::Driver::Backend::ResourceUsageFlagBits::TRANSFER_DST );
    if( BGS_FAILED( m_pAPIDevice->CreateResource( vbDesc, &m_hVertexBuffer ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Driver::Backend::ResourceDesc ibDesc;
    ibDesc.size.width      = 6 /* two triangles */ * sizeof( uint16_t );
    ibDesc.size.height     = 1;
    ibDesc.size.depth      = 1;
    ibDesc.mipLevelCount   = 1;
    ibDesc.arrayLayerCount = 1;
    ibDesc.format          = BIGOS::Driver::Backend::Formats::UNKNOWN;
    ibDesc.resourceLayout  = BIGOS::Driver::Backend::ResourceLayouts::LINEAR;
    ibDesc.resourceType    = BIGOS::Driver::Backend::ResourceTypes::BUFFER;
    ibDesc.sampleCount     = BIGOS::Driver::Backend::SampleCount::COUNT_1;
    ibDesc.sharingMode     = BIGOS::Driver::Backend::ResourceSharingModes::EXCLUSIVE_ACCESS;
    ibDesc.resourceUsage   = BGS_FLAG( BIGOS::Driver::Backend::ResourceUsageFlagBits::INDEX_BUFFER ) |
                           BGS_FLAG( BIGOS::Driver::Backend::ResourceUsageFlagBits::TRANSFER_DST );
    if( BGS_FAILED( m_pAPIDevice->CreateResource( ibDesc, &m_hIndexBuffer ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Allocating memory for buffers
    BIGOS::Driver::Backend::ResourceAllocationInfo vbAllocInfo;
    m_pAPIDevice->GetResourceAllocationInfo( m_hVertexBuffer, &vbAllocInfo );
    BIGOS::Driver::Backend::ResourceAllocationInfo ibAllocInfo;
    m_pAPIDevice->GetResourceAllocationInfo( m_hIndexBuffer, &ibAllocInfo );

    BIGOS::Driver::Backend::AllocateMemoryDesc bufferAllocDesc;
    bufferAllocDesc.size      = vbAllocInfo.size + ibAllocInfo.size;
    bufferAllocDesc.alignment = vbAllocInfo.alignment;
    bufferAllocDesc.heapType  = BIGOS::Driver::Backend::MemoryHeapTypes::DEFAULT;
    bufferAllocDesc.heapUsage = BIGOS::Driver::Backend::MemoryHeapUsages::BUFFERS;
    if( BGS_FAILED( m_pAPIDevice->AllocateMemory( bufferAllocDesc, &m_hBufferMemory ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Binding vertex and index buffer
    BIGOS::Driver::Backend::BindResourceMemoryDesc bindMemDesc;
    bindMemDesc.hMemory      = m_hBufferMemory;
    bindMemDesc.hResource    = m_hVertexBuffer;
    bindMemDesc.memoryOffset = 0;
    if( BGS_FAILED( m_pAPIDevice->BindResourceMemory( bindMemDesc ) ) )
    {
        return BIGOS::Results::FAIL;
    }
    bindMemDesc.hResource    = m_hIndexBuffer;
    bindMemDesc.memoryOffset = vbAllocInfo.size;
    if( BGS_FAILED( m_pAPIDevice->BindResourceMemory( bindMemDesc ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Creating texture
    BIGOS::Driver::Backend::ResourceDesc textureDesc;
    textureDesc.size.width      = 256;
    textureDesc.size.height     = 256;
    textureDesc.size.depth      = 1;
    textureDesc.mipLevelCount   = 1;
    textureDesc.arrayLayerCount = 1;
    textureDesc.format          = BIGOS::Driver::Backend::Formats::R8G8B8A8_UNORM;
    textureDesc.resourceLayout  = BIGOS::Driver::Backend::ResourceLayouts::LINEAR;
    textureDesc.resourceType    = BIGOS::Driver::Backend::ResourceTypes::TEXTURE_2D;
    textureDesc.sampleCount     = BIGOS::Driver::Backend::SampleCount::COUNT_1;
    textureDesc.sharingMode     = BIGOS::Driver::Backend::ResourceSharingModes::EXCLUSIVE_ACCESS;
    textureDesc.resourceUsage   = BGS_FLAG( BIGOS::Driver::Backend::ResourceUsageFlagBits::SAMPLED_TEXTURE ) |
                                BGS_FLAG( BIGOS::Driver::Backend::ResourceUsageFlagBits::TRANSFER_DST );
    if( BGS_FAILED( m_pAPIDevice->CreateResource( textureDesc, &m_hTexture ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Allocating memory for textures
    BIGOS::Driver::Backend::ResourceAllocationInfo texAllocInfo;
    m_pAPIDevice->GetResourceAllocationInfo( m_hTexture, &texAllocInfo );

    BIGOS::Driver::Backend::AllocateMemoryDesc texAllocDesc;
    texAllocDesc.size      = texAllocInfo.size;
    texAllocDesc.alignment = texAllocInfo.alignment;
    texAllocDesc.heapType  = BIGOS::Driver::Backend::MemoryHeapTypes::DEFAULT;
    texAllocDesc.heapUsage = BIGOS::Driver::Backend::MemoryHeapUsages::TEXTURES;
    if( BGS_FAILED( m_pAPIDevice->AllocateMemory( texAllocDesc, &m_hTextureMemory ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Binding texture
    bindMemDesc.hMemory      = m_hTextureMemory;
    bindMemDesc.hResource    = m_hTexture;
    bindMemDesc.memoryOffset = 0;
    if( BGS_FAILED( m_pAPIDevice->BindResourceMemory( bindMemDesc ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Creating sampled texture view
    BIGOS::Driver::Backend::TextureViewDesc texViewDesc;
    texViewDesc.usage                 = BGS_FLAG( BIGOS::Driver::Backend::ResourceViewUsageFlagBits::SAMPLED_TEXTURE );
    texViewDesc.format                = BIGOS::Driver::Backend::Formats::R8G8B8A8_UNORM;
    texViewDesc.textureType           = BIGOS::Driver::Backend::TextureTypes::TEXTURE_2D;
    texViewDesc.range.components      = BGS_FLAG( BIGOS::Driver::Backend::TextureComponentFlagBits::COLOR );
    texViewDesc.range.mipLevel        = 0;
    texViewDesc.range.mipLevelCount   = 1;
    texViewDesc.range.arrayLayer      = 0;
    texViewDesc.range.arrayLayerCount = 1;
    texViewDesc.hResource             = m_hTexture;

    if( BGS_FAILED( m_pAPIDevice->CreateResourceView( texViewDesc, &m_hSampledTextureView ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Creating sampler
    BIGOS::Driver::Backend::SamplerDesc samplerDesc;
    samplerDesc.type                = BIGOS::Driver::Backend::SamplerTypes::NORMAL;
    samplerDesc.minFilter           = BIGOS::Driver::Backend::FilterTypes::LINEAR;
    samplerDesc.magFilter           = BIGOS::Driver::Backend::FilterTypes::LINEAR;
    samplerDesc.mipMapFilter        = BIGOS::Driver::Backend::FilterTypes::LINEAR;
    samplerDesc.addressU            = BIGOS::Driver::Backend::TextureAddressModes::CLAMP_TO_BORDER;
    samplerDesc.addressV            = BIGOS::Driver::Backend::TextureAddressModes::CLAMP_TO_BORDER;
    samplerDesc.addressW            = BIGOS::Driver::Backend::TextureAddressModes::CLAMP_TO_BORDER;
    samplerDesc.anisotropyEnable    = BIGOS::BGS_FALSE;
    samplerDesc.compareEnable       = BIGOS::BGS_FALSE;
    samplerDesc.compareOperation    = BIGOS::Driver::Backend::CompareOperationTypes::NEVER;
    samplerDesc.reductionMode       = BIGOS::Driver::Backend::SamplerReductionModes::WEIGHTED_AVERAGE;
    samplerDesc.minLod              = 0;
    samplerDesc.maxLod              = 0;
    samplerDesc.mipLodBias          = 0;
    samplerDesc.customBorderColor.r = 1.0f;
    samplerDesc.customBorderColor.g = 1.0f;
    samplerDesc.customBorderColor.b = 1.0f;
    samplerDesc.customBorderColor.a = 1.0f;

    if( BGS_FAILED( m_pAPIDevice->CreateSampler( samplerDesc, &m_hSampler ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Creating shaders
    BIGOS::Driver::Frontend::ShaderCompilerOutput* pVSBlob = nullptr;
    BIGOS::Driver::Frontend::ShaderCompilerOutput* pPSBlob = nullptr;
    BIGOS::Driver::Frontend::CompileShaderDesc     shaderDesc;
    shaderDesc.argCount           = 0;
    shaderDesc.ppArgs             = nullptr;
    shaderDesc.outputFormat       = m_APIType == BIGOS::Driver::Backend::APITypes::D3D12 ? BIGOS::Driver::Frontend::ShaderFormats::DXIL
                                                                                         : BIGOS::Driver::Frontend::ShaderFormats::SPIRV;
    shaderDesc.model              = BIGOS::Driver::Frontend::ShaderModels::SHADER_MODEL_6_5;
    shaderDesc.source.pSourceCode = m_pShaderSource;
    shaderDesc.source.sourceSize  = m_shaderSourceSize;
    shaderDesc.type               = BIGOS::Driver::Backend::ShaderTypes::VERTEX;
    shaderDesc.pEntryPoint        = "VSMain";
    if( BGS_FAILED( Application::GetFramework()->GetRenderSystem().GetDefaultCompiler()->Compile( shaderDesc, &pVSBlob ) ) )
    {
        return BIGOS::Results::FAIL;
    }
    shaderDesc.type        = BIGOS::Driver::Backend::ShaderTypes::PIXEL;
    shaderDesc.pEntryPoint = "PSMain";
    if( BGS_FAILED( Application::GetFramework()->GetRenderSystem().GetDefaultCompiler()->Compile( shaderDesc, &pPSBlob ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Driver::Backend::ShaderDesc shDesc;
    shDesc.pByteCode = pVSBlob->pByteCode;
    shDesc.codeSize  = pVSBlob->byteCodeSize;
    if( BGS_FAILED( m_pAPIDevice->CreateShader( shDesc, &m_hVertexShader ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    shDesc.pByteCode = pPSBlob->pByteCode;
    shDesc.codeSize  = pPSBlob->byteCodeSize;
    if( BGS_FAILED( m_pAPIDevice->CreateShader( shDesc, &m_hPixelShader ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    Application::GetFramework()->GetRenderSystem().GetDefaultCompiler()->DestroyOutput( &pVSBlob );
    Application::GetFramework()->GetRenderSystem().GetDefaultCompiler()->DestroyOutput( &pPSBlob );

    delete[] m_pShaderSource;

    return BIGOS::Results::OK;
}

BIGOS::RESULT TexturedQuad::UploadResourceData()
{
    BGS_ASSERT( m_pQueue );

    BIGOS::Driver::Backend::CommandPoolHandle hCpyPool;
    BIGOS::Driver::Backend::CommandPoolDesc   cpyPoolDesc;
    cpyPoolDesc.pQueue = m_pQueue;
    if( BGS_FAILED( m_pAPIDevice->CreateCommandPool( cpyPoolDesc, &hCpyPool ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Driver::Backend::ICommandBuffer*   pCpyCmdBuffer = nullptr;
    BIGOS::Driver::Backend::CommandBufferDesc cpyCmdBufferDesc;
    cpyCmdBufferDesc.hCommandPool = hCpyPool;
    cpyCmdBufferDesc.level        = BIGOS::Driver::Backend::CommandBufferLevels::PRIMARY;
    cpyCmdBufferDesc.pQueue       = m_pQueue;
    if( BGS_FAILED( m_pAPIDevice->CreateCommandBuffer( cpyCmdBufferDesc, &pCpyCmdBuffer ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    uint64_t                            fenceWaitValue = 1;
    BIGOS::Driver::Backend::FenceHandle hUploadFence;
    BIGOS::Driver::Backend::FenceDesc   uploadFenceDesc;
    uploadFenceDesc.initialValue = 0;
    if( BGS_FAILED( m_pAPIDevice->CreateFence( uploadFenceDesc, &hUploadFence ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Driver::Backend::BeginCommandBufferDesc beginCmdBuffDesc;
    pCpyCmdBuffer->Begin( beginCmdBuffDesc );

    BIGOS::Driver::Backend::CopyBufferDesc cpyBuffDesc;
    cpyBuffDesc.hSrcBuffer = m_hUploadBuffer;
    cpyBuffDesc.hDstBuffer = m_hVertexBuffer;
    cpyBuffDesc.srcOffset  = 0;
    cpyBuffDesc.dstOffset  = 0;
    cpyBuffDesc.size       = 4 * 6 * sizeof( float );
    pCpyCmdBuffer->CopyBuffer( cpyBuffDesc );

    cpyBuffDesc.hDstBuffer = m_hIndexBuffer;
    cpyBuffDesc.srcOffset  = 4 * 6 * sizeof( float );
    cpyBuffDesc.size       = 6 * sizeof( uint16_t );
    pCpyCmdBuffer->CopyBuffer( cpyBuffDesc );

    BIGOS::Driver::Backend::TextureBarrierDesc preCpyBarrier;
    preCpyBarrier.srcStage     = BGS_FLAG( BIGOS::Driver::Backend::PipelineStageFlagBits::NONE );
    preCpyBarrier.srcAccess    = BGS_FLAG( BIGOS::Driver::Backend::AccessFlagBits::NONE );
    preCpyBarrier.srcLayout    = BIGOS::Driver::Backend::TextureLayouts::UNDEFINED;
    preCpyBarrier.dstStage     = BGS_FLAG( BIGOS::Driver::Backend::PipelineStageFlagBits::TRANSFER );
    preCpyBarrier.dstAccess    = BGS_FLAG( BIGOS::Driver::Backend::AccessFlagBits::TRANSFER_DST );
    preCpyBarrier.dstLayout    = BIGOS::Driver::Backend::TextureLayouts::TRANSFER_DST;
    preCpyBarrier.hResouce     = m_hTexture;
    preCpyBarrier.textureRange = { BGS_FLAG( TextureComponentFlagBits::COLOR ), 0, 1, 0, 1 };

    BIGOS::Driver::Backend::BarierDesc barrier;
    barrier.textureBarrierCount = 1;
    barrier.pTextureBarriers    = &preCpyBarrier;
    barrier.globalBarrierCount  = 0;
    barrier.pGlobalBarriers     = nullptr;
    barrier.bufferBarrierCount  = 0;
    barrier.pBufferBarriers     = nullptr;
    pCpyCmdBuffer->Barrier( barrier );

    BIGOS::Driver::Backend::CopyBufferTextureDesc cpyBuffTexDesc;
    cpyBuffTexDesc.hBuffer       = m_hUploadBuffer;
    cpyBuffTexDesc.hTexture      = m_hTexture;
    cpyBuffTexDesc.bufferOffset  = 512;
    cpyBuffTexDesc.textureOffset = { 0, 0, 0 };
    cpyBuffTexDesc.size          = { 256, 256, 1 };
    cpyBuffTexDesc.textureFormat = BIGOS::Driver::Backend::Formats::R8G8B8A8_UNORM;
    cpyBuffTexDesc.textureRange  = { BGS_FLAG( TextureComponentFlagBits::COLOR ), 0, 1, 0, 1 };
    pCpyCmdBuffer->CopyBuferToTexture( cpyBuffTexDesc );

    BIGOS::Driver::Backend::TextureBarrierDesc postCpyBarrier;
    postCpyBarrier.srcStage     = BGS_FLAG( BIGOS::Driver::Backend::PipelineStageFlagBits::TRANSFER );
    postCpyBarrier.srcAccess    = BGS_FLAG( BIGOS::Driver::Backend::AccessFlagBits::TRANSFER_DST );
    postCpyBarrier.srcLayout    = BIGOS::Driver::Backend::TextureLayouts::TRANSFER_DST;
    postCpyBarrier.dstStage     = BGS_FLAG( BIGOS::Driver::Backend::PipelineStageFlagBits::PIXEL_SHADING );
    postCpyBarrier.dstAccess    = BGS_FLAG( BIGOS::Driver::Backend::AccessFlagBits::SHADER_READ_ONLY );
    postCpyBarrier.dstLayout    = BIGOS::Driver::Backend::TextureLayouts::SHADER_READ_ONLY;
    postCpyBarrier.hResouce     = m_hTexture;
    postCpyBarrier.textureRange = { BGS_FLAG( TextureComponentFlagBits::COLOR ), 0, 1, 0, 1 };

    barrier.pTextureBarriers = &postCpyBarrier;
    pCpyCmdBuffer->Barrier( barrier );

    pCpyCmdBuffer->End();

    BIGOS::Driver::Backend::QueueSubmitDesc submitDesc;
    submitDesc.waitSemaphoreCount   = 0;
    submitDesc.phWaitSemaphores     = nullptr;
    submitDesc.waitFenceCount       = 0;
    submitDesc.phWaitFences         = nullptr;
    submitDesc.pWaitValues          = nullptr;
    submitDesc.commandBufferCount   = 1;
    submitDesc.ppCommandBuffers     = &pCpyCmdBuffer;
    submitDesc.signalSemaphoreCount = 0;
    submitDesc.phSignalSemaphores   = nullptr;
    submitDesc.signalFenceCount     = 1;
    submitDesc.phSignalFences       = &hUploadFence;
    submitDesc.pSignalValues        = &fenceWaitValue;
    if( BGS_FAILED( m_pQueue->Submit( submitDesc ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Driver::Backend::WaitForFencesDesc waitForFenceDesc;
    waitForFenceDesc.fenceCount  = 1;
    waitForFenceDesc.pFences     = &hUploadFence;
    waitForFenceDesc.pWaitValues = &fenceWaitValue;
    waitForFenceDesc.waitAll     = BIGOS::BGS_FALSE;
    if( BGS_FAILED( m_pAPIDevice->WaitForFences( waitForFenceDesc, UINT64_MAX ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    m_pAPIDevice->DestroyFence( &hUploadFence );
    m_pAPIDevice->DestroyCommandBuffer( &pCpyCmdBuffer );
    m_pAPIDevice->DestroyCommandPool( &hCpyPool );

    return BIGOS::Results::OK;
}

BIGOS::RESULT TexturedQuad::CreatePipeline()
{
    // Binding heap layout
    BIGOS::Driver::Backend::BindingRangeDesc bindingRange;
    bindingRange.baseBindingSlot    = 0;
    bindingRange.baseShaderRegister = 0;
    bindingRange.bindingCount       = 1;
    bindingRange.type               = BIGOS::Driver::Backend::BindingTypes::SAMPLED_TEXTURE;
    BIGOS::Driver::Backend::BindingSetLayoutDesc bindingLayoutDesc;
    bindingLayoutDesc.visibility        = BIGOS::Driver::Backend::ShaderVisibilities::ALL;
    bindingLayoutDesc.bindingRangeCount = 1;
    bindingLayoutDesc.pBindingRanges    = &bindingRange;
    if( BGS_FAILED( m_pAPIDevice->CreateBindingSetLayout( bindingLayoutDesc, &m_hShaderResourceSetLayout ) ) )
    {
        return BIGOS::Results::FAIL;
    }
    bindingRange.baseBindingSlot    = 1;
    bindingRange.baseShaderRegister = 1;
    bindingRange.bindingCount       = 1;
    bindingRange.type               = BIGOS::Driver::Backend::BindingTypes::SAMPLER;
    if( BGS_FAILED( m_pAPIDevice->CreateBindingSetLayout( bindingLayoutDesc, &m_hSamplerSetLayout ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Binding heaps
    BIGOS::Driver::Backend::BindingHeapDesc bindingHeapDesc;
    bindingHeapDesc.bindingCount = 1;
    bindingHeapDesc.type         = BIGOS::Driver::Backend::BindingHeapTypes::SAMPLER;
    if( BGS_FAILED( m_pAPIDevice->CreateBindingHeap( bindingHeapDesc, &m_hSamplerHeap ) ) )
    {
        return BIGOS::Results::FAIL;
    }
    bindingHeapDesc.type = BIGOS::Driver::Backend::BindingHeapTypes::SHADER_RESOURCE;
    if( BGS_FAILED( m_pAPIDevice->CreateBindingHeap( bindingHeapDesc, &m_hShaderResourceHeap ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Write bindings to heaps
    BIGOS::Driver::Backend::GetBindingOffsetDesc getAddressDesc;
    getAddressDesc.hBindingSetLayout = m_hSamplerSetLayout;
    getAddressDesc.bindingNdx        = 0;
    m_pAPIDevice->GetBindingOffset( getAddressDesc, &m_samplerOffset );
    getAddressDesc.hBindingSetLayout = m_hShaderResourceSetLayout;
    m_pAPIDevice->GetBindingOffset( getAddressDesc, &m_textureOffset );

    BIGOS::Driver::Backend::WriteBindingDesc writeBindingDesc;
    writeBindingDesc.bindingType   = BindingTypes::SAMPLED_TEXTURE;
    writeBindingDesc.hDstHeap      = m_hShaderResourceHeap;
    writeBindingDesc.dstOffset     = m_textureOffset;
    writeBindingDesc.hResourceView = m_hSampledTextureView;
    m_pAPIDevice->WriteBinding( writeBindingDesc );

    writeBindingDesc.bindingType = BindingTypes::SAMPLER;
    writeBindingDesc.hDstHeap    = m_hSamplerHeap;
    writeBindingDesc.dstOffset   = m_samplerOffset;
    writeBindingDesc.hSampler    = m_hSampler;
    m_pAPIDevice->WriteBinding( writeBindingDesc );

    // Pipeline layout
    BIGOS::Driver::Backend::BindingSetLayoutHandle layouts[] = { m_hShaderResourceSetLayout, m_hSamplerSetLayout };
    BIGOS::Driver::Backend::PipelineLayoutDesc     pipelineLayoutDesc;
    pipelineLayoutDesc.constantRangeCount    = 0;
    pipelineLayoutDesc.bindingSetLayoutCount = 2;
    pipelineLayoutDesc.phBindigSetLayouts    = layouts;
    pipelineLayoutDesc.pConstantRanges       = nullptr;
    if( BGS_FAILED( m_pAPIDevice->CreatePipelineLayout( pipelineLayoutDesc, &m_hPipelineLayout ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Pipeline
    BIGOS::Driver::Backend::GraphicsPipelineDesc pipelineDesc;
    // Type
    pipelineDesc.type = BIGOS::Driver::Backend::PipelineTypes::GRAPHICS;
    // Shaders
    pipelineDesc.vertexShader.hShader     = m_hVertexShader;
    pipelineDesc.vertexShader.pEntryPoint = "VSMain";
    pipelineDesc.pixelShader.hShader      = m_hPixelShader;
    pipelineDesc.pixelShader.pEntryPoint  = "PSMain";
    pipelineDesc.hullShader.hShader       = BIGOS::Driver::Backend::ShaderHandle();
    pipelineDesc.domainShader.hShader     = BIGOS::Driver::Backend::ShaderHandle();
    pipelineDesc.geometryShader.hShader   = BIGOS::Driver::Backend::ShaderHandle();
    // VI
    BIGOS::Driver::Backend::InputBindingDesc inputBinding;
    inputBinding.binding   = 0;
    inputBinding.inputRate = BIGOS::Driver::Backend::InputStepRates::PER_VERTEX;
    BIGOS::Driver::Backend::InputElementDesc inputElements[ 2 ];
    inputElements[ 0 ].binding                = 0;
    inputElements[ 0 ].format                 = BIGOS::Driver::Backend::Formats::R32G32B32A32_FLOAT;
    inputElements[ 0 ].pSemanticName          = "POSITION";
    inputElements[ 0 ].location               = 0;
    inputElements[ 0 ].offset                 = 0 * sizeof( float );
    inputElements[ 1 ].binding                = 0;
    inputElements[ 1 ].format                 = BIGOS::Driver::Backend::Formats::R32G32_FLOAT;
    inputElements[ 1 ].pSemanticName          = "TEXCOORD";
    inputElements[ 1 ].location               = 1;
    inputElements[ 1 ].offset                 = 4 * sizeof( float );
    pipelineDesc.inputState.inputBindingCount = 1;
    pipelineDesc.inputState.inputElementCount = 2;
    pipelineDesc.inputState.pInputBindings    = &inputBinding;
    pipelineDesc.inputState.pInputElements    = inputElements;
    // IA
    pipelineDesc.inputAssemblerState.indexRestartValue = BIGOS::Driver::Backend::IndexRestartValues::DISABLED;
    pipelineDesc.inputAssemblerState.topology          = BIGOS::Driver::Backend::PrimitiveTopologies::TRIANGLE_LIST;
    // RS
    pipelineDesc.rasterizeState.frontFaceMode           = BIGOS::Driver::Backend::FrontFaceModes::COUNTER_CLOCKWISE;
    pipelineDesc.rasterizeState.cullMode                = BIGOS::Driver::Backend::CullModes::BACK;
    pipelineDesc.rasterizeState.fillMode                = BIGOS::Driver::Backend::PolygonFillModes::SOLID;
    pipelineDesc.rasterizeState.depthBiasClamp          = 0.0f;
    pipelineDesc.rasterizeState.depthBiasConstantFactor = 0.0f;
    pipelineDesc.rasterizeState.depthBiasSlopeFactor    = 0.0f;
    pipelineDesc.rasterizeState.depthClipEnable         = BIGOS::Core::BGS_FALSE;
    // MS
    pipelineDesc.multisampleState.sampleCount = BIGOS::Driver::Backend::SampleCount::COUNT_1;
    // DS
    pipelineDesc.depthStencilState.depthTestEnable   = BIGOS::Core::BGS_FALSE;
    pipelineDesc.depthStencilState.stencilTestEnable = BIGOS::Core::BGS_FALSE;
    pipelineDesc.depthStencilState.depthWriteEnable  = BIGOS::Core::BGS_TRUE;
    // BS
    BIGOS::Driver::Backend::RenderTargetBlendDesc rtb;
    rtb.blendEnable                                    = BIGOS::Core::BGS_FALSE;
    rtb.writeFlag                                      = BGS_FLAG( BIGOS::Driver::Backend::ColorComponentFlagBits::ALL );
    pipelineDesc.blendState.enableLogicOperations      = BIGOS::Core::BGS_FALSE;
    pipelineDesc.blendState.pRenderTargetBlendDescs    = &rtb;
    pipelineDesc.blendState.renderTargetBlendDescCount = 1;
    // RT
    BIGOS::Driver::Backend::FORMAT        rtFormats[ 1 ] = { BIGOS::Driver::Backend::Formats::B8G8R8A8_UNORM };
    const BIGOS::Driver::Backend::FORMAT* pFormat        = rtFormats;
    pipelineDesc.renderTargetCount                       = 1;
    pipelineDesc.pRenderTargetFormats                    = pFormat;
    pipelineDesc.depthStencilFormat                      = BIGOS::Driver::Backend::Formats::UNKNOWN;
    // PL
    pipelineDesc.hPipelineLayout = m_hPipelineLayout;
    if( BGS_FAILED( m_pAPIDevice->CreatePipeline( pipelineDesc, &m_hPipeline ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    return BIGOS::Results::OK;
}
