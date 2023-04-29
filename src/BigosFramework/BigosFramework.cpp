#include "BigosFramework/BigosFramework.h"

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
        m_memorySystem.Destroy();
    }
} // namespace BIGOS