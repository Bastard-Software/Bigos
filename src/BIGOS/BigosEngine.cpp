#include "BIGOS/BigosEngine.h"

namespace BIGOS
{

    BigosEngine::BigosEngine()
        : m_memorySystem()
        , m_renderSystem()
        , m_windowSystem()
        , m_inputSystem()
        , m_eventSystem()
    {
    }

    RESULT BigosEngine::Create( const BigosEngineDesc& desc )
    {
        // Creating core framework systems
        if( BGS_FAILED( m_memorySystem.Create( desc.memorySystemDesc, this ) ) )
        {
            Destroy();
            return Results::FAIL;
        }
        if( BGS_FAILED( m_eventSystem.Create( desc.eventSystemDesc, this ) ) )
        {
            Destroy();
            return Results::FAIL;
        }
        if( BGS_FAILED( m_inputSystem.Create( desc.inputSystemDesc, this ) ) )
        {
            Destroy();
            return Results::FAIL;
        }
        if( BGS_FAILED( m_renderSystem.Create( desc.renderSystemDesc, m_memorySystem.GetSystemHeapAllocator(), this ) ) )
        {
            Destroy();
            return Results::FAIL;
        }
        if( BGS_FAILED( m_windowSystem.Create( desc.windowSystemDesc, m_memorySystem.GetSystemHeapAllocator(), this ) ) )
        {
            Destroy();
            return Results::FAIL;
        }

        return Results::OK;
    }

    void BigosEngine::Destroy()
    {
        m_renderSystem.Destroy();
        m_windowSystem.Destroy();
        m_inputSystem.Destroy();
        m_eventSystem.Destroy();
        m_memorySystem.Destroy();
    }

} // namespace BIGOS

BGS_API BIGOS::RESULT CreateBigosEngine( const BIGOS::BigosEngineDesc& desc, BIGOS::BigosEngine** ppFramework )
{
    BGS_ASSERT( ( ppFramework != nullptr ) && ( *ppFramework == nullptr ) );

    if( *ppFramework != nullptr )
    {
        return BIGOS::Results::OK;
    }
    // TODO: How to handle that? This function for custom creation?
    // Or engine should own memory system and create framework?
    BIGOS::BigosEngine* pFramework = new BIGOS::BigosEngine;
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

BGS_API void DestroyBigosEngine( BIGOS::BigosEngine** ppFramework )
{
    BGS_ASSERT( ( ppFramework != nullptr ) && ( *ppFramework != nullptr ) );

    if( ( ppFramework != nullptr ) && ( *ppFramework != nullptr ) )
    {
        BIGOS::BigosEngine* pFramework = *ppFramework;
        pFramework->Destroy();
        delete pFramework;
        pFramework = nullptr;
    }
}
