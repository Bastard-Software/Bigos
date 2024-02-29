#include "DebugNormals.h"

#include "Application.h"
#include "Driver/Frontend/RenderSystem.h"
#include "Driver/Frontend/Shader/IShaderCompiler.h"
#include "Platform/Window.h"

DebugNormals::DebugNormals( APITypes APIType, uint32_t width, uint32_t height, const char* pName )
    : Sample( APIType, width, height, pName )
    , m_pWindowSystem( nullptr )
    , m_pRenderSystem( nullptr )
    , m_pWindow( nullptr )
    , m_pDevice( nullptr )
    , m_pAPIDevice( nullptr )
    , m_pQueue( nullptr )
    , m_pSwapchain( nullptr )
    , m_hRTVs()
    , m_backBuffers()
    , m_hCommandPools()
    , m_hShaderResourceSetLayout()
    , m_hShaderResourceHeap()
    , m_hPipelineLayout()
    , m_frameNdx( 0 )
    , m_bindingsOffset( 0 )
    , m_pCommandBuffers()
    , m_hSolidPipeline()
    , m_hSolidVertexShader()
    , m_hSolidPixelShader()
    , m_hVertexBuffer()
    , m_hIndexBuffer()
    , m_hUploadBuffer()
    , m_hVertexIndexBufferMemory()
    , m_hUploadBufferMemory()
    , m_hDepthStencilBuffer()
    , m_hDepthStencilBufferMemory()
    , m_hDepthStencilBufferView()
    , m_hConstantBuffer()
    , m_hConstantBufferMemory()
    , m_pConstantBufferHost( nullptr )
{
    BIGOS::Memory::Set( &m_constantBufferData, 0, sizeof( m_constantBufferData ) );
    BIGOS::Memory::Set( &m_fenceValues, 0, sizeof( m_fenceValues ) );
    BIGOS::Memory::Set( &m_pCommandBuffers, 0, sizeof( m_pCommandBuffers ) );
}

BIGOS::RESULT DebugNormals::OnInit()
{
    if( BGS_FAILED( InitDevice() ) )
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

void DebugNormals::OnUpdate()
{
    m_pWindow->Update();

    m_constantBufferData.model[ 0 ][ 0 ] = 1.0f;
    m_constantBufferData.model[ 1 ][ 1 ] = 1.0f;
    m_constantBufferData.model[ 2 ][ 2 ] = 1.0f;
    m_constantBufferData.model[ 3 ][ 3 ] = 1.0f;

    m_constantBufferData.projection[ 0 ][ 0 ] = 1.0f;
    m_constantBufferData.projection[ 1 ][ 1 ] = 1.0f;
    m_constantBufferData.projection[ 2 ][ 2 ] = 1.0f;
    m_constantBufferData.projection[ 3 ][ 3 ] = 1.0f;

    // Mapping constant buffer memory
    BIGOS::Driver::Backend::MapResourceDesc mapConstant;
    mapConstant.hResource          = m_hConstantBuffer;
    mapConstant.bufferRange.size   = sizeof( m_constantBufferData );
    mapConstant.bufferRange.offset = 0;

    m_pAPIDevice->MapResource( mapConstant, &m_pConstantBufferHost );

    BIGOS::Memory::Copy( &m_constantBufferData, sizeof( m_constantBufferData ), m_pConstantBufferHost, sizeof( m_constantBufferData ) );

    m_pAPIDevice->UnmapResource( mapConstant );
}

void DebugNormals::OnRender()
{
    BIGOS::Driver::Backend::FrameInfo frameInfo;
    m_pSwapchain->GetNextFrame( &frameInfo );
    m_frameNdx = frameInfo.swapChainBackBufferIndex;

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

    if( BGS_FAILED( m_pAPIDevice->ResetCommandPool( m_hCommandPools[ m_frameNdx ] ) ) )
    {
        return;
    }

    // Render commands
    BIGOS::Driver::Backend::BeginCommandBufferDesc beginDesc;
    m_pCommandBuffers[ m_frameNdx ]->Begin( beginDesc );

    BIGOS::Driver::Backend::TextureBarrierDesc preTexBarrier;
    preTexBarrier.srcStage     = BGS_FLAG( BIGOS::Driver::Backend::PipelineStageFlagBits::RENDER_TARGET );
    preTexBarrier.srcAccess    = BGS_FLAG( BIGOS::Driver::Backend::AccessFlagBits::GENERAL );
    preTexBarrier.srcLayout    = BIGOS::Driver::Backend::TextureLayouts::PRESENT;
    preTexBarrier.dstStage     = BGS_FLAG( BIGOS::Driver::Backend::PipelineStageFlagBits::RENDER_TARGET );
    preTexBarrier.dstAccess    = BGS_FLAG( BIGOS::Driver::Backend::AccessFlagBits::RENDER_TARGET );
    preTexBarrier.dstLayout    = BIGOS::Driver::Backend::TextureLayouts::RENDER_TARGET;
    preTexBarrier.hResouce     = m_backBuffers[ m_frameNdx ].hBackBuffer;
    preTexBarrier.textureRange = { BGS_FLAG( TextureComponentFlagBits::COLOR ), 0, 1, 0, 1 };

    BIGOS::Driver::Backend::BarierDesc preBarrier;
    preBarrier.textureBarrierCount = 1;
    preBarrier.pTextureBarriers    = &preTexBarrier;
    preBarrier.globalBarrierCount  = 0;
    preBarrier.pGlobalBarriers     = nullptr;
    preBarrier.bufferBarrierCount  = 0;
    preBarrier.pBufferBarriers     = nullptr;

    m_pCommandBuffers[ m_frameNdx ]->Barrier( preBarrier );

    BIGOS::Driver::Backend::BeginRenderingDesc renderingDesc;
    renderingDesc.colorRenderTargetCount   = 1;
    renderingDesc.phColorRenderTargetViews = &m_hRTVs[ m_frameNdx ];
    renderingDesc.hDepthStencilTargetView  = m_hDepthStencilBufferView;
    renderingDesc.renderArea.offset        = { 0, 0 };
    renderingDesc.renderArea.size          = { m_width, m_height };
    m_pCommandBuffers[ m_frameNdx ]->BeginRendering( renderingDesc );

    m_pCommandBuffers[ m_frameNdx ]->SetPrimitiveTopology( BIGOS::Driver::Backend::PrimitiveTopologies::TRIANGLE_LIST );

    BIGOS::Driver::Backend::BindingHeapHandle bindingHeaps[] = {
        m_hShaderResourceHeap,
    };
    m_pCommandBuffers[ m_frameNdx ]->SetBindingHeaps( 1, bindingHeaps );

    BIGOS::Driver::Backend::ViewportDesc vp;
    vp.upperLeftX = 0.0f;
    vp.upperLeftY = 0.0f;
    vp.width      = static_cast<float>( m_width );
    vp.height     = static_cast<float>( m_height );
    vp.minDepth   = 0.0f;
    vp.maxDepth   = 1.0f;
    m_pCommandBuffers[ m_frameNdx ]->SetViewports( 1, &vp );

    BIGOS::Driver::Backend::ScissorDesc sr;
    sr.upperLeftX = 0;
    sr.upperLeftY = 0;
    sr.width      = m_width;
    sr.height     = m_height;
    m_pCommandBuffers[ m_frameNdx ]->SetScissors( 1, &sr );

    BIGOS::Driver::Backend::ColorValue clrColor;
    clrColor.r = 0.0f;
    clrColor.g = 0.0f;
    clrColor.b = 0.0f;
    clrColor.a = 0.0f;

    BIGOS::Core::Rect2D clrRect;
    clrRect.offset = { 0, 0 };
    clrRect.size   = { m_width, m_height };
    m_pCommandBuffers[ m_frameNdx ]->ClearBoundColorRenderTarget( 0, clrColor, 1, &clrRect );

    BIGOS::Driver::Backend::DepthStencilValue dsValue;
    dsValue.depth = 1.0f;

    m_pCommandBuffers[ m_frameNdx ]->ClearBoundDepthStencilTarget( dsValue, BGS_FLAG( BIGOS::Driver::Backend::TextureComponentFlagBits::DEPTH ), 1,
                                                                   &clrRect );

    BIGOS::Driver::Backend::VertexBufferDesc vbBindDesc;
    vbBindDesc.hVertexBuffer = m_hVertexBuffer;
    vbBindDesc.offset        = 0;
    vbBindDesc.size          = sizeof( VERTEX_DATA );
    vbBindDesc.elementStride = 10 * sizeof( float );
    m_pCommandBuffers[ m_frameNdx ]->SetVertexBuffers( 0, 1, &vbBindDesc );

    BIGOS::Driver::Backend::IndexBufferDesc ibBindDesc;
    ibBindDesc.hIndexBuffer = m_hIndexBuffer;
    ibBindDesc.indexType    = BIGOS::Driver::Backend::IndexTypes::UINT16;
    ibBindDesc.offset       = 0;
    m_pCommandBuffers[ m_frameNdx ]->SetIndexBuffer( ibBindDesc );

    m_pCommandBuffers[ m_frameNdx ]->SetPipeline( m_hSolidPipeline, BIGOS::Driver::Backend::PipelineTypes::GRAPHICS );

    BIGOS::Driver::Backend::SetBindingsDesc setBindingDesc;
    setBindingDesc.baseBindingOffset = m_bindingsOffset;
    setBindingDesc.heapNdx           = 0;
    setBindingDesc.setSpaceNdx       = 0;
    setBindingDesc.hPipelineLayout   = m_hPipelineLayout;
    setBindingDesc.type              = BIGOS::Driver::Backend::PipelineTypes::GRAPHICS;
    m_pCommandBuffers[ m_frameNdx ]->SetBindings( setBindingDesc );

    BIGOS::Driver::Backend::DrawDesc drawDesc;
    drawDesc.indexCount    = 36;
    drawDesc.instanceCount = 1;
    drawDesc.firstIndex    = 0;
    drawDesc.firstInstance = 0;
    drawDesc.vertexOffset  = 0;
    m_pCommandBuffers[ m_frameNdx ]->DrawIndexed( drawDesc );

    m_pCommandBuffers[ m_frameNdx ]->EndRendering();

    BIGOS::Driver::Backend::TextureBarrierDesc postTexBarrier;
    postTexBarrier.srcStage     = BGS_FLAG( BIGOS::Driver::Backend::PipelineStageFlagBits::RENDER_TARGET );
    postTexBarrier.srcAccess    = BGS_FLAG( BIGOS::Driver::Backend::AccessFlagBits::RENDER_TARGET );
    postTexBarrier.srcLayout    = BIGOS::Driver::Backend::TextureLayouts::RENDER_TARGET;
    postTexBarrier.dstStage     = BGS_FLAG( BIGOS::Driver::Backend::PipelineStageFlagBits::RENDER_TARGET );
    postTexBarrier.dstAccess    = BGS_FLAG( BIGOS::Driver::Backend::AccessFlagBits::GENERAL );
    postTexBarrier.dstLayout    = BIGOS::Driver::Backend::TextureLayouts::PRESENT;
    postTexBarrier.hResouce     = m_backBuffers[ m_frameNdx ].hBackBuffer;
    postTexBarrier.textureRange = { BGS_FLAG( TextureComponentFlagBits::COLOR ), 0, 1, 0, 1 };

    BIGOS::Driver::Backend::BarierDesc postBarrier;
    postBarrier.textureBarrierCount = 1;
    postBarrier.pTextureBarriers    = &postTexBarrier;
    postBarrier.globalBarrierCount  = 0;
    postBarrier.pGlobalBarriers     = nullptr;
    postBarrier.bufferBarrierCount  = 0;
    postBarrier.pBufferBarriers     = nullptr;

    m_pCommandBuffers[ m_frameNdx ]->Barrier( postBarrier );

    m_pCommandBuffers[ m_frameNdx ]->End();

    // Execute commands
    BIGOS::Driver::Backend::QueueSubmitDesc submitDesc;
    submitDesc.waitSemaphoreCount   = 1;
    submitDesc.phWaitSemaphores     = &frameInfo.hBackBufferAvailableSemaphore;
    submitDesc.waitFenceCount       = 0;
    submitDesc.phWaitFences         = nullptr;
    submitDesc.pWaitValues          = nullptr;
    submitDesc.commandBufferCount   = 1;
    submitDesc.ppCommandBuffers     = &m_pCommandBuffers[ m_frameNdx ];
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
    if( BGS_FAILED( m_pSwapchain->Present( presentDesc ) ) )
    {
        return;
    }
}

void DebugNormals::OnDestroy()
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
    m_pAPIDevice->DestroyResource( &m_hVertexBuffer );
    m_pAPIDevice->DestroyResource( &m_hIndexBuffer );
    m_pAPIDevice->DestroyResource( &m_hUploadBuffer );
    m_pAPIDevice->FreeMemory( &m_hVertexIndexBufferMemory );
    m_pAPIDevice->FreeMemory( &m_hUploadBufferMemory );
    m_pAPIDevice->DestroyResourceView( &m_hDepthStencilBufferView );
    m_pAPIDevice->DestroyResourceView( &m_hConstantBufferView );
    m_pAPIDevice->DestroyResource( &m_hDepthStencilBuffer );
    m_pAPIDevice->DestroyResource( &m_hConstantBuffer );
    m_pAPIDevice->FreeMemory( &m_hDepthStencilBufferMemory );
    m_pAPIDevice->FreeMemory( &m_hConstantBufferMemory );
    m_pAPIDevice->DestroyShader( &m_hSolidVertexShader );
    m_pAPIDevice->DestroyShader( &m_hSolidPixelShader );
    m_pAPIDevice->DestroyPipeline( &m_hSolidPipeline );
    m_pAPIDevice->DestroyPipelineLayout( &m_hPipelineLayout );
    m_pAPIDevice->DestroyBindingHeap( &m_hShaderResourceHeap );
    m_pAPIDevice->DestroyBindingSetLayout( &m_hShaderResourceSetLayout );
    for( uint32_t ndx = 0; ndx < FRAME_COUNT; ++ndx )
    {
        m_pAPIDevice->DestroyFence( &m_hFences[ ndx ] );
        m_pAPIDevice->DestroySemaphore( &m_hFrameFinishedSemaphores[ ndx ] );
        m_pAPIDevice->DestroyCommandBuffer( &m_pCommandBuffers[ ndx ] );
        m_pAPIDevice->DestroyCommandPool( &m_hCommandPools[ ndx ] );
        m_pAPIDevice->DestroyResourceView( &m_hRTVs[ ndx ] );
    }
    m_pAPIDevice->DestroySwapchain( &m_pSwapchain );
    m_pAPIDevice->DestroyQueue( &m_pQueue );
}

BIGOS::RESULT DebugNormals::InitDevice()
{
    BIGOS::Platform::WindowSystemDesc wndSystemDesc;
    if( BGS_FAILED( Application::GetFramework()->CreateWindowSystem( wndSystemDesc, &m_pWindowSystem ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Platform::WindowDesc wndDesc;
    wndDesc.pTitle    = m_pName;
    wndDesc.mode      = BIGOS::Platform::WindowModes::WINDOW;
    wndDesc.xPosition = 100;
    wndDesc.yPosition = 100;
    wndDesc.width     = m_width;
    wndDesc.height    = m_height;
    if( BGS_FAILED( m_pWindowSystem->CreateWindow( wndDesc, &m_pWindow ) ) )
    {
        return BIGOS::Results::FAIL;
    }
    m_pWindow->Show();

    // Creating device
    BIGOS::Driver::Frontend::RenderSystemDesc renderDesc;
    renderDesc.factoryDesc.apiType = m_APIType;
    renderDesc.factoryDesc.flags   = BGS_FLAG( BIGOS::Driver::Backend::FactoryFlagBits::ENABLE_DEBUG_LAYERS_IF_AVAILABLE );
    if( BGS_FAILED( Application::GetFramework()->CreateRenderSystem( renderDesc, &m_pRenderSystem ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Driver::Frontend::RenderDeviceDesc devDesc;
    devDesc.adapter.index = 0;
    if( BGS_FAILED( m_pRenderSystem->CreateDevice( devDesc, &m_pDevice ) ) )
    {
        return BIGOS::Results::FAIL;
    }
    m_pAPIDevice = m_pDevice->GetNativeAPIDevice();

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
    swapDesc.backBufferCount = FRAME_COUNT;
    swapDesc.format          = BIGOS::Driver::Backend::Formats::B8G8R8A8_UNORM;

    if( BGS_FAILED( m_pAPIDevice->CreateSwapchain( swapDesc, &m_pSwapchain ) ) )
    {
        return BIGOS::Results::OK;
    }

    return BIGOS::Results::OK;
}

BIGOS::RESULT DebugNormals::CreateFrameResources()
{
    m_backBuffers = m_pSwapchain->GetBackBuffers();

    BIGOS::Driver::Backend::TextureRangeDesc rtRange;
    rtRange.components      = BGS_FLAG( BIGOS::Driver::Backend::TextureComponentFlagBits::COLOR );
    rtRange.mipLevel        = 0;
    rtRange.mipLevelCount   = 1;
    rtRange.arrayLayer      = 0;
    rtRange.arrayLayerCount = 1;

    for( uint32_t ndx = 0; ndx < FRAME_COUNT; ++ndx )
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

BIGOS::RESULT DebugNormals::CreateApplicationResources()
{
    // Creating vertex buffer
    BIGOS::Driver::Backend::ResourceDesc vbDesc;
    vbDesc.arrayLayerCount = 1;
    vbDesc.format          = BIGOS::Driver::Backend::Formats::UNKNOWN;
    vbDesc.mipLevelCount   = 1;
    vbDesc.resourceLayout  = BIGOS::Driver::Backend::ResourceLayouts::LINEAR;
    vbDesc.resourceType    = BIGOS::Driver::Backend::ResourceTypes::BUFFER;
    vbDesc.resourceUsage   = BGS_FLAG( BIGOS::Driver::Backend::ResourceUsageFlagBits::VERTEX_BUFFER ) |
                           BGS_FLAG( BIGOS::Driver::Backend::ResourceUsageFlagBits::TRANSFER_DST );
    vbDesc.sampleCount = BIGOS::Driver::Backend::SampleCount::COUNT_1;
    vbDesc.sharingMode = BIGOS::Driver::Backend::ResourceSharingModes::EXCLUSIVE_ACCESS;
    vbDesc.size.width  = sizeof( VERTEX_DATA );
    vbDesc.size.height = 1;
    vbDesc.size.depth  = 1;
    if( BGS_FAILED( m_pAPIDevice->CreateResource( vbDesc, &m_hVertexBuffer ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Creating index buffer
    BIGOS::Driver::Backend::ResourceDesc ibDesc;
    ibDesc.arrayLayerCount = 1;
    ibDesc.format          = BIGOS::Driver::Backend::Formats::UNKNOWN;
    ibDesc.mipLevelCount   = 1;
    ibDesc.resourceLayout  = BIGOS::Driver::Backend::ResourceLayouts::LINEAR;
    ibDesc.resourceType    = BIGOS::Driver::Backend::ResourceTypes::BUFFER;
    ibDesc.resourceUsage   = BGS_FLAG( BIGOS::Driver::Backend::ResourceUsageFlagBits::INDEX_BUFFER ) |
                           BGS_FLAG( BIGOS::Driver::Backend::ResourceUsageFlagBits::TRANSFER_DST );
    ibDesc.sampleCount = BIGOS::Driver::Backend::SampleCount::COUNT_1;
    ibDesc.sharingMode = BIGOS::Driver::Backend::ResourceSharingModes::EXCLUSIVE_ACCESS;
    ibDesc.size.width  = sizeof( INDEX_DATA );
    ibDesc.size.height = 1;
    ibDesc.size.depth  = 1;
    if( BGS_FAILED( m_pAPIDevice->CreateResource( ibDesc, &m_hIndexBuffer ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Creating upload buffer
    BIGOS::Driver::Backend::ResourceDesc ubDesc;
    ubDesc.arrayLayerCount = 1;
    ubDesc.format          = BIGOS::Driver::Backend::Formats::UNKNOWN;
    ubDesc.mipLevelCount   = 1;
    ubDesc.resourceLayout  = BIGOS::Driver::Backend::ResourceLayouts::LINEAR;
    ubDesc.resourceType    = BIGOS::Driver::Backend::ResourceTypes::BUFFER;
    ubDesc.resourceUsage   = BGS_FLAG( BIGOS::Driver::Backend::ResourceUsageFlagBits::TRANSFER_SRC );
    ubDesc.sampleCount     = BIGOS::Driver::Backend::SampleCount::COUNT_1;
    ubDesc.sharingMode     = BIGOS::Driver::Backend::ResourceSharingModes::EXCLUSIVE_ACCESS;
    ubDesc.size.width      = sizeof( INDEX_DATA ) + sizeof( VERTEX_DATA );
    ubDesc.size.height     = 1;
    ubDesc.size.depth      = 1;
    if( BGS_FAILED( m_pAPIDevice->CreateResource( ubDesc, &m_hUploadBuffer ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Driver::Backend::ResourceAllocationInfo vbAllocInfo;
    m_pAPIDevice->GetResourceAllocationInfo( m_hVertexBuffer, &vbAllocInfo );

    BIGOS::Driver::Backend::ResourceAllocationInfo ibAllocInfo;
    m_pAPIDevice->GetResourceAllocationInfo( m_hIndexBuffer, &ibAllocInfo );

    BIGOS::Driver::Backend::ResourceAllocationInfo ubAllocInfo;
    m_pAPIDevice->GetResourceAllocationInfo( m_hUploadBuffer, &ubAllocInfo );

    BIGOS::Driver::Backend::AllocateMemoryDesc allocDesc;
    allocDesc.size      = vbAllocInfo.size + ibAllocInfo.size;
    allocDesc.alignment = max( vbAllocInfo.alignment, ibAllocInfo.alignment );
    allocDesc.heapType  = BIGOS::Driver::Backend::MemoryHeapTypes::DEFAULT;
    allocDesc.heapUsage = BIGOS::Driver::Backend::MemoryHeapUsages::BUFFERS;
    if( BGS_FAILED( m_pAPIDevice->AllocateMemory( allocDesc, &m_hVertexIndexBufferMemory ) ) )
    {
        return BIGOS::Results::FAIL;
    }
    allocDesc.size      = ubAllocInfo.size;
    allocDesc.alignment = ubAllocInfo.alignment;
    allocDesc.heapType  = BIGOS::Driver::Backend::MemoryHeapTypes::UPLOAD;
    if( BGS_FAILED( m_pAPIDevice->AllocateMemory( allocDesc, &m_hUploadBufferMemory ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Driver::Backend::BindResourceMemoryDesc bindVertexMemDesc;
    bindVertexMemDesc.hMemory      = m_hVertexIndexBufferMemory;
    bindVertexMemDesc.hResource    = m_hVertexBuffer;
    bindVertexMemDesc.memoryOffset = 0;
    if( BGS_FAILED( m_pAPIDevice->BindResourceMemory( bindVertexMemDesc ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Driver::Backend::BindResourceMemoryDesc bindIndexMemDesc;
    bindIndexMemDesc.hMemory      = m_hVertexIndexBufferMemory;
    bindIndexMemDesc.hResource    = m_hIndexBuffer;
    bindIndexMemDesc.memoryOffset = vbAllocInfo.size;
    if( BGS_FAILED( m_pAPIDevice->BindResourceMemory( bindIndexMemDesc ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Driver::Backend::BindResourceMemoryDesc bindUploadMemDesc;
    bindUploadMemDesc.hMemory      = m_hUploadBufferMemory;
    bindUploadMemDesc.hResource    = m_hUploadBuffer;
    bindUploadMemDesc.memoryOffset = 0;
    if( BGS_FAILED( m_pAPIDevice->BindResourceMemory( bindUploadMemDesc ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    void*                                   pHostAccess = nullptr;
    BIGOS::Driver::Backend::MapResourceDesc mapStaging;
    mapStaging.hResource          = m_hUploadBuffer;
    mapStaging.bufferRange.size   = sizeof( VERTEX_DATA ) + sizeof( INDEX_DATA );
    mapStaging.bufferRange.offset = 0;
    if( BGS_FAILED( m_pAPIDevice->MapResource( mapStaging, &pHostAccess ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Memory::Copy( VERTEX_DATA, sizeof( VERTEX_DATA ), pHostAccess, sizeof( VERTEX_DATA ) );
    BIGOS::byte_t* pOffsetedHost = static_cast<BIGOS::byte_t*>( pHostAccess ) + sizeof( VERTEX_DATA );
    BIGOS::Memory::Copy( INDEX_DATA, sizeof( INDEX_DATA ), pOffsetedHost, sizeof( INDEX_DATA ) );

    if( BGS_FAILED( m_pAPIDevice->UnmapResource( mapStaging ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Creating constant buffer
    BIGOS::Driver::Backend::ResourceDesc cbDesc;
    cbDesc.arrayLayerCount = 1;
    cbDesc.format          = BIGOS::Driver::Backend::Formats::UNKNOWN;
    cbDesc.mipLevelCount   = 1;
    cbDesc.resourceLayout  = BIGOS::Driver::Backend::ResourceLayouts::LINEAR;
    cbDesc.resourceType    = BIGOS::Driver::Backend::ResourceTypes::BUFFER;
    cbDesc.resourceUsage   = BGS_FLAG( BIGOS::Driver::Backend::ResourceUsageFlagBits::CONSTANT_BUFFER );
    cbDesc.sampleCount     = BIGOS::Driver::Backend::SampleCount::COUNT_1;
    cbDesc.sharingMode     = BIGOS::Driver::Backend::ResourceSharingModes::EXCLUSIVE_ACCESS;
    cbDesc.size.width      = sizeof( m_constantBufferData );
    cbDesc.size.height     = 1;
    cbDesc.size.depth      = 1;
    if( BGS_FAILED( m_pAPIDevice->CreateResource( cbDesc, &m_hConstantBuffer ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Driver::Backend::ResourceAllocationInfo cbAllocInfo;
    m_pAPIDevice->GetResourceAllocationInfo( m_hConstantBuffer, &cbAllocInfo );

    allocDesc.size      = cbAllocInfo.size;
    allocDesc.alignment = cbAllocInfo.alignment;
    allocDesc.heapType  = BIGOS::Driver::Backend::MemoryHeapTypes::UPLOAD;
    allocDesc.heapUsage = BIGOS::Driver::Backend::MemoryHeapUsages::BUFFERS;
    if( BGS_FAILED( m_pAPIDevice->AllocateMemory( allocDesc, &m_hConstantBufferMemory ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Binding constant buffer memory
    BIGOS::Driver::Backend::BindResourceMemoryDesc bindConstantMemDesc;
    bindConstantMemDesc.hMemory      = m_hConstantBufferMemory;
    bindConstantMemDesc.hResource    = m_hConstantBuffer;
    bindConstantMemDesc.memoryOffset = 0;
    if( BGS_FAILED( m_pAPIDevice->BindResourceMemory( bindConstantMemDesc ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Creating constant buffer view
    BIGOS::Driver::Backend::BufferViewDesc cbvDesc;
    cbvDesc.hResource    = m_hConstantBuffer;
    cbvDesc.usage        = BGS_FLAG( BIGOS::Driver::Backend::ResourceViewUsageFlagBits::CONSTANT_BUFFER );
    cbvDesc.range.offset = 0;
    cbvDesc.range.size   = sizeof( m_constantBufferData );
    if( BGS_FAILED( m_pAPIDevice->CreateResourceView( cbvDesc, &m_hConstantBufferView ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Creating depth stencil buffer
    BIGOS::Driver::Backend::ResourceDesc dsbDesc;
    dsbDesc.arrayLayerCount = 1;
    dsbDesc.format          = BIGOS::Driver::Backend::Formats::D24_UNORM_S8_UINT;
    dsbDesc.mipLevelCount   = 1;
    dsbDesc.resourceLayout  = BIGOS::Driver::Backend::ResourceLayouts::OPTIMAL;
    dsbDesc.resourceType    = BIGOS::Driver::Backend::ResourceTypes::TEXTURE_2D;
    dsbDesc.resourceUsage   = BGS_FLAG( BIGOS::Driver::Backend::ResourceUsageFlagBits::DEPTH_STENCIL_TARGET );
    dsbDesc.sampleCount     = BIGOS::Driver::Backend::SampleCount::COUNT_1;
    dsbDesc.sharingMode     = BIGOS::Driver::Backend::ResourceSharingModes::EXCLUSIVE_ACCESS;
    dsbDesc.size.width      = m_width;
    dsbDesc.size.height     = m_height;
    dsbDesc.size.depth      = 1;
    if( BGS_FAILED( m_pAPIDevice->CreateResource( dsbDesc, &m_hDepthStencilBuffer ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Driver::Backend::ResourceAllocationInfo dsbAllocInfo;
    m_pAPIDevice->GetResourceAllocationInfo( m_hDepthStencilBuffer, &dsbAllocInfo );

    allocDesc.size      = dsbAllocInfo.size;
    allocDesc.alignment = dsbAllocInfo.alignment;
    allocDesc.heapType  = BIGOS::Driver::Backend::MemoryHeapTypes::DEFAULT;
    allocDesc.heapUsage = BIGOS::Driver::Backend::MemoryHeapUsages::RENDER_TARGETS;
    if( BGS_FAILED( m_pAPIDevice->AllocateMemory( allocDesc, &m_hDepthStencilBufferMemory ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Driver::Backend::BindResourceMemoryDesc bindDepthMemDesc;
    bindDepthMemDesc.hMemory      = m_hDepthStencilBufferMemory;
    bindDepthMemDesc.hResource    = m_hDepthStencilBuffer;
    bindDepthMemDesc.memoryOffset = 0;
    if( BGS_FAILED( m_pAPIDevice->BindResourceMemory( bindDepthMemDesc ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Driver::Backend::TextureViewDesc dsvDesc;
    dsvDesc.hResource             = m_hDepthStencilBuffer;
    dsvDesc.usage                 = BGS_FLAG( BIGOS::Driver::Backend::ResourceViewUsageFlagBits::DEPTH_STENCIL_TARGET );
    dsvDesc.format                = BIGOS::Driver::Backend::Formats::D24_UNORM_S8_UINT;
    dsvDesc.layout                = BIGOS::Driver::Backend::TextureLayouts::DEPTH_STENCIL_WRITE;
    dsvDesc.textureType           = BIGOS::Driver::Backend::TextureTypes::TEXTURE_2D;
    dsvDesc.range.components      = BGS_FLAG( BIGOS::Driver::Backend::TextureComponentFlagBits::DEPTH );
    dsvDesc.range.arrayLayer      = 0;
    dsvDesc.range.arrayLayerCount = 1;
    dsvDesc.range.mipLevel        = 0;
    dsvDesc.range.mipLevelCount   = 1;
    if( BGS_FAILED( m_pAPIDevice->CreateResourceView( dsvDesc, &m_hDepthStencilBufferView ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    return BIGOS::Results::OK;
}

BIGOS::RESULT DebugNormals::UploadResourceData()
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

    // Uploading buffers
    BIGOS::Driver::Backend::CopyBufferDesc cpyBuffDesc;
    cpyBuffDesc.hSrcBuffer = m_hUploadBuffer;
    cpyBuffDesc.hDstBuffer = m_hVertexBuffer;
    cpyBuffDesc.srcOffset  = 0;
    cpyBuffDesc.dstOffset  = 0;
    cpyBuffDesc.size       = sizeof( VERTEX_DATA );
    pCpyCmdBuffer->CopyBuffer( cpyBuffDesc );

    cpyBuffDesc.hDstBuffer = m_hIndexBuffer;
    cpyBuffDesc.srcOffset  = sizeof( VERTEX_DATA );
    cpyBuffDesc.size       = sizeof( INDEX_DATA );
    pCpyCmdBuffer->CopyBuffer( cpyBuffDesc );

    // Expilicitly transitioning depth buffer
    BIGOS::Driver::Backend::TextureBarrierDesc dsBarrier;
    dsBarrier.srcAccess    = BGS_FLAG( BIGOS::Driver::Backend::AccessFlagBits::NONE );
    dsBarrier.srcStage     = BGS_FLAG( BIGOS::Driver::Backend::PipelineStageFlagBits::NONE );
    dsBarrier.srcLayout    = BIGOS::Driver::Backend::TextureLayouts::UNDEFINED;
    dsBarrier.dstAccess    = BGS_FLAG( BIGOS::Driver::Backend::AccessFlagBits::DEPTH_STENCIL_WRITE );
    dsBarrier.dstStage     = BGS_FLAG( BIGOS::Driver::Backend::PipelineStageFlagBits::DEPTH_STENCIL );
    dsBarrier.dstLayout    = BIGOS::Driver::Backend::TextureLayouts::DEPTH_STENCIL_WRITE;
    dsBarrier.hResouce     = m_hDepthStencilBuffer;
    dsBarrier.textureRange = { BGS_FLAG( TextureComponentFlagBits::DEPTH ), 0, 1, 0, 1 };

    BIGOS::Driver::Backend::BarierDesc barrierDesc;
    barrierDesc.bufferBarrierCount  = 0;
    barrierDesc.pBufferBarriers     = nullptr;
    barrierDesc.textureBarrierCount = 1;
    barrierDesc.pTextureBarriers    = &dsBarrier;
    barrierDesc.globalBarrierCount  = 0;
    barrierDesc.pGlobalBarriers     = nullptr;
    pCpyCmdBuffer->Barrier( barrierDesc );

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

BIGOS::RESULT DebugNormals::CreatePipeline()
{
    // Compiling solid pipeline shaders
    {
        void*    pShader = nullptr;
        uint32_t size    = 0;
        // Allocates memory that needs to be freed
        if( BGS_FAILED( readDataFromFile( "../Samples/BackendAPI/DebugNormals/solidShader.hlsl", &pShader, &size ) ) )
        {
            return BIGOS::Results::FAIL;
        }

        BIGOS::Driver::Frontend::ShaderCompilerOutput* pVSBlob = nullptr;
        BIGOS::Driver::Frontend::ShaderCompilerOutput* pPSBlob = nullptr;
        BIGOS::Driver::Frontend::CompileShaderDesc     shaderDesc;
        shaderDesc.argCount           = 0;
        shaderDesc.ppArgs             = nullptr;
        shaderDesc.outputFormat       = m_APIType == BIGOS::Driver::Backend::APITypes::D3D12 ? BIGOS::Driver::Frontend::ShaderFormats::DXIL
                                                                                             : BIGOS::Driver::Frontend::ShaderFormats::SPIRV;
        shaderDesc.model              = BIGOS::Driver::Frontend::ShaderModels::SHADER_MODEL_6_5;
        shaderDesc.source.pSourceCode = pShader;
        shaderDesc.source.sourceSize  = size;
        shaderDesc.type               = BIGOS::Driver::Backend::ShaderTypes::VERTEX;
        shaderDesc.pEntryPoint        = "VSMain";
        if( BGS_FAILED( m_pRenderSystem->GetDefaultCompiler()->Compile( shaderDesc, &pVSBlob ) ) )
        {
            return BIGOS::Results::FAIL;
        }
        shaderDesc.type        = BIGOS::Driver::Backend::ShaderTypes::PIXEL;
        shaderDesc.pEntryPoint = "PSMain";
        if( BGS_FAILED( m_pRenderSystem->GetDefaultCompiler()->Compile( shaderDesc, &pPSBlob ) ) )
        {
            return BIGOS::Results::FAIL;
        }

        BIGOS::Driver::Backend::ShaderDesc shDesc;
        shDesc.pByteCode = pVSBlob->pByteCode;
        shDesc.codeSize  = pVSBlob->byteCodeSize;
        if( BGS_FAILED( m_pAPIDevice->CreateShader( shDesc, &m_hSolidVertexShader ) ) )
        {
            return BIGOS::Results::FAIL;
        }

        shDesc.pByteCode = pPSBlob->pByteCode;
        shDesc.codeSize  = pPSBlob->byteCodeSize;
        if( BGS_FAILED( m_pAPIDevice->CreateShader( shDesc, &m_hSolidPixelShader ) ) )
        {
            return BIGOS::Results::FAIL;
        }

        m_pRenderSystem->GetDefaultCompiler()->DestroyOutput( &pVSBlob );
        m_pRenderSystem->GetDefaultCompiler()->DestroyOutput( &pPSBlob );

        delete[] pShader;
    }

    /*
    // Compiling debug normals pipeline shaders
    {
        void*    pShader = nullptr;
        uint32_t size    = 0;
        // Allocates memory that needs to be freed
        if( BGS_FAILED( readDataFromFile( "../Samples/BackendAPI/DebugNormals/normalShader.hlsl", &pShader, &size ) ) )
        {
            return BIGOS::Results::FAIL;
        }

        BIGOS::Driver::Frontend::ShaderCompilerOutput* pVSBlob = nullptr;
        BIGOS::Driver::Frontend::ShaderCompilerOutput* pPSBlob = nullptr;
        BIGOS::Driver::Frontend::ShaderCompilerOutput* pGSBlob = nullptr;
        BIGOS::Driver::Frontend::CompileShaderDesc     shaderDesc;
        shaderDesc.argCount           = 0;
        shaderDesc.ppArgs             = nullptr;
        shaderDesc.outputFormat       = m_APIType == BIGOS::Driver::Backend::APITypes::D3D12 ? BIGOS::Driver::Frontend::ShaderFormats::DXIL
                                                                                             : BIGOS::Driver::Frontend::ShaderFormats::SPIRV;
        shaderDesc.model              = BIGOS::Driver::Frontend::ShaderModels::SHADER_MODEL_6_5;
        shaderDesc.source.pSourceCode = pShader;
        shaderDesc.source.sourceSize  = size;
        shaderDesc.type               = BIGOS::Driver::Backend::ShaderTypes::VERTEX;
        shaderDesc.pEntryPoint        = "VSMain";
        if( BGS_FAILED( m_pRenderSystem->GetDefaultCompiler()->Compile( shaderDesc, &pVSBlob ) ) )
        {
            return BIGOS::Results::FAIL;
        }
        shaderDesc.type        = BIGOS::Driver::Backend::ShaderTypes::PIXEL;
        shaderDesc.pEntryPoint = "PSMain";
        if( BGS_FAILED( m_pRenderSystem->GetDefaultCompiler()->Compile( shaderDesc, &pPSBlob ) ) )
        {
            return BIGOS::Results::FAIL;
        }
        shaderDesc.type        = BIGOS::Driver::Backend::ShaderTypes::GEOMETRY;
        shaderDesc.pEntryPoint = "GSMain";
        if( BGS_FAILED( m_pRenderSystem->GetDefaultCompiler()->Compile( shaderDesc, &pGSBlob ) ) )
        {
            return BIGOS::Results::FAIL;
        }

        BIGOS::Driver::Backend::ShaderDesc shDesc;
        shDesc.pByteCode = pVSBlob->pByteCode;
        shDesc.codeSize  = pVSBlob->byteCodeSize;
        if( BGS_FAILED( m_pAPIDevice->CreateShader( shDesc, &m_hNormalsVertexShader ) ) )
        {
            return BIGOS::Results::FAIL;
        }
        shDesc.pByteCode = pPSBlob->pByteCode;
        shDesc.codeSize  = pPSBlob->byteCodeSize;
        if( BGS_FAILED( m_pAPIDevice->CreateShader( shDesc, &m_hNormalsPixelShader ) ) )
        {
            return BIGOS::Results::FAIL;
        }
        shDesc.pByteCode = pPSBlob->pByteCode;
        shDesc.codeSize  = pPSBlob->byteCodeSize;
        if( BGS_FAILED( m_pAPIDevice->CreateShader( shDesc, &m_hNormalsGeometryShader ) ) )
        {
            return BIGOS::Results::FAIL;
        }

        m_pRenderSystem->GetDefaultCompiler()->DestroyOutput( &pVSBlob );
        m_pRenderSystem->GetDefaultCompiler()->DestroyOutput( &pPSBlob );
        m_pRenderSystem->GetDefaultCompiler()->DestroyOutput( &pGSBlob );

        delete[] pShader;
    }
    */

    // Binding set layout
    BIGOS::Driver::Backend::BindingRangeDesc bindingRange;
    bindingRange.baseBindingSlot    = 0;
    bindingRange.baseShaderRegister = 0;
    bindingRange.bindingCount       = 1;
    bindingRange.type               = BIGOS::Driver::Backend::BindingTypes::CONSTANT_BUFFER;
    BIGOS::Driver::Backend::BindingSetLayoutDesc bindingLayoutDesc;
    bindingLayoutDesc.visibility        = BIGOS::Driver::Backend::ShaderVisibilities::ALL;
    bindingLayoutDesc.bindingRangeCount = 1;
    bindingLayoutDesc.pBindingRanges    = &bindingRange;
    if( BGS_FAILED( m_pAPIDevice->CreateBindingSetLayout( bindingLayoutDesc, &m_hShaderResourceSetLayout ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Binding heap
    BIGOS::Driver::Backend::BindingHeapDesc bindingHeapDesc;
    bindingHeapDesc.bindingCount = 1;
    bindingHeapDesc.type         = BIGOS::Driver::Backend::BindingHeapTypes::SHADER_RESOURCE;
    if( BGS_FAILED( m_pAPIDevice->CreateBindingHeap( bindingHeapDesc, &m_hShaderResourceHeap ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Write binding to heap
    BIGOS::Driver::Backend::GetBindingOffsetDesc getAddressDesc;
    getAddressDesc.hBindingSetLayout = m_hShaderResourceSetLayout;
    getAddressDesc.bindingNdx        = 0;
    m_pAPIDevice->GetBindingOffset( getAddressDesc, &m_bindingsOffset );

    BIGOS::Driver::Backend::WriteBindingDesc writeBindingDesc;
    writeBindingDesc.bindingType   = BindingTypes::CONSTANT_BUFFER;
    writeBindingDesc.hDstHeap      = m_hShaderResourceHeap;
    writeBindingDesc.dstOffset     = m_bindingsOffset;
    writeBindingDesc.hResourceView = m_hConstantBufferView;
    m_pAPIDevice->WriteBinding( writeBindingDesc );

    // Pipeline layout
    BIGOS::Driver::Backend::PipelineLayoutDesc pipelineLayoutDesc;
    pipelineLayoutDesc.constantRangeCount    = 0;
    pipelineLayoutDesc.bindingSetLayoutCount = 1;
    pipelineLayoutDesc.phBindigSetLayouts    = &m_hShaderResourceSetLayout;
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
    pipelineDesc.vertexShader.hShader     = m_hSolidVertexShader;
    pipelineDesc.vertexShader.pEntryPoint = "VSMain";
    pipelineDesc.pixelShader.hShader      = m_hSolidPixelShader;
    pipelineDesc.pixelShader.pEntryPoint  = "PSMain";
    pipelineDesc.hullShader.hShader       = BIGOS::Driver::Backend::ShaderHandle();
    pipelineDesc.domainShader.hShader     = BIGOS::Driver::Backend::ShaderHandle();
    pipelineDesc.geometryShader.hShader   = BIGOS::Driver::Backend::ShaderHandle();
    // VI
    BIGOS::Driver::Backend::InputBindingDesc inputBinding;
    inputBinding.binding   = 0;
    inputBinding.inputRate = BIGOS::Driver::Backend::InputStepRates::PER_VERTEX;
    BIGOS::Driver::Backend::InputElementDesc inputElements[ 3 ];
    inputElements[ 0 ].binding                = 0;
    inputElements[ 0 ].format                 = BIGOS::Driver::Backend::Formats::R32G32B32A32_FLOAT;
    inputElements[ 0 ].pSemanticName          = "POSITION";
    inputElements[ 0 ].location               = 0;
    inputElements[ 0 ].offset                 = 0 * sizeof( float );
    inputElements[ 1 ].binding                = 0;
    inputElements[ 1 ].format                 = BIGOS::Driver::Backend::Formats::R32G32B32_FLOAT;
    inputElements[ 1 ].pSemanticName          = "NORMAL";
    inputElements[ 1 ].location               = 1;
    inputElements[ 1 ].offset                 = 4 * sizeof( float );
    inputElements[ 2 ].binding                = 0;
    inputElements[ 2 ].format                 = BIGOS::Driver::Backend::Formats::R32G32B32_FLOAT;
    inputElements[ 2 ].pSemanticName          = "COLOR";
    inputElements[ 2 ].location               = 2;
    inputElements[ 2 ].offset                 = 7 * sizeof( float );
    pipelineDesc.inputState.inputBindingCount = 1;
    pipelineDesc.inputState.inputElementCount = 3;
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
    pipelineDesc.depthStencilState.depthTestEnable   = BIGOS::Core::BGS_TRUE;
    pipelineDesc.depthStencilState.depthWriteEnable  = BIGOS::Core::BGS_TRUE;
    pipelineDesc.depthStencilState.depthCompare      = BIGOS::Driver::Backend::CompareOperationTypes::LESS_OR_EQUAL;
    pipelineDesc.depthStencilState.stencilTestEnable = BIGOS::Core::BGS_FALSE;
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
    pipelineDesc.depthStencilFormat                      = BIGOS::Driver::Backend::Formats::D24_UNORM_S8_UINT;
    // PL
    pipelineDesc.hPipelineLayout = m_hPipelineLayout;
    if( BGS_FAILED( m_pAPIDevice->CreatePipeline( pipelineDesc, &m_hSolidPipeline ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    return BIGOS::Results::OK;
}
