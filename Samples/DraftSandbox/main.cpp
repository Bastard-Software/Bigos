#include "BigosFramework/BigosFramework.h"
#include "Core/Memory/SystemHeapAllocator.h"
#include "Core/Utils/String.h"
#include "Driver/Frontend/RenderDevice.h"
#include "Driver/Frontend/RenderSystem.h"
#include "Driver/Frontend/Shader/IShaderCompiler.h"
#include "Platform/Window.h"
#include "Platform/WindowSystem.h"
#include <stdio.h>

const char* SHADER = "struct PSInput\n"
                     "{\n"
                     "  float4 position : SV_POSITION;\n"
                     "  float4 color : COLOR;\n"
                     "};\n\n"
                     "PSInput VSMain(float4 position : POSITION, float4 color : COLOR)\n"
                     "{\n"
                     "  PSInput result;\n\n"
                     "  result.position = position;\n"
                     "  result.color = color;\n\n"
                     "  return result;\n"
                     "}\n\n"
                     "float4 PSMain(PSInput input) : SV_TARGET\n"
                     "{\n"
                     "  return input.color;\n"
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

    BIGOS::Driver::Backend::IQueue*   pGraphicsQueue = nullptr;
    BIGOS::Driver::Backend::QueueDesc queueDesc;
    queueDesc.type     = BIGOS::Driver::Backend::QueueTypes::GRAPHICS;
    queueDesc.priority = BIGOS::Driver::Backend::QueuePriorityTypes::NORMAL;

    if( BGS_FAILED( pAPIDevice->CreateQueue( queueDesc, &pGraphicsQueue ) ) )
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

    pAPIDevice->DestroyShader( &hPS );
    pAPIDevice->DestroyShader( &hVS );
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
