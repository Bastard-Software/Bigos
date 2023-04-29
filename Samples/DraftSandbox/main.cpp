#include <stdio.h>
#include "BigosFramework/BigosFramework.h"
#include "Core/Memory/SystemHeapAllocator.h"

int main()
{
    printf( "Hello bigos\n" );

    BIGOS::BigosFramework* pFramework = nullptr;
    BIGOS::BigosFrameworkDesc frameworkDesc;

    if( BGS_FAILED( CreateBigosFramework( frameworkDesc, &pFramework ) ) )
    {
        printf( "Framework initialization failed." );
        return -1;
    }

    auto allocator = pFramework->GetMemorySystem()->GetSystemHeapAllocator();

    void* pBlock = nullptr;
    void* pAlignedBlock = nullptr;
    allocator->Allocate( 64, &pBlock );
    allocator->AllocateAligned( 64, 8, &pAlignedBlock );
    const auto& infoPtrs = pFramework->GetMemorySystem()->GetMemoryBlockInfoPtrs();
    infoPtrs;

    allocator->Free( &pBlock );
    allocator->FreeAligned( &pAlignedBlock );

    DestroyBigosFramework( &pFramework );

    return 0;
}
