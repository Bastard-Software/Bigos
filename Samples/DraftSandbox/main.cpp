#include "BigosFramework/BigosFramework.h"
#include "Core/Memory/SystemHeapAllocator.h"
#include "Driver/Frontend/RenderDevice.h"
#include "Driver/Frontend/RenderSystem.h"
#include <stdio.h>

int main()
{
    BIGOS::BigosFramework*    pFramework = nullptr;
    BIGOS::BigosFrameworkDesc frameworkDesc;
    frameworkDesc.renderSystemDesc.factoryDesc.apiType = BIGOS::Driver::Backend::APITypes::VULKAN;
    frameworkDesc.renderSystemDesc.factoryDesc.flags   = 1;

    if( BGS_FAILED( CreateBigosFramework( frameworkDesc, &pFramework ) ) )
    {
        printf( "Framework initialization failed." );
        return -1;
    }

    BIGOS::Driver::Frontend::RenderSystem* pRenderSystem = nullptr;
    if( BGS_FAILED( pFramework->CreateRenderSystem( frameworkDesc.renderSystemDesc, &pRenderSystem ) ) )
    {
        return -1;
    }

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

    if( BGS_FAILED( pAPIDevice->ResetCommandPool( hCmdPool ) ) )
    {
        return -1;
    }

    pAPIDevice->DestroyCommandPool( &hCmdPool );

    blocks = pFramework->GetMemorySystemPtr()->GetMemoryBlockInfoPtrs();
    blocks;

    const auto& adapters = pRenderSystem->GetFactoryPtr()->GetAdapters();
    adapters;

    pAPIDevice->DestroyQueue( &pGraphicsQueue );
    DestroyBigosFramework( &pFramework );

    printf( "Hello bigos\n" );

    return 0;
}
