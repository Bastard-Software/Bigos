#include "BigosFramework/BigosFramework.h"
#include "Core/Memory/SystemHeapAllocator.h"
#include "Core/Utils/String.h"
#include "Driver/Frontend/RenderDevice.h"
#include "Driver/Frontend/RenderSystem.h"
#include "Driver/Frontend/Shader/IShaderCompiler.h"
#include "Platform/Window.h"
#include "Platform/WindowSystem.h"
#include <chrono>
#include <stdio.h>

const char* SHADER = "struct VSInput\n"
                     "{\n"
                     "    float4 position : POSITION;\n"
                     "    float4 color : COLOR;\n"
                     "};\n"

                     "struct PSInput\n"
                     "{\n"
                     "    float4 position : SV_POSITION;\n"
                     "    float4 color : COLOR;\n"
                     "};\n"

                     "PSInput VSMain( VSInput input )\n"
                     "{\n"
                     "    PSInput output;\n"
                     "    output.position = input.position;\n"
                     "    output.color = input.color;\n"

                     "    return output;\n"
                     "}\n\n"

                     "float4 PSMain( PSInput input ) : SV_Target\n"
                     "{\n"
                     "  return input.color;\n"
                     "}\n";

const BIGOS::Driver::Backend::API_TYPE API_TYPE               = BIGOS::Driver::Backend::APITypes::D3D12;
const uint32_t                         BACK_BUFFER_COUNT      = 2;
const uint32_t                         FRAMES_IN_FLIGHT_COUNT = 2;

// Temporary solution
extern "C"
{

#if( BGS_D3D12_API )

    BGS_API extern const UINT  D3D12SDKVersion = 610;
    BGS_API extern const char* D3D12SDKPath    = ".\\D3D12\\";

#endif // ( BGS_D3D12_API )
};

int main()
{
    BIGOS::BigosFramework*    pFramework = nullptr;
    BIGOS::BigosFrameworkDesc frameworkDesc;
    frameworkDesc.renderSystemDesc.factoryDesc.apiType = API_TYPE;
    frameworkDesc.renderSystemDesc.factoryDesc.flags   = 1;

    if( BGS_FAILED( CreateBigosFramework( frameworkDesc, &pFramework ) ) )
    {
        printf( "Framework initialization failed." );
        return -1;
    }

    BIGOS::Platform::WindowSystem* pWindowSystem = nullptr;
    if( BGS_FAILED( pFramework->CreateWindowSystem( frameworkDesc.windowSystemDesc, &pWindowSystem ) ) )
    {
        return -1;
    }

    BIGOS::Driver::Frontend::RenderSystem* pRenderSystem = nullptr;
    if( BGS_FAILED( pFramework->CreateRenderSystem( frameworkDesc.renderSystemDesc, &pRenderSystem ) ) )
    {
        return -1;
    }

    BIGOS::Platform::Window*    pWnd = nullptr;
    BIGOS::Platform::WindowDesc wndDesc;
    wndDesc.pTitle    = "Test window";
    wndDesc.mode      = BIGOS::Platform::WindowModes::WINDOW;
    wndDesc.xPosition = 100;
    wndDesc.yPosition = 100;
    wndDesc.width     = 1280;
    wndDesc.height    = 720;
    if( BGS_FAILED( pWindowSystem->CreateWindow( wndDesc, &pWnd ) ) )
    {
        return -1;
    }
    pWnd->Show();

    BIGOS::Driver::Frontend::RenderDeviceDesc devDesc;
    devDesc.adapter.index                                = 0;
    BIGOS::Driver::Frontend::RenderDevice* pRenderDevice = nullptr;
    if( BGS_FAILED( pRenderSystem->CreateDevice( devDesc, &pRenderDevice ) ) )
    {
        return -1;
    }

    BIGOS::Driver::Backend::IDevice* pAPIDevice = pRenderDevice->GetNativeAPIDevice();

    // Synchronization objects
    BIGOS::Driver::Backend::FenceHandle     hFences[ FRAMES_IN_FLIGHT_COUNT ];
    BIGOS::Driver::Backend::SemaphoreHandle hSemaphores[ FRAMES_IN_FLIGHT_COUNT ];
    uint64_t                                fenceValues[ FRAMES_IN_FLIGHT_COUNT ] = {
        0,
        0,
    };

    for( uint32_t ndx = 0; ndx < BACK_BUFFER_COUNT; ++ndx )
    {
        BIGOS::Driver::Backend::SemaphoreDesc semaphoreDesc;
        if( BGS_FAILED( pAPIDevice->CreateSemaphore( semaphoreDesc, &hSemaphores[ ndx ] ) ) )
        {
            return -1;
        }

        BIGOS::Driver::Backend::FenceDesc fenceDesc;
        fenceDesc.initialValue = fenceValues[ ndx ];
        if( BGS_FAILED( pAPIDevice->CreateFence( fenceDesc, &hFences[ ndx ] ) ) )
        {
            return -1;
        }
    }

    // Queue and swapchain
    BIGOS::Driver::Backend::IQueue*   pGraphicsQueue = nullptr;
    BIGOS::Driver::Backend::QueueDesc queueDesc;
    queueDesc.type     = BIGOS::Driver::Backend::QueueTypes::GRAPHICS;
    queueDesc.priority = BIGOS::Driver::Backend::QueuePriorityTypes::NORMAL;
    if( BGS_FAILED( pAPIDevice->CreateQueue( queueDesc, &pGraphicsQueue ) ) )
    {
        return -1;
    }

    BIGOS::Driver::Backend::ISwapchain*   pSwapchain = nullptr;
    BIGOS::Driver::Backend::SwapchainDesc swapDesc;
    swapDesc.pQueue          = pGraphicsQueue;
    swapDesc.pWindow         = pWnd;
    swapDesc.vSync           = BIGOS::Core::BGS_TRUE;
    swapDesc.backBufferCount = 2;
    swapDesc.format          = BIGOS::Driver::Backend::Formats::B8G8R8A8_UNORM;

    if( BGS_FAILED( pAPIDevice->CreateSwapchain( swapDesc, &pSwapchain ) ) )
    {
        return -1;
    }

    const BIGOS::Driver::Backend::BackBufferArray& backBuffers = pSwapchain->GetBackBuffers();

    BIGOS::Driver::Backend::ResourceViewHandle hRtvs[ BACK_BUFFER_COUNT ];
    BIGOS::Driver::Backend::CommandPoolHandle  hCmdPools[ BACK_BUFFER_COUNT ];
    BIGOS::Driver::Backend::ICommandBuffer*    pCmdBuffers[ BACK_BUFFER_COUNT ] = {
        nullptr,
        nullptr,
    };

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
        rtvDesc.hResource             = backBuffers[ ndx ].hBackBuffer;

        if( BGS_FAILED( pAPIDevice->CreateResourceView( rtvDesc, &hRtvs[ ndx ] ) ) )
        {
            return -1;
        }

        BIGOS::Driver::Backend::CommandPoolDesc cmdPoolDesc;
        cmdPoolDesc.pQueue = pGraphicsQueue;
        if( BGS_FAILED( pAPIDevice->CreateCommandPool( cmdPoolDesc, &hCmdPools[ ndx ] ) ) )
        {
            return -1;
        }

        BIGOS::Driver::Backend::CommandBufferDesc cmdBufferDesc;
        cmdBufferDesc.hCommandPool = hCmdPools[ ndx ];
        cmdBufferDesc.level        = BIGOS::Driver::Backend::CommandBufferLevels::PRIMARY;
        cmdBufferDesc.pQueue       = pGraphicsQueue;
        if( BGS_FAILED( pAPIDevice->CreateCommandBuffer( cmdBufferDesc, &pCmdBuffers[ ndx ] ) ) )
        {
            return -1;
        }

        uint64_t                            fenceVal = 0;
        BIGOS::Driver::Backend::FenceDesc   tempFenceDesc;
        BIGOS::Driver::Backend::FenceHandle hTempFence;
        tempFenceDesc.initialValue = fenceVal;
        if( BGS_FAILED( pAPIDevice->CreateFence( tempFenceDesc, &hTempFence ) ) )
        {
            return -1;
        }
        fenceVal++;

        // Transition from undefined layout to general layout
        BIGOS::Driver::Backend::BeginCommandBufferDesc beginDesc;
        pCmdBuffers[ ndx ]->Begin( beginDesc );

        BIGOS::Driver::Backend::TextureBarrierDesc preRenderTexBarrier;
        preRenderTexBarrier.srcStage     = BGS_FLAG( BIGOS::Driver::Backend::PipelineStageFlagBits::NONE );
        preRenderTexBarrier.srcAccess    = BGS_FLAG( BIGOS::Driver::Backend::AccessFlagBits::NONE );
        preRenderTexBarrier.srcLayout    = BIGOS::Driver::Backend::TextureLayouts::UNDEFINED;
        preRenderTexBarrier.dstStage     = BGS_FLAG( BIGOS::Driver::Backend::PipelineStageFlagBits::NONE );
        preRenderTexBarrier.dstAccess    = BGS_FLAG( BIGOS::Driver::Backend::AccessFlagBits::NONE );
        preRenderTexBarrier.dstLayout    = BIGOS::Driver::Backend::TextureLayouts::PRESENT;
        preRenderTexBarrier.hResouce     = backBuffers[ ndx ].hBackBuffer;
        preRenderTexBarrier.textureRange = rtRange;

        BIGOS::Driver::Backend::BarierDesc preRenderBarrier;
        preRenderBarrier.textureBarrierCount = 1;
        preRenderBarrier.pTextureBarriers    = &preRenderTexBarrier;
        preRenderBarrier.globalBarrierCount  = 0;
        preRenderBarrier.pGlobalBarriers     = nullptr;
        preRenderBarrier.bufferBarrierCount  = 0;
        preRenderBarrier.pBufferBarriers     = nullptr;

        pCmdBuffers[ ndx ]->Barrier( preRenderBarrier );

        pCmdBuffers[ ndx ]->End();

        BIGOS::Driver::Backend::QueueSubmitDesc submitDesc;
        submitDesc.waitSemaphoreCount   = 0;
        submitDesc.phWaitSemaphores     = nullptr;
        submitDesc.waitFenceCount       = 0;
        submitDesc.phWaitFences         = nullptr;
        submitDesc.pWaitValues          = nullptr;
        submitDesc.commandBufferCount   = 1;
        submitDesc.ppCommandBuffers     = &pCmdBuffers[ ndx ];
        submitDesc.signalSemaphoreCount = 0;
        submitDesc.phSignalSemaphores   = nullptr;
        submitDesc.signalFenceCount     = 1;
        submitDesc.phSignalFences       = &hTempFence;
        submitDesc.pSignalValues        = &fenceVal;
        if( BGS_FAILED( pGraphicsQueue->Submit( submitDesc ) ) )
        {
            return -1;
        }

        BIGOS::Driver::Backend::WaitForFencesDesc waitDesc;
        waitDesc.fenceCount  = 1;
        waitDesc.waitAll     = BIGOS::Core::BGS_TRUE;
        waitDesc.pFences     = &hTempFence;
        waitDesc.pWaitValues = &fenceVal;
        if( BGS_FAILED( pAPIDevice->WaitForFences( waitDesc, UINT64_MAX ) ) )
        {
            return -1;
        }

        pAPIDevice->DestroyFence( &hTempFence );
    }

    auto                                           compiler = pRenderSystem->GetDefaultCompiler();
    BIGOS::Driver::Frontend::ShaderCompilerOutput* pVSBlob  = nullptr;
    BIGOS::Driver::Frontend::ShaderCompilerOutput* pPSBlob  = nullptr;
    BIGOS::Driver::Frontend::CompileShaderDesc     shaderDesc;
    shaderDesc.argCount           = 0;
    shaderDesc.ppArgs             = nullptr;
    shaderDesc.outputFormat       = API_TYPE == BIGOS::Driver::Backend::APITypes::D3D12 ? BIGOS::Driver::Frontend::ShaderFormats::DXIL
                                                                                        : BIGOS::Driver::Frontend::ShaderFormats::SPIRV;
    shaderDesc.model              = BIGOS::Driver::Frontend::ShaderModels::SHADER_MODEL_6_5;
    shaderDesc.source.pSourceCode = SHADER;
    shaderDesc.source.sourceSize  = static_cast<uint32_t>( BIGOS::Core::Utils::String::Length( SHADER ) );
    shaderDesc.type               = BIGOS::Driver::Backend::ShaderTypes::VERTEX;
    shaderDesc.pEntryPoint        = "VSMain";
    if( BGS_FAILED( compiler->Compile( shaderDesc, &pVSBlob ) ) )
    {
        return -1;
    }
    shaderDesc.type        = BIGOS::Driver::Backend::ShaderTypes::PIXEL;
    shaderDesc.pEntryPoint = "PSMain";
    if( BGS_FAILED( compiler->Compile( shaderDesc, &pPSBlob ) ) )
    {
        return -1;
    }

    BIGOS::Driver::Backend::ShaderHandle hVS;
    BIGOS::Driver::Backend::ShaderDesc   shDesc;
    shDesc.pByteCode = pVSBlob->pByteCode;
    shDesc.codeSize  = pVSBlob->byteCodeSize;
    if( BGS_FAILED( pAPIDevice->CreateShader( shDesc, &hVS ) ) )
    {
        return -1;
    }

    BIGOS::Driver::Backend::ShaderHandle hPS;
    shDesc.pByteCode = pPSBlob->pByteCode;
    shDesc.codeSize  = pPSBlob->byteCodeSize;
    if( BGS_FAILED( pAPIDevice->CreateShader( shDesc, &hPS ) ) )
    {
        return -1;
    }

    compiler->DestroyOutput( &pVSBlob );
    compiler->DestroyOutput( &pPSBlob );

    // Emplty binding heap layout, pipeline layout and pipeline
    BIGOS::Driver::Backend::BindingHeapLayoutHandle hEmptyBindingLayout;
    BIGOS::Driver::Backend::BindingHeapLayoutDesc   emptyBindingLayoutDesc;
    emptyBindingLayoutDesc.visibility        = BIGOS::Driver::Backend::ShaderVisibilities::ALL;
    emptyBindingLayoutDesc.bindingRangeCount = 0;
    emptyBindingLayoutDesc.pBindingRanges    = nullptr;
    if( BGS_FAILED( pAPIDevice->CreateBindingHeapLayout( emptyBindingLayoutDesc, &hEmptyBindingLayout ) ) )
    {
        return -1;
    }

    BIGOS::Driver::Backend::PipelineLayoutHandle hEmptyLayout;
    BIGOS::Driver::Backend::PipelineLayoutDesc   emptyLayoutDesc;
    emptyLayoutDesc.constantRangeCount = 0;
    emptyLayoutDesc.hBindigHeapLayout  = hEmptyBindingLayout;
    emptyLayoutDesc.pConstantRanges    = nullptr;
    if( BGS_FAILED( pAPIDevice->CreatePipelineLayout( emptyLayoutDesc, &hEmptyLayout ) ) )
    {
        return -1;
    }

    BIGOS::Driver::Backend::PipelineHandle       hPipeline;
    BIGOS::Driver::Backend::GraphicsPipelineDesc pipelineDesc;
    // Type
    pipelineDesc.type = BIGOS::Driver::Backend::PipelineTypes::GRAPHICS;
    // Shaders
    pipelineDesc.vertexShader.hShader     = hVS;
    pipelineDesc.vertexShader.pEntryPoint = "VSMain";
    pipelineDesc.pixelShader.hShader      = hPS;
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
    inputElements[ 1 ].format                 = BIGOS::Driver::Backend::Formats::R32G32B32A32_FLOAT;
    inputElements[ 1 ].pSemanticName          = "COLOR";
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
    pipelineDesc.hPipelineLayout = hEmptyLayout;
    if( BGS_FAILED( pAPIDevice->CreatePipeline( pipelineDesc, &hPipeline ) ) )
    {
        return -1;
    }

    BIGOS::Driver::Backend::ResourceHandle hVertexBuffer;
    BIGOS::Driver::Backend::ResourceDesc   vbDesc;
    vbDesc.arrayLayerCount = 1;
    vbDesc.format          = BIGOS::Driver::Backend::Formats::UNKNOWN;
    vbDesc.mipLevelCount   = 1;
    vbDesc.resourceLayout  = BIGOS::Driver::Backend::ResourceLayouts::LINEAR;
    vbDesc.resourceType    = BIGOS::Driver::Backend::ResourceTypes::BUFFER;
    vbDesc.resourceUsage   = static_cast<uint32_t>( BIGOS::Driver::Backend::ResourceUsageFlagBits::VERTEX_BUFFER );
    vbDesc.sampleCount     = BIGOS::Driver::Backend::SampleCount::COUNT_1;
    vbDesc.sharingMode     = BIGOS::Driver::Backend::ResourceSharingModes::EXCLUSIVE_ACCESS;
    vbDesc.size.width      = 3 /* triangle */ * 8 /* position (4) + color (4) */ * sizeof( float );
    vbDesc.size.height     = 1;
    vbDesc.size.depth      = 1;
    if( BGS_FAILED( pAPIDevice->CreateResource( vbDesc, &hVertexBuffer ) ) )
    {
        return -1;
    }

    BIGOS::Driver::Backend::ResourceAllocationInfo vbAllocInfo;
    pAPIDevice->GetResourceAllocationInfo( hVertexBuffer, &vbAllocInfo );

    BIGOS::Driver::Backend::MemoryHandle       hBufferMem;
    BIGOS::Driver::Backend::AllocateMemoryDesc allocDesc;
    allocDesc.size      = vbAllocInfo.size;
    allocDesc.alignment = vbAllocInfo.alignment;
    allocDesc.heapType  = BIGOS::Driver::Backend::MemoryHeapTypes::UPLOAD;
    allocDesc.heapUsage = BIGOS::Driver::Backend::MemoryHeapUsages::BUFFERS;
    if( BGS_FAILED( pAPIDevice->AllocateMemory( allocDesc, &hBufferMem ) ) )
    {
        return -1;
    }

    BIGOS::Driver::Backend::BindResourceMemoryDesc bindMemDesc;
    bindMemDesc.hMemory      = hBufferMem;
    bindMemDesc.hResource    = hVertexBuffer;
    bindMemDesc.memoryOffset = 0;
    if( BGS_FAILED( pAPIDevice->BindResourceMemory( bindMemDesc ) ) )
    {
        return -1;
    }

    void*                                   pHostAccess = nullptr;
    BIGOS::Driver::Backend::MapResourceDesc mapVertex;
    mapVertex.hResource          = hVertexBuffer;
    mapVertex.bufferRange.size   = 3 * 8 * sizeof( float );
    mapVertex.bufferRange.offset = 0;

    if( BGS_FAILED( pAPIDevice->MapResource( mapVertex, &pHostAccess ) ) )
    {
        return -1;
    }

    float vertexData[] = {
        -0.5f, -0.5f, 0.0f, 1.0f /* POSITION */, 1.0f, 0.0f, 0.0f, 1.0f, /* COLOR */
        0.5f,  -0.5f, 0.0f, 1.0f /* POSITION */, 0.0f, 1.0f, 0.0f, 1.0f, /* COLOR */
        0.0f,  0.5f,  0.0f, 1.0f /* POSITION */, 0.0f, 0.0f, 1.0f, 1.0f, /* COLOR */
    };

    BIGOS::Memory::Copy( vertexData, sizeof( vertexData ), pHostAccess, sizeof( vertexData ) );

    if( BGS_FAILED( pAPIDevice->UnmapResource( mapVertex ) ) )
    {
        return -1;
    }

    uint32_t currentFrameNdx = 0;
    // Renders and display frame for 10 seconds
    for( auto start = std::chrono::steady_clock::now(), now = start; now < start + std::chrono::seconds{ 10 };
         now = std::chrono::steady_clock::now() )
    {
        BIGOS::Driver::Backend::FrameInfo frameInfo;
        pSwapchain->GetNextFrame( &frameInfo );
        const uint32_t bufferNdx = frameInfo.swapChainBackBufferIndex;

        BIGOS::Driver::Backend::WaitForFencesDesc waitDesc;
        waitDesc.fenceCount  = 1;
        waitDesc.waitAll     = BIGOS::Core::BGS_TRUE;
        waitDesc.pFences     = &hFences[ currentFrameNdx ];
        waitDesc.pWaitValues = &fenceValues[ currentFrameNdx ];
        if( BGS_FAILED( pAPIDevice->WaitForFences( waitDesc, UINT64_MAX ) ) )
        {
            return -1;
        }
        fenceValues[ currentFrameNdx ]++;
        pWnd->Update();
        if( BGS_FAILED( pAPIDevice->ResetCommandPool( hCmdPools[ bufferNdx ] ) ) )
        {
            return -1;
        }

        // Recording commands needed to render frame
        BIGOS::Driver::Backend::BeginCommandBufferDesc beginDesc;
        pCmdBuffers[ bufferNdx ]->Begin( beginDesc );

        BIGOS::Driver::Backend::TextureBarrierDesc preTexBarrier;
        preTexBarrier.srcStage     = BGS_FLAG( BIGOS::Driver::Backend::PipelineStageFlagBits::RENDER_TARGET );
        preTexBarrier.srcAccess    = BGS_FLAG( BIGOS::Driver::Backend::AccessFlagBits::GENERAL );
        preTexBarrier.srcLayout    = BIGOS::Driver::Backend::TextureLayouts::PRESENT;
        preTexBarrier.dstStage     = BGS_FLAG( BIGOS::Driver::Backend::PipelineStageFlagBits::RENDER_TARGET );
        preTexBarrier.dstAccess    = BGS_FLAG( BIGOS::Driver::Backend::AccessFlagBits::RENDER_TARGET );
        preTexBarrier.dstLayout    = BIGOS::Driver::Backend::TextureLayouts::RENDER_TARGET;
        preTexBarrier.hResouce     = backBuffers[ bufferNdx ].hBackBuffer;
        preTexBarrier.textureRange = rtRange;

        BIGOS::Driver::Backend::BarierDesc preBarrier;
        preBarrier.textureBarrierCount = 1;
        preBarrier.pTextureBarriers    = &preTexBarrier;
        preBarrier.globalBarrierCount  = 0;
        preBarrier.pGlobalBarriers     = nullptr;
        preBarrier.bufferBarrierCount  = 0;
        preBarrier.pBufferBarriers     = nullptr;

        pCmdBuffers[ bufferNdx ]->Barrier( preBarrier );

        BIGOS::Driver::Backend::BeginRenderingDesc renderingDesc;
        renderingDesc.colorRenderTargetCount   = 1;
        renderingDesc.phColorRenderTargetViews = &hRtvs[ bufferNdx ];
        renderingDesc.hDepthStencilTargetView  = BIGOS::Driver::Backend::ResourceViewHandle();
        renderingDesc.renderArea.offset        = { 0, 0 };
        renderingDesc.renderArea.size          = { pWnd->GetDesc().width, pWnd->GetDesc().height };
        pCmdBuffers[ bufferNdx ]->BeginRendering( renderingDesc );

        pCmdBuffers[ bufferNdx ]->SetPipeline( hPipeline, BIGOS::Driver::Backend::PipelineTypes::GRAPHICS );

        pCmdBuffers[ bufferNdx ]->SetPrimitiveTopology( BIGOS::Driver::Backend::PrimitiveTopologies::TRIANGLE_LIST );

        BIGOS::Driver::Backend::ViewportDesc vp;
        vp.upperLeftX = 0.0f;
        vp.upperLeftY = 0.0f;
        vp.width      = static_cast<float>( pWnd->GetDesc().width );
        vp.height     = static_cast<float>( pWnd->GetDesc().height );
        vp.minDepth   = 0.0f;
        vp.maxDepth   = 1.0f;
        pCmdBuffers[ bufferNdx ]->SetViewports( 1, &vp );

        BIGOS::Driver::Backend::ScissorDesc sr;
        sr.upperLeftX = 0;
        sr.upperLeftY = 0;
        sr.width      = pWnd->GetDesc().width;
        sr.height     = pWnd->GetDesc().height;
        pCmdBuffers[ bufferNdx ]->SetScissors( 1, &sr );

        BIGOS::Driver::Backend::ColorValue clrColor;
        clrColor.r = 0.0f;
        clrColor.g = 0.0f;
        clrColor.b = 0.0f;
        clrColor.a = 0.0f;

        BIGOS::Core::Rect2D clrRect;
        clrRect.offset = { 0, 0 };
        clrRect.size   = { pWnd->GetDesc().width, pWnd->GetDesc().height };
        pCmdBuffers[ bufferNdx ]->ClearBoundColorRenderTarget( 0, clrColor, 1, &clrRect );

        BIGOS::Driver::Backend::VertexBufferDesc vbBindDesc;
        vbBindDesc.hVertexBuffer = hVertexBuffer;
        vbBindDesc.offset        = 0;
        vbBindDesc.size          = 3 * 8 * sizeof( float );
        vbBindDesc.elementStride = 8 * sizeof( float );
        pCmdBuffers[ bufferNdx ]->SetVertexBuffers( 0, 1, &vbBindDesc );

        BIGOS::Driver::Backend::DrawDesc drawDesc;
        drawDesc.vertexCount   = 3;
        drawDesc.instanceCount = 1;
        drawDesc.firstVertex   = 0;
        drawDesc.firstInstance = 0;
        drawDesc.vertexOffset  = 0;

        pCmdBuffers[ bufferNdx ]->Draw( drawDesc );

        pCmdBuffers[ bufferNdx ]->EndRendering();

        BIGOS::Driver::Backend::TextureBarrierDesc postTexBarrier;
        postTexBarrier.srcStage     = BGS_FLAG( BIGOS::Driver::Backend::PipelineStageFlagBits::RENDER_TARGET );
        postTexBarrier.srcAccess    = BGS_FLAG( BIGOS::Driver::Backend::AccessFlagBits::RENDER_TARGET );
        postTexBarrier.srcLayout    = BIGOS::Driver::Backend::TextureLayouts::RENDER_TARGET;
        postTexBarrier.dstStage     = BGS_FLAG( BIGOS::Driver::Backend::PipelineStageFlagBits::RENDER_TARGET );
        postTexBarrier.dstAccess    = BGS_FLAG( BIGOS::Driver::Backend::AccessFlagBits::GENERAL );
        postTexBarrier.dstLayout    = BIGOS::Driver::Backend::TextureLayouts::PRESENT;
        postTexBarrier.hResouce     = backBuffers[ bufferNdx ].hBackBuffer;
        postTexBarrier.textureRange = rtRange;

        BIGOS::Driver::Backend::BarierDesc postBarrier;
        postBarrier.textureBarrierCount = 1;
        postBarrier.pTextureBarriers    = &postTexBarrier;
        postBarrier.globalBarrierCount  = 0;
        postBarrier.pGlobalBarriers     = nullptr;
        postBarrier.bufferBarrierCount  = 0;
        postBarrier.pBufferBarriers     = nullptr;

        pCmdBuffers[ bufferNdx ]->Barrier( postBarrier );

        pCmdBuffers[ bufferNdx ]->End();

        // Execute commands
        BIGOS::Driver::Backend::QueueSubmitDesc submitDesc;
        submitDesc.waitSemaphoreCount   = 1;
        submitDesc.phWaitSemaphores     = &frameInfo.hBackBufferAvailableSemaphore;
        submitDesc.waitFenceCount       = 0;
        submitDesc.phWaitFences         = nullptr;
        submitDesc.pWaitValues          = nullptr;
        submitDesc.commandBufferCount   = 1;
        submitDesc.ppCommandBuffers     = &pCmdBuffers[ bufferNdx ];
        submitDesc.signalSemaphoreCount = 1;
        submitDesc.phSignalSemaphores   = &hSemaphores[ currentFrameNdx ];
        submitDesc.signalFenceCount     = 1;
        submitDesc.phSignalFences       = &hFences[ currentFrameNdx ];
        submitDesc.pSignalValues        = &fenceValues[ currentFrameNdx ];
        if( BGS_FAILED( pGraphicsQueue->Submit( submitDesc ) ) )
        {
            return -1;
        }

        // Presents frame
        BIGOS::Driver::Backend::SwapchainPresentDesc presentDesc;
        presentDesc.waitSemaphoreCount = 1;
        presentDesc.pWaitSemaphores    = &hSemaphores[ currentFrameNdx ];
        if( BGS_FAILED( pSwapchain->Present( presentDesc ) ) )
        {
            return -1;
        }

        currentFrameNdx = ( currentFrameNdx + 1 ) % FRAMES_IN_FLIGHT_COUNT;
    }

    BIGOS::Driver::Backend::WaitForFencesDesc waitDesc;
    waitDesc.fenceCount  = FRAMES_IN_FLIGHT_COUNT;
    waitDesc.waitAll     = BIGOS::Core::BGS_TRUE;
    waitDesc.pFences     = hFences;
    waitDesc.pWaitValues = fenceValues;
    if( BGS_FAILED( pAPIDevice->WaitForFences( waitDesc, UINT64_MAX ) ) )
    {
        return -1;
    }

    pAPIDevice->DestroyResource( &hVertexBuffer );
    pAPIDevice->FreeMemory( &hBufferMem );

    pAPIDevice->DestroyShader( &hPS );
    pAPIDevice->DestroyShader( &hVS );
    pAPIDevice->DestroyPipeline( &hPipeline );
    pAPIDevice->DestroyPipelineLayout( &hEmptyLayout );
    pAPIDevice->DestroyBindingHeapLayout( &hEmptyBindingLayout );

    for( uint32_t ndx = 0; ndx < BACK_BUFFER_COUNT; ++ndx )
    {
        pAPIDevice->DestroyResourceView( &hRtvs[ ndx ] );
        pAPIDevice->DestroyCommandBuffer( &pCmdBuffers[ ndx ] );
        pAPIDevice->DestroyCommandPool( &hCmdPools[ ndx ] );
    }

    pAPIDevice->DestroySwapchain( &pSwapchain );

    for( uint32_t ndx = 0; ndx < FRAMES_IN_FLIGHT_COUNT; ++ndx )
    {
        pAPIDevice->DestroySemaphore( &hSemaphores[ ndx ] );
        pAPIDevice->DestroyFence( &hFences[ ndx ] );
    }

    const auto& adapters = pRenderSystem->GetFactory()->GetAdapters();
    adapters;

    pAPIDevice->DestroyQueue( &pGraphicsQueue );
    DestroyBigosFramework( &pFramework );

    printf( "Hello bigos\n" );

    return 0;
}
