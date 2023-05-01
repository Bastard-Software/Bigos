#include "BigosFramework/BigosFramework.h"
#include "Core/Memory/SystemHeapAllocator.h"
#include <stdio.h>

int main()
{
    printf( "Hello bigos\n" );

    BIGOS::BigosFramework*                 pFramework    = nullptr;
    BIGOS::BigosFrameworkDesc              frameworkDesc;
    frameworkDesc.driverSystemDesc.factoryDesc.apiType = BIGOS::Driver::Backend::APITypes::D3D12;

    if( BGS_FAILED( CreateBigosFramework( frameworkDesc, &pFramework ) ) )
    {
        printf( "Framework initialization failed." );
        return -1;
    }

    BIGOS::Driver::Frontend::DriverSystem* pDriverSystem = nullptr;
    pFramework->CreateDriverSystem( frameworkDesc.driverSystemDesc, &pDriverSystem );
   
    auto allocator = pFramework->GetMemorySystemPtr()->GetSystemHeapAllocatorPtr();

    void* pBlock        = nullptr;
    void* pAlignedBlock = nullptr;
    allocator->Allocate( 64, &pBlock );
    allocator->AllocateAligned( 64, 8, &pAlignedBlock );
    const auto& infoPtrs = pFramework->GetMemorySystemPtr()->GetMemoryBlockInfoPtrs();
    infoPtrs;

    allocator->Free( &pBlock );
    allocator->FreeAligned( &pAlignedBlock );

    DestroyBigosFramework( &pFramework );

    return 0;
}
