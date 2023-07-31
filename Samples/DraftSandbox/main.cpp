#include "BigosFramework/BigosFramework.h"
#include "Core/Memory/SystemHeapAllocator.h"
#include "Driver/Frontend/RenderSystem.h"
#include <stdio.h>

int main()
{
    printf( "Hello bigos\n" );

    BIGOS::BigosFramework*    pFramework = nullptr;
    BIGOS::BigosFrameworkDesc frameworkDesc;
    frameworkDesc.renderSystemDesc.factoryDesc.apiType = BIGOS::Driver::Backend::APITypes::D3D12;
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
    devDesc.adapter.index                          = 0;
    BIGOS::Driver::Frontend::RenderDevice* pDevice = nullptr;
    if( BGS_FAILED( pRenderSystem->CreateDevice( devDesc, &pDevice ) ) )
    {
        return -1;
    }
    devDesc.adapter.index = 1;

    blocks = pFramework->GetMemorySystemPtr()->GetMemoryBlockInfoPtrs();
    blocks;

    const auto& adapters = pRenderSystem->GetFactoryPtr()->GetAdapters();
    adapters;

    pRenderSystem->DestroyDevice( &pDevice );
    DestroyBigosFramework( &pFramework );

    return 0;
}
