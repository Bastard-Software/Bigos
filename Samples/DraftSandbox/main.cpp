#include "BigosFramework/BigosFramework.h"
#include "Core/Memory/SystemHeapAllocator.h"
#include "Core/Utils/String.h"
#include "Driver/Frontend/RenderDevice.h"
#include "Driver/Frontend/RenderSystem.h"
#include "Driver/Frontend/Shader/IShaderCompiler.h"
#include "Platform/Window.h"
#include "Platform/WindowSystem.h"
#include <stdio.h>

const char* SHADER = "static const float4 positions[ 3 ] = { float4( -0.5f, -0.5f, 0.0f, 1.0f ), float4( 0.5f, -0.5f, 0.0f, 1.0f ), float4( 0.0f, "
                     "0.5f, 0.0f, 1.0f ) };\n"

                     "float4 VSMain(uint vertexID : SV_VertexID) : SV_Position\n"
                     "{\n"
                     "  return positions[ vertexID ];\n"
                     "}\n\n"

                     "float4 PSMain(float4 position : SV_Position ) : SV_Target\n"
                     "{\n"
                     "  return float4( 255.0f / 255.0f, 240.0f / 255.0f, 0.0f / 255.0f, 1.0f );"
                     "}\n";

const BIGOS::Driver::Backend::API_TYPE API_TYPE = BIGOS::Driver::Backend::APITypes::VULKAN;

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

    auto                                           compiler = pRenderSystem->GetDefaultCompiler();
    BIGOS::Driver::Frontend::ShaderCompilerOutput* pVSBlob  = nullptr;
    BIGOS::Driver::Frontend::ShaderCompilerOutput* pPSBlob  = nullptr;
    BIGOS::Driver::Frontend::CompileShaderDesc     shaderDesc;
    shaderDesc.argCount           = 0;
    shaderDesc.ppArgs             = nullptr;
    shaderDesc.outputFormat       = API_TYPE == BIGOS::Driver::Backend::APITypes::D3D12 ? BIGOS::Driver::Frontend::ShaderFormats::DXIL
                                                                                        : BIGOS::Driver::Frontend::ShaderFormats::SPIRV;
    shaderDesc.model              = BIGOS::Driver::Frontend::ShaderModels::SHADER_MODEL_6_7;
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

    auto blocks = pFramework->GetMemorySystem()->GetMemoryBlockInfo();

    BIGOS::Driver::Frontend::RenderDeviceDesc devDesc;
    devDesc.adapter.index                                = 0;
    BIGOS::Driver::Frontend::RenderDevice* pRenderDevice = nullptr;
    if( BGS_FAILED( pRenderSystem->CreateDevice( devDesc, &pRenderDevice ) ) )
    {
        return -1;
    }

    BIGOS::Driver::Backend::IDevice* pAPIDevice = pRenderDevice->GetNativeAPIDevice();

    BIGOS::Driver::Backend::ResourceHandle hVB;
    BIGOS::Driver::Backend::ResourceDesc   vbDesc;
    vbDesc.arrayLayerCount = 1;
    vbDesc.format          = BIGOS::Driver::Backend::Formats::UNKNOWN;
    vbDesc.mipLevelCount   = 1;
    vbDesc.resourceLayout  = BIGOS::Driver::Backend::ResourceLayouts::LINEAR;
    vbDesc.resourceType    = BIGOS::Driver::Backend::ResourceTypes::BUFFER;
    vbDesc.resourceUsage   = static_cast<uint32_t>( BIGOS::Driver::Backend::ResourceUsageFlagBits::VERTEX_BUFFER );
    vbDesc.sampleCount     = BIGOS::Driver::Backend::SampleCount::COUNT_1;
    vbDesc.sharingMode     = BIGOS::Driver::Backend::ResourceSharingModes::EXCLUSIVE_ACCESS;
    vbDesc.size.width      = 64;
    vbDesc.size.height     = 1;
    vbDesc.size.depth      = 1;
    if( BGS_FAILED( pAPIDevice->CreateResource( vbDesc, &hVB ) ) )
    {
        return -1;
    }

    BIGOS::Driver::Backend::ResourceAllocationInfo vbAllocInfo;
    pAPIDevice->GetResourceAllocationInfo( hVB, &vbAllocInfo );

    BIGOS::Driver::Backend::MemoryHandle       hBufferMem;
    BIGOS::Driver::Backend::MemoryHandle       hTextureMem;
    BIGOS::Driver::Backend::MemoryHandle       hRTMem;
    BIGOS::Driver::Backend::AllocateMemoryDesc allocDesc;
    allocDesc.size      = vbAllocInfo.size;
    allocDesc.alignment = vbAllocInfo.alignment;
    allocDesc.heapType  = BIGOS::Driver::Backend::MemoryHeapTypes::UPLOAD;
    allocDesc.heapUsage = BIGOS::Driver::Backend::MemoryHeapUsages::BUFFERS;
    if( BGS_FAILED( pAPIDevice->AllocateMemory( allocDesc, &hBufferMem ) ) )
    {
        return -1;
    }
    allocDesc.heapType  = BIGOS::Driver::Backend::MemoryHeapTypes::READBACK;
    allocDesc.heapUsage = BIGOS::Driver::Backend::MemoryHeapUsages::TEXTURES;
    if( BGS_FAILED( pAPIDevice->AllocateMemory( allocDesc, &hTextureMem ) ) )
    {
        return -1;
    }
    allocDesc.heapType  = BIGOS::Driver::Backend::MemoryHeapTypes::DEFAULT;
    allocDesc.heapUsage = BIGOS::Driver::Backend::MemoryHeapUsages::RENDER_TARGETS;
    if( BGS_FAILED( pAPIDevice->AllocateMemory( allocDesc, &hRTMem ) ) )
    {
        return -1;
    }

    BIGOS::Driver::Backend::BindResourceMemoryDesc vbBindDesc;
    vbBindDesc.memoryOffset = 0;
    vbBindDesc.hMemory      = hBufferMem;
    vbBindDesc.hResource    = hVB;
    if( BGS_FAILED( pAPIDevice->BindResourceMemory( vbBindDesc ) ) )
    {
        return -1;
    }

    void*                                   pVB = nullptr;
    BIGOS::Driver::Backend::MapResourceDesc mapDesc;
    mapDesc.hResource          = hVB;
    mapDesc.bufferRange.offset = 0;
    mapDesc.bufferRange.size   = 64;
    if( BGS_FAILED( pAPIDevice->MapResource( mapDesc, &pVB ) ) )
    {
        return -1;
    }

    if( BGS_FAILED( pAPIDevice->UnmapResource( mapDesc ) ) )
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

    BIGOS::Driver::Backend::PipelineHandle       hPipeline;
    BIGOS::Driver::Backend::GraphicsPipelineDesc pipelineDesc;
    // Shaders
    pipelineDesc.vertexShader.hShader     = hVS;
    pipelineDesc.vertexShader.pEntryPoint = "VSMain";
    pipelineDesc.pixelShader.hShader      = hPS;
    pipelineDesc.pixelShader.pEntryPoint  = "PSMain";
    pipelineDesc.hullShader.hShader       = BIGOS::Driver::Backend::ShaderHandle();
    pipelineDesc.domainShader.hShader     = BIGOS::Driver::Backend::ShaderHandle();
    pipelineDesc.geometryShader.hShader   = BIGOS::Driver::Backend::ShaderHandle();
    // VI
    pipelineDesc.inputState.inputBindingCount = 0;
    pipelineDesc.inputState.inputElementCount = 0;
    pipelineDesc.inputState.pInputBindings    = nullptr;
    pipelineDesc.inputState.pInputElements    = nullptr;
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
    // BS
    BIGOS::Driver::Backend::RenderTargetBlendDesc rtb;
    rtb.blendEnable                                    = BIGOS::Core::BGS_FALSE;
    pipelineDesc.blendState.enableLogicOperations      = BIGOS::Core::BGS_FALSE;
    pipelineDesc.blendState.pRenderTargetBlendDescs    = &rtb;
    pipelineDesc.blendState.renderTargetBlendDescCount = 1;
    // RT
    BIGOS::Driver::Backend::FORMAT        rtFormats[ 1 ] = { BIGOS::Driver::Backend::Formats::B8G8R8A8_UNORM };
    const BIGOS::Driver::Backend::FORMAT* pFormat        = rtFormats;
    pipelineDesc.renderTargetCount                       = 1;
    pipelineDesc.pRenderTargetFormats                    = pFormat;
    pipelineDesc.renderTargetCount                       = 1;
    pipelineDesc.depthStencilFormat                      = BIGOS::Driver::Backend::Formats::UNKNOWN;
    // PL
    pipelineDesc.hPipelineLayout = BIGOS::Driver::Backend::PipelineLayoutHandle();
    if( BGS_FAILED( pAPIDevice->CreatePipeline( pipelineDesc, &hPipeline ) ) )
    {
        return -1;
    }

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

    BIGOS::Driver::Backend::CommandPoolHandle hCmdPool;
    BIGOS::Driver::Backend::CommandPoolDesc   cmdPoolDesc;
    cmdPoolDesc.pQueue = pGraphicsQueue;
    if( BGS_FAILED( pAPIDevice->CreateCommandPool( cmdPoolDesc, &hCmdPool ) ) )
    {
        return -1;
    }

    BIGOS::Driver::Backend::SemaphoreHandle hSemaphore;
    BIGOS::Driver::Backend::SemaphoreDesc   semaphoreDesc;
    if( BGS_FAILED( pAPIDevice->CreateSemaphore( semaphoreDesc, &hSemaphore ) ) )
    {
        return -1;
    }

    BIGOS::Driver::Backend::FenceHandle hFence;
    BIGOS::Driver::Backend::FenceDesc   fenceDesc;
    fenceDesc.initialValue = 0;
    if( BGS_FAILED( pAPIDevice->CreateFence( fenceDesc, &hFence ) ) )
    {
        return -1;
    }

    BIGOS::Driver::Backend::ICommandBuffer*   pCmdBuffer = nullptr;
    BIGOS::Driver::Backend::CommandBufferDesc cmdBufferDesc;
    cmdBufferDesc.hCommandPool = hCmdPool;
    cmdBufferDesc.level        = BIGOS::Driver::Backend::CommandBufferLevels::PRIMARY;
    cmdBufferDesc.pQueue       = pGraphicsQueue;
    if( BGS_FAILED( pAPIDevice->CreateCommandBuffer( cmdBufferDesc, &pCmdBuffer ) ) )
    {
        return -1;
    }

    BIGOS::Driver::Backend::SamplerHandle hSampler;
    BIGOS::Driver::Backend::SamplerDesc   samplerDesc;
    samplerDesc.minFilter        = BIGOS::Driver::Backend::FilterTypes::LINEAR;
    samplerDesc.magFilter        = BIGOS::Driver::Backend::FilterTypes::LINEAR;
    samplerDesc.mipMapFilter     = BIGOS::Driver::Backend::FilterTypes::LINEAR;
    samplerDesc.addressU         = BIGOS::Driver::Backend::TextureAddressModes::CLAMP_TO_BORDER;
    samplerDesc.addressV         = BIGOS::Driver::Backend::TextureAddressModes::CLAMP_TO_BORDER;
    samplerDesc.addressW         = BIGOS::Driver::Backend::TextureAddressModes::CLAMP_TO_BORDER;
    samplerDesc.anisotropyEnable = BIGOS::BGS_FALSE;
    samplerDesc.compareEnable    = BIGOS::BGS_FALSE;
    samplerDesc.compareOperation = BIGOS::Driver::Backend::CompareOperationTypes::NEVER;
    samplerDesc.reductionMode    = BIGOS::Driver::Backend::SamplerReductionModes::WEIGHTED_AVERAGE;
    samplerDesc.minLod           = 0;
    samplerDesc.maxLod           = 0;
    samplerDesc.mipLodBias       = 0;
    samplerDesc.borderColor.r    = 1.0f;
    samplerDesc.borderColor.g    = 1.0f;
    samplerDesc.borderColor.b    = 1.0f;
    samplerDesc.borderColor.a    = 1.0f;

    if( BGS_FAILED( pAPIDevice->CreateSampler( samplerDesc, &hSampler ) ) )
    {
        return -1;
    }

    // Bindings
    BIGOS::Driver::Backend::BindingRangeDesc immutableSamplerRange;
    immutableSamplerRange.type                        = BIGOS::Driver::Backend::BindingTypes::SAMPLER;
    immutableSamplerRange.baseShaderRegister          = 0;
    immutableSamplerRange.baseBindingSlot             = 0;
    immutableSamplerRange.bindingCount                = 1;
    immutableSamplerRange.immutableSampler.phSamplers = &hSampler;
    immutableSamplerRange.immutableSampler.visibility = BIGOS::Driver::Backend::ShaderVisibilities::ALL;

    BIGOS::Driver::Backend::BindingRangeDesc samplerRange;
    samplerRange.type                        = BIGOS::Driver::Backend::BindingTypes::SAMPLER;
    samplerRange.baseShaderRegister          = 1;
    samplerRange.baseBindingSlot             = 1;
    samplerRange.bindingCount                = 2;
    samplerRange.immutableSampler.phSamplers = nullptr;

    BIGOS::Driver::Backend::BindingRangeDesc cbRange;
    cbRange.type                        = BIGOS::Driver::Backend::BindingTypes::CONSTANT_BUFFER;
    cbRange.baseShaderRegister          = 0;
    cbRange.baseBindingSlot             = 4;
    cbRange.bindingCount                = 2;
    cbRange.immutableSampler.phSamplers = nullptr;

    BIGOS::Driver::Backend::BindingRangeDesc texRange;
    texRange.type                        = BIGOS::Driver::Backend::BindingTypes::SAMPLED_TEXTURE;
    texRange.baseShaderRegister          = 0;
    texRange.baseBindingSlot             = 6;
    texRange.bindingCount                = 2;
    texRange.immutableSampler.phSamplers = nullptr;

    BIGOS::Driver::Backend::BindingRangeDesc sbRange;
    sbRange.type                        = BIGOS::Driver::Backend::BindingTypes::READ_WRITE_STORAGE_BUFFER;
    sbRange.baseShaderRegister          = 0;
    sbRange.baseBindingSlot             = 8;
    sbRange.bindingCount                = 2;
    sbRange.immutableSampler.phSamplers = nullptr;

    BIGOS::Driver::Backend::BindingRangeDesc ranges[] = { immutableSamplerRange, samplerRange, cbRange, texRange, sbRange };

    BIGOS::Driver::Backend::BindingHeapLayoutHandle hBindingLayout;
    BIGOS::Driver::Backend::BindingHeapLayoutDesc   bindingLayoutDesc;
    bindingLayoutDesc.visibility        = BIGOS::Driver::Backend::ShaderVisibilities::ALL;
    bindingLayoutDesc.bindingRangeCount = 5;
    bindingLayoutDesc.pBindingRanges    = ranges;
    if( BGS_FAILED( pAPIDevice->CreateBindingHeapLayout( bindingLayoutDesc, &hBindingLayout ) ) )
    {
        return -1;
    }

    // TODO: Close into loop
    if( BGS_FAILED( pAPIDevice->ResetCommandPool( hCmdPool ) ) )
    {
        return -1;
    }
    pWnd->Update();

    BIGOS::Driver::Backend::BeginCommandBufferDesc beginDesc;
    pCmdBuffer->Begin( beginDesc );

    BIGOS::Driver::Backend::ViewportDesc vp;
    vp.upperLeftX = 0.0f;
    vp.upperLeftY = 0.0f;
    vp.width      = static_cast<float>( pWnd->GetDesc().width );
    vp.height     = static_cast<float>( pWnd->GetDesc().height );
    vp.minDepth   = 0.0f;
    vp.maxDepth   = 1.0f;
    pCmdBuffer->SetViewports( 1, &vp );

    BIGOS::Driver::Backend::ScissorDesc sr;
    sr.upperLeftX = 0;
    sr.upperLeftY = 0;
    sr.width      = pWnd->GetDesc().width;
    sr.height     = pWnd->GetDesc().height;
    pCmdBuffer->SetScissors( 1, &sr );

    pCmdBuffer->End();

    pCmdBuffer->Reset();

    uint64_t waitVal;
    if( BGS_FAILED( pAPIDevice->GetFenceValue( hFence, &waitVal ) ) )
    {
        return -1;
    }
    waitVal++;
    if( BGS_FAILED( pAPIDevice->SignalFence( waitVal, hFence ) ) )
    {
        return -1;
    }

    BIGOS::Driver::Backend::WaitForFencesDesc waitDesc;
    waitDesc.fenceCount  = 1;
    waitDesc.waitAll     = BIGOS::Core::BGS_TRUE;
    waitDesc.pFences     = &hFence;
    waitDesc.pWaitValues = &waitVal;
    if( BGS_FAILED( pAPIDevice->WaitForFences( waitDesc, UINT64_MAX ) ) )
    {
        return -1;
    }

    compiler->DestroyOutput( &pVSBlob );
    compiler->DestroyOutput( &pPSBlob );

    pAPIDevice->DestroySampler( &hSampler );
    pAPIDevice->DestroyBindingHeapLayout( &hBindingLayout );
    pAPIDevice->DestroySwapchain( &pSwapchain );
    pAPIDevice->DestroyResource( &hVB );
    pAPIDevice->FreeMemory( &hBufferMem );
    pAPIDevice->FreeMemory( &hTextureMem );
    pAPIDevice->FreeMemory( &hRTMem );
    pAPIDevice->DestroyShader( &hPS );
    pAPIDevice->DestroyShader( &hVS );
    pAPIDevice->DestroyPipeline( &hPipeline );
    pAPIDevice->DestroyCommandBuffer( &pCmdBuffer );
    pAPIDevice->DestroyCommandPool( &hCmdPool );
    pAPIDevice->DestroySemaphore( &hSemaphore );
    pAPIDevice->DestroyFence( &hFence );

    blocks = pFramework->GetMemorySystem()->GetMemoryBlockInfo();
    blocks;

    const auto& adapters = pRenderSystem->GetFactory()->GetAdapters();
    adapters;

    pAPIDevice->DestroyQueue( &pGraphicsQueue );
    DestroyBigosFramework( &pFramework );

    printf( "Hello bigos\n" );

    return 0;
}
