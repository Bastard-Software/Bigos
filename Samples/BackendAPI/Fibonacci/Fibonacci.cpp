#include "Fibonacci.h"

#include "Application.h"
#include "Driver/Frontend/RenderSystem.h"
#include "Driver/Frontend/Shader/IShaderCompiler.h"
#include "Platform/Window.h"

Fibonacci::Fibonacci( APITypes APIType, uint32_t width, uint32_t height, const char* pName )
    : Sample( APIType, width, height, pName )
    , m_pRenderSystem( nullptr )
    , m_pDevice( nullptr )
    , m_pAPIDevice( nullptr )
    , m_pQueue( nullptr )
    , m_hComputeShader()
    , m_hShaderResourceSetLayout()
    , m_hShaderResourceHeap()
    , m_hPipelineLayout()
    , m_hPipeline()
    , m_sbvOffset( 0 )
    , m_cbvOffset( 0 )
    , m_hCommandPool()
    , m_pCommandBuffer( nullptr )
    , m_hStorageBuffer()
    , m_hStorageBufferMemory()
    , m_hStorageBufferView()
    , m_hComputeFinishedFence()
    , m_fenceValue( 0 )
{
    for (uint32_t ndx = 0; ndx < BUFFER_ELEMENT_COUNT; ++ndx)
    {
        m_storageBufferData.data[ ndx ] = ndx;
    }
}

BIGOS::RESULT Fibonacci::OnInit()
{
    if( BGS_FAILED( InitDevice() ) )
    {
        return BIGOS::Results::FAIL;
    }

    if( BGS_FAILED( CreateApplicationResources() ) )
    {
        return BIGOS::Results::FAIL;
    }

    if( BGS_FAILED( CreatePipeline() ) )
    {
        return BIGOS::Results::FAIL;
    }

    if( BGS_FAILED( RecordComputeCommandBuffer() ) )
    {
        return BIGOS::Results::FAIL;
    }

    if( BGS_FAILED( PrintResult() ) )
    {
        return BIGOS::Results::FAIL;
    }

    return BIGOS::Results::OK;
}

void Fibonacci::OnUpdate()
{
    // Not needed in this sample
}

void Fibonacci::OnRender()
{
    // Not needed in this sample
}

void Fibonacci::OnDestroy()
{
    m_pAPIDevice->DestroyFence( &m_hComputeFinishedFence );
    m_pAPIDevice->DestroyResource( &m_hStorageBuffer );
    m_pAPIDevice->FreeMemory( &m_hStorageBufferMemory );
    m_pAPIDevice->DestroyResourceView( &m_hStorageBufferView );
    m_pAPIDevice->DestroyShader( &m_hComputeShader );
    m_pAPIDevice->DestroyPipeline( &m_hPipeline );
    m_pAPIDevice->DestroyPipelineLayout( &m_hPipelineLayout );
    m_pAPIDevice->DestroyBindingHeap( &m_hShaderResourceHeap );
    m_pAPIDevice->DestroyBindingSetLayout( &m_hShaderResourceSetLayout );
    m_pAPIDevice->DestroyCommandBuffer( &m_pCommandBuffer );
    m_pAPIDevice->DestroyCommandPool( &m_hCommandPool );
    m_pAPIDevice->DestroyQueue( &m_pQueue );
}

BIGOS::RESULT Fibonacci::InitDevice()
{
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
    BIGOS::Driver::Backend::FenceDesc fenceDesc;
    fenceDesc.initialValue = m_fenceValue;
    if( BGS_FAILED( m_pAPIDevice->CreateFence( fenceDesc, &m_hComputeFinishedFence ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Queue and swapchain
    BIGOS::Driver::Backend::QueueDesc queueDesc;
    queueDesc.type     = BIGOS::Driver::Backend::QueueTypes::GRAPHICS;
    queueDesc.priority = BIGOS::Driver::Backend::QueuePriorityTypes::NORMAL;
    if( BGS_FAILED( m_pAPIDevice->CreateQueue( queueDesc, &m_pQueue ) ) )
    {
        return BIGOS::Results::OK;
    }

    return BIGOS::Results::OK;
}

BIGOS::RESULT Fibonacci::CreateApplicationResources()
{
    // Creating command buffer and command pool here as we do not generate frames
    BIGOS::Driver::Backend::CommandPoolDesc cmdPoolDesc;
    cmdPoolDesc.pQueue = m_pQueue;
    if( BGS_FAILED( m_pAPIDevice->CreateCommandPool( cmdPoolDesc, &m_hCommandPool ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Driver::Backend::CommandBufferDesc cmdBufferDesc;
    cmdBufferDesc.hCommandPool = m_hCommandPool;
    cmdBufferDesc.level        = BIGOS::Driver::Backend::CommandBufferLevels::PRIMARY;
    cmdBufferDesc.pQueue       = m_pQueue;
    if( BGS_FAILED( m_pAPIDevice->CreateCommandBuffer( cmdBufferDesc, &m_pCommandBuffer ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Driver::Backend::ResourceDesc sbDesc;
    sbDesc.arrayLayerCount = 1;
    sbDesc.format          = BIGOS::Driver::Backend::Formats::UNKNOWN;
    sbDesc.mipLevelCount   = 1;
    sbDesc.resourceLayout  = BIGOS::Driver::Backend::ResourceLayouts::LINEAR;
    sbDesc.resourceType    = BIGOS::Driver::Backend::ResourceTypes::BUFFER;
    sbDesc.resourceUsage   = BGS_FLAG( BIGOS::Driver::Backend::ResourceUsageFlagBits::READ_WRITE_STORAGE_BUFFER );
    sbDesc.sampleCount     = BIGOS::Driver::Backend::SampleCount::COUNT_1;
    sbDesc.sharingMode     = BIGOS::Driver::Backend::ResourceSharingModes::EXCLUSIVE_ACCESS;
    sbDesc.size.width      = sizeof( m_storageBufferData );
    sbDesc.size.height     = 1;
    sbDesc.size.depth      = 1;
    if( BGS_FAILED( m_pAPIDevice->CreateResource( sbDesc, &m_hStorageBuffer ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Driver::Backend::ResourceAllocationInfo sbAllocInfo;
    m_pAPIDevice->GetResourceAllocationInfo( m_hStorageBuffer, &sbAllocInfo );

    BIGOS::Driver::Backend::AllocateMemoryDesc allocDesc;
    allocDesc.size      = sbAllocInfo.size;
    allocDesc.alignment = sbAllocInfo.alignment;
    allocDesc.heapType  = BIGOS::Driver::Backend::MemoryHeapTypes::READBACK;
    allocDesc.heapUsage = BIGOS::Driver::Backend::MemoryHeapUsages::BUFFERS;
    if( BGS_FAILED( m_pAPIDevice->AllocateMemory( allocDesc, &m_hStorageBufferMemory ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Driver::Backend::BindResourceMemoryDesc bindMemDesc;
    bindMemDesc.hMemory      = m_hStorageBufferMemory;
    bindMemDesc.hResource    = m_hStorageBuffer;
    bindMemDesc.memoryOffset = 0;
    if( BGS_FAILED( m_pAPIDevice->BindResourceMemory( bindMemDesc ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    void*                                   pHostAccess = nullptr;
    BIGOS::Driver::Backend::MapResourceDesc mapStorage;
    mapStorage.hResource          = m_hStorageBuffer;
    mapStorage.bufferRange.size   = sizeof( m_storageBufferData );
    mapStorage.bufferRange.offset = 0;

    if( BGS_FAILED( m_pAPIDevice->MapResource( mapStorage, &pHostAccess ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Memory::Copy( &m_storageBufferData, sizeof( m_storageBufferData ), pHostAccess, sizeof( m_storageBufferData ) );

    if( BGS_FAILED( m_pAPIDevice->UnmapResource( mapStorage ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Driver::Backend::BufferViewDesc sbViewDesc;
    sbViewDesc.hResource    = m_hStorageBuffer;
    sbViewDesc.usage        = BGS_FLAG( BIGOS::Driver::Backend::ResourceViewUsageFlagBits::READ_WRITE_STORAGE_BUFFER );
    sbViewDesc.range.offset = 0;
    sbViewDesc.range.size   = sizeof( m_storageBufferData );
    if( BGS_FAILED( m_pAPIDevice->CreateResourceView( sbViewDesc, &m_hStorageBufferView ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    return BIGOS::Results::OK;
}

BIGOS::RESULT Fibonacci::CreatePipeline()
{
    void*    pShader = nullptr;
    uint32_t size    = 0;
    // Allocates memory that needs to be freed
    if( BGS_FAILED( readDataFromFile( "../Samples/BackendAPI/Fibonacci/shader.hlsl", &pShader, &size ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    const wchar_t* pArgs = L"-Wignored-attributes";

    BIGOS::Driver::Frontend::ShaderCompilerOutput* pCSBlob = nullptr;
    BIGOS::Driver::Frontend::CompileShaderDesc     shaderDesc;
    shaderDesc.argCount           = 0;
    shaderDesc.ppArgs             = &pArgs;
    shaderDesc.outputFormat       = m_APIType == BIGOS::Driver::Backend::APITypes::D3D12 ? BIGOS::Driver::Frontend::ShaderFormats::DXIL
                                                                                         : BIGOS::Driver::Frontend::ShaderFormats::SPIRV;
    shaderDesc.model              = BIGOS::Driver::Frontend::ShaderModels::SHADER_MODEL_6_5;
    shaderDesc.source.pSourceCode = pShader;
    shaderDesc.source.sourceSize  = size;
    shaderDesc.type               = BIGOS::Driver::Backend::ShaderTypes::COMPUTE;
    shaderDesc.pEntryPoint        = "CSMain";
    shaderDesc.compileDebug       = BIGOS::BGS_FALSE;
    if( BGS_FAILED( m_pRenderSystem->GetDefaultCompiler()->Compile( shaderDesc, &pCSBlob ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Driver::Backend::ShaderDesc shDesc;
    shDesc.pByteCode = pCSBlob->pByteCode;
    shDesc.codeSize  = pCSBlob->byteCodeSize;
    if( BGS_FAILED( m_pAPIDevice->CreateShader( shDesc, &m_hComputeShader ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    m_pRenderSystem->GetDefaultCompiler()->DestroyOutput( &pCSBlob );

    delete[] pShader;

    // Binding set layout
    BIGOS::Driver::Backend::BindingRangeDesc bindingRange;
    bindingRange.baseBindingSlot    = 0;
    bindingRange.baseShaderRegister = 0;
    bindingRange.bindingCount       = 1;
    bindingRange.type               = BIGOS::Driver::Backend::BindingTypes::READ_WRITE_STORAGE_BUFFER;
    BIGOS::Driver::Backend::BindingSetLayoutDesc bindingLayoutDesc;
    bindingLayoutDesc.visibility        = BIGOS::Driver::Backend::ShaderVisibilities::COMPUTE;
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
    m_pAPIDevice->GetBindingOffset( getAddressDesc, &m_sbvOffset );

    BIGOS::Driver::Backend::WriteBindingDesc writeBindingDesc;
    writeBindingDesc.bindingType   = BindingTypes::READ_WRITE_STORAGE_BUFFER;
    writeBindingDesc.hDstHeap      = m_hShaderResourceHeap;
    writeBindingDesc.dstOffset     = m_sbvOffset;
    writeBindingDesc.hResourceView = m_hStorageBufferView;
    m_pAPIDevice->WriteBinding( writeBindingDesc );

    // Push constants description
    BIGOS::Driver::Backend::PushConstantRangeDesc pushConstantRange;
    pushConstantRange.baseConstantSlot = 0;
    pushConstantRange.constantCount    = 1;
    pushConstantRange.shaderRegister   = 0;
    pushConstantRange.visibility       = BIGOS::Driver::Backend::ShaderVisibilities::COMPUTE;

    // Pipeline layout
    BIGOS::Driver::Backend::PipelineLayoutDesc pipelineLayoutDesc;
    pipelineLayoutDesc.constantRangeCount    = 1;
    pipelineLayoutDesc.bindingSetLayoutCount = 1;
    pipelineLayoutDesc.phBindigSetLayouts    = &m_hShaderResourceSetLayout;
    pipelineLayoutDesc.pConstantRanges       = &pushConstantRange;
    if( BGS_FAILED( m_pAPIDevice->CreatePipelineLayout( pipelineLayoutDesc, &m_hPipelineLayout ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Pipeline
    BIGOS::Driver::Backend::ComputePipelineDesc pipelineDesc;
    // Type
    pipelineDesc.type = BIGOS::Driver::Backend::PipelineTypes::COMPUTE;
    // Shaders
    pipelineDesc.computeShader.hShader     = m_hComputeShader;
    pipelineDesc.computeShader.pEntryPoint = "CSMain";
    // PL
    pipelineDesc.hPipelineLayout = m_hPipelineLayout;
    if( BGS_FAILED( m_pAPIDevice->CreatePipeline( pipelineDesc, &m_hPipeline ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    return BIGOS::Results::OK;
}

BIGOS::RESULT Fibonacci::RecordComputeCommandBuffer()
{
    if( BGS_FAILED( m_pAPIDevice->ResetCommandPool( m_hCommandPool ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    // Compute commands
    BIGOS::Driver::Backend::BeginCommandBufferDesc beginDesc;
    m_pCommandBuffer->Begin( beginDesc );

    BIGOS::Driver::Backend::BufferBarrierDesc preDispatchBarrier;
    preDispatchBarrier.srcStage           = BGS_FLAG( BIGOS::Driver::Backend::PipelineStageFlagBits::NONE );
    preDispatchBarrier.srcAccess          = BGS_FLAG( BIGOS::Driver::Backend::AccessFlagBits::NONE );
    preDispatchBarrier.dstStage           = BGS_FLAG( BIGOS::Driver::Backend::PipelineStageFlagBits::COMPUTE_SHADING );
    preDispatchBarrier.dstAccess          = BGS_FLAG( BIGOS::Driver::Backend::AccessFlagBits::SHADER_READ_WRITE );
    preDispatchBarrier.hResouce           = m_hStorageBuffer;
    preDispatchBarrier.bufferRange.offset = 0;
    preDispatchBarrier.bufferRange.size   = sizeof( m_storageBufferData );

    BIGOS::Driver::Backend::BarierDesc preBarrier;
    preBarrier.textureBarrierCount = 0;
    preBarrier.pTextureBarriers    = nullptr;
    preBarrier.globalBarrierCount  = 0;
    preBarrier.pGlobalBarriers     = nullptr;
    preBarrier.bufferBarrierCount  = 1;
    preBarrier.pBufferBarriers     = &preDispatchBarrier;
    m_pCommandBuffer->Barrier( preBarrier );

    m_pCommandBuffer->SetPipeline( m_hPipeline, BIGOS::Driver::Backend::PipelineTypes::COMPUTE );

    m_pCommandBuffer->SetBindingHeaps( 1, &m_hShaderResourceHeap );

    BIGOS::Driver::Backend::SetBindingsDesc setBindingDesc;
    setBindingDesc.baseBindingOffset = m_sbvOffset;
    setBindingDesc.heapNdx           = 0;
    setBindingDesc.setSpaceNdx       = 0;
    setBindingDesc.hPipelineLayout   = m_hPipelineLayout;
    setBindingDesc.type              = BIGOS::Driver::Backend::PipelineTypes::COMPUTE;
    m_pCommandBuffer->SetBindings( setBindingDesc );

    BIGOS::Driver::Backend::PushConstantsDesc pushConstantDesc;
    pushConstantDesc.type             = BIGOS::Driver::Backend::PipelineTypes::COMPUTE;
    pushConstantDesc.hPipelineLayout  = m_hPipelineLayout;
    pushConstantDesc.shaderVisibility = BIGOS::Driver::Backend::ShaderVisibilities::COMPUTE;
    pushConstantDesc.firstConstant    = 0;
    pushConstantDesc.constantCount    = 1;
    pushConstantDesc.pData            = &BUFFER_ELEMENT_COUNT;
    m_pCommandBuffer->PushConstants( pushConstantDesc );

    BIGOS::Driver::Backend::DispatchDesc dispatchDesc;
    dispatchDesc.groupCountX = BUFFER_ELEMENT_COUNT;
    dispatchDesc.groupCountY = 1;
    dispatchDesc.groupCountZ = 1;
    m_pCommandBuffer->Dispatch( dispatchDesc );

    BIGOS::Driver::Backend::BufferBarrierDesc postDispatchBarrier;
    postDispatchBarrier.srcStage           = BGS_FLAG( BIGOS::Driver::Backend::PipelineStageFlagBits::COMPUTE_SHADING );
    postDispatchBarrier.srcAccess          = BGS_FLAG( BIGOS::Driver::Backend::AccessFlagBits::SHADER_READ_WRITE );
    postDispatchBarrier.dstStage           = BGS_FLAG( BIGOS::Driver::Backend::PipelineStageFlagBits::TRANSFER );
    postDispatchBarrier.dstAccess          = BGS_FLAG( BIGOS::Driver::Backend::AccessFlagBits::GENERAL );
    postDispatchBarrier.hResouce           = m_hStorageBuffer;
    postDispatchBarrier.bufferRange.offset = 0;
    postDispatchBarrier.bufferRange.size   = sizeof( m_storageBufferData );

    BIGOS::Driver::Backend::BarierDesc postBarrier;
    postBarrier.textureBarrierCount = 0;
    postBarrier.pTextureBarriers    = nullptr;
    postBarrier.globalBarrierCount  = 0;
    postBarrier.pGlobalBarriers     = nullptr;
    postBarrier.bufferBarrierCount  = 1;
    postBarrier.pBufferBarriers     = &postDispatchBarrier;
    m_pCommandBuffer->Barrier( postBarrier );

    m_pCommandBuffer->End();

    ++m_fenceValue;
    // Execute commands
    BIGOS::Driver::Backend::QueueSubmitDesc submitDesc;
    submitDesc.waitSemaphoreCount   = 0;
    submitDesc.phWaitSemaphores     = nullptr;
    submitDesc.waitFenceCount       = 0;
    submitDesc.phWaitFences         = nullptr;
    submitDesc.pWaitValues          = nullptr;
    submitDesc.commandBufferCount   = 1;
    submitDesc.ppCommandBuffers     = &m_pCommandBuffer;
    submitDesc.signalSemaphoreCount = 0;
    submitDesc.phSignalSemaphores   = nullptr;
    submitDesc.signalFenceCount     = 1;
    submitDesc.phSignalFences       = &m_hComputeFinishedFence;
    submitDesc.pSignalValues        = &m_fenceValue;
    if( BGS_FAILED( m_pQueue->Submit( submitDesc ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Driver::Backend::WaitForFencesDesc waitDesc;
    waitDesc.fenceCount  = 1;
    waitDesc.waitAll     = BIGOS::Core::BGS_FALSE;
    waitDesc.pFences     = &m_hComputeFinishedFence;
    waitDesc.pWaitValues = &m_fenceValue;
    if( BGS_FAILED( m_pAPIDevice->WaitForFences( waitDesc, UINT64_MAX ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    return BIGOS::Results::OK;
}

BIGOS::RESULT Fibonacci::PrintResult()
{
    void*                                   pHostAccess = nullptr;
    BIGOS::Driver::Backend::MapResourceDesc mapStorage;
    mapStorage.hResource          = m_hStorageBuffer;
    mapStorage.bufferRange.size   = sizeof( m_storageBufferData );
    mapStorage.bufferRange.offset = 0;

    if( BGS_FAILED( m_pAPIDevice->MapResource( mapStorage, &pHostAccess ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    BIGOS::Memory::Copy( pHostAccess, sizeof( m_storageBufferData ), &m_storageBufferData, sizeof( m_storageBufferData ) );

    if( BGS_FAILED( m_pAPIDevice->UnmapResource( mapStorage ) ) )
    {
        return BIGOS::Results::FAIL;
    }

    printf( "\nComputed %d fibonacci values:\n", BUFFER_ELEMENT_COUNT );
    for( uint32_t ndx = 0; ndx < BUFFER_ELEMENT_COUNT; ++ndx )
    {
        printf( "\t %d |", m_storageBufferData.data[ ndx ] );
    }
    printf( "\n" );

    return BIGOS::Results::OK;
}
