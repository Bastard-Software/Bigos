#include "BigosFramework/BigosFramework.h"

#include "Driver/Frontend/DriverSystemTypes.h"

#include "Driver/Frontend/DriverSystem.h"

BGS_API BIGOS::RESULT CreateBigosFramework( const BIGOS::BigosFrameworkDesc& desc, BIGOS::BigosFramework** ppFramework )
{
    BGS_ASSERT( ( ppFramework != nullptr ) && ( *ppFramework == nullptr ) );

    // TODO: How to handle that? This function for custom creation?
    // Or engine should own memory system and create framework?
    BIGOS::BigosFramework* pFramework = new BIGOS::BigosFramework;
    if( pFramework == nullptr )
    {
        return BIGOS::Core::Results::NO_MEMORY;
    }

    if( BGS_FAILED( pFramework->Create( desc ) ) )
    {
        delete pFramework;
        pFramework = nullptr;
        return BIGOS::Results::FAIL;
    }

    *ppFramework = pFramework;

    return BIGOS::Results::OK;
}

BGS_API void DestroyBigosFramework( BIGOS::BigosFramework** ppFramework )
{
    BGS_ASSERT( ( ppFramework != nullptr ) && ( *ppFramework != nullptr ) );

    BIGOS::BigosFramework* pFramework = *ppFramework;
    pFramework->Destroy();
    delete pFramework;
    pFramework = nullptr;
}

namespace BIGOS
{

    RESULT BigosFramework::CreateDriverSystem( const Driver::Frontend::DriverSystemDesc& desc,
                                               Driver::Frontend::DriverSystem** ppSystem )
    {
        BGS_ASSERT( ( ppSystem != nullptr ) && ( *ppSystem == nullptr ) );
        BGS_ASSERT( m_pDriverSystem == nullptr );

        if( BGS_FAILED( Core::Memory::AllocateObject( m_memorySystem.GetSystemHeapAllocatorPtr(), &m_pDriverSystem ) ) )
        {
            return Results::NO_MEMORY;
        }

        if( BGS_FAILED( m_pDriverSystem->Create( desc, m_memorySystem.GetSystemHeapAllocatorPtr(), this ) ) )
        {
            Core::Memory::FreeObject( m_memorySystem.GetSystemHeapAllocatorPtr(), &m_pDriverSystem );
            return Results::FAIL;
        }

        *ppSystem = m_pDriverSystem;

        return Results::OK;
    }

    RESULT BigosFramework::Create( const BigosFrameworkDesc& desc )
    {
        if( BGS_FAILED( m_memorySystem.Create( desc.memorySystemDesc ) ) )
        {
            return Results::FAIL;
        }

        return Results::OK;
    }

    void BigosFramework::Destroy()
    {
        m_pDriverSystem->Destroy();
        Core::Memory::FreeObject( m_memorySystem.GetSystemHeapAllocatorPtr(), &m_pDriverSystem );
        m_pDriverSystem = nullptr;

        m_memorySystem.Destroy();
    }
} // namespace BIGOS