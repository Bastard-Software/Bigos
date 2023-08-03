#include "BigosFramework/BigosFramework.h"
#include "Core/Memory/SystemHeapAllocator.h"
#include "Driver/Frontend/RenderDevice.h"
#include "Driver/Frontend/RenderSystem.h"
#include "Platform/Window.h"
#include "Platform/WindowSystem.h"
#include <stdio.h>

int main()
{
    BIGOS::BigosFramework*    pFramework = nullptr;
    BIGOS::BigosFrameworkDesc frameworkDesc;
    frameworkDesc.renderSystemDesc.factoryDesc.apiType = BIGOS::Driver::Backend::APITypes::D3D12;
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

    auto blocks = pFramework->GetMemorySystemPtr()->GetMemoryBlockInfoPtrs();

    BIGOS::Driver::Frontend::RenderDeviceDesc devDesc;
    devDesc.adapter.index                                = 0;
    BIGOS::Driver::Frontend::RenderDevice* pRenderDevice = nullptr;
    if( BGS_FAILED( pRenderSystem->CreateDevice( devDesc, &pRenderDevice ) ) )
    {
        return -1;
    }

    BIGOS::Driver::Backend::IDevice* pAPIDevice = pRenderDevice->GetNativeAPIDevice();

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

    // TODO: Close into loop
    if( BGS_FAILED( pAPIDevice->ResetCommandPool( hCmdPool ) ) )
    {
        return -1;
    }
    pWnd->Update();

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

    pAPIDevice->DestroyCommandPool( &hCmdPool );
    pAPIDevice->DestroySemaphore( &hSemaphore );
    pAPIDevice->DestroyFence( &hFence );

    blocks = pFramework->GetMemorySystemPtr()->GetMemoryBlockInfoPtrs();
    blocks;

    const auto& adapters = pRenderSystem->GetFactoryPtr()->GetAdapters();
    adapters;

    pAPIDevice->DestroyQueue( &pGraphicsQueue );
    DestroyBigosFramework( &pFramework );

    printf( "Hello bigos\n" );

    return 0;
}
