#include "BigosFramework/BigosFramework.h"

#include "Driver/Frontend/RenderSystemTypes.h"

#include "Driver/Frontend/RenderSystem.h"

BGS_API BIGOS::RESULT CreateBigosFramework( const BIGOS::BigosFrameworkDesc& desc, BIGOS::BigosFramework** ppFramework )
{
    BGS_ASSERT( ( ppFramework != nullptr ) && ( *ppFramework == nullptr ) );

    if( *ppFramework != nullptr )
    {
        return BIGOS::Results::OK;
    }
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

    if( ( ppFramework != nullptr ) && ( *ppFramework != nullptr ) )
    {
        BIGOS::BigosFramework* pFramework = *ppFramework;
        pFramework->Destroy();
        delete pFramework;
        pFramework = nullptr;
    }
}

namespace BIGOS
{

    RESULT BigosFramework::CreateRenderSystem( const Driver::Frontend::RenderSystemDesc& desc, Driver::Frontend::RenderSystem** ppSystem )
    {
        BGS_ASSERT( ( ppSystem != nullptr ) && ( *ppSystem == nullptr ) );

        // If render system exist we return it otherwise we need to create.
        if( m_pRenderSystem == nullptr )
        {
            if( BGS_FAILED( Core::Memory::AllocateObject( m_memorySystem.GetSystemHeapAllocatorPtr(), &m_pRenderSystem ) ) )
            {
                return Results::NO_MEMORY;
            }

            if( BGS_FAILED( m_pRenderSystem->Create( desc, m_memorySystem.GetSystemHeapAllocatorPtr(), this ) ) )
            {
                Core::Memory::FreeObject( m_memorySystem.GetSystemHeapAllocatorPtr(), &m_pRenderSystem );
                return Results::FAIL;
            }
        }

        *ppSystem = m_pRenderSystem;

        return Results::OK;
    }

    void BigosFramework::DestroyRenderSystem( Driver::Frontend::RenderSystem** ppSystem )
    {
        BGS_ASSERT( ( ppSystem != nullptr ) && ( *ppSystem != nullptr ) );

        if( *ppSystem != nullptr )
        {
            Driver::Frontend::RenderSystem* pSystem = *ppSystem;
            pSystem->Destroy();
            Core::Memory::FreeObject( m_memorySystem.GetSystemHeapAllocatorPtr(), &pSystem );
            pSystem = nullptr;
        }
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
        DestroyRenderSystem( &m_pRenderSystem );

        m_memorySystem.Destroy();
    }

} // namespace BIGOS