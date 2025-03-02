#include "BigosFramework/BigosFramework.h"

#include "Driver/Frontend/RenderSystemTypes.h"

#include "Driver/Frontend/RenderSystem.h"
#include "Platform/Event/EventSystem.h"
#include "Platform/Input/InputSystem.h"
#include "Platform/WindowSystem.h"

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
    BigosFramework::BigosFramework()
        : m_memorySystem()
        , m_pRenderSystem( nullptr )
        , m_pWindowSystem( nullptr )
    {
    }

    RESULT BigosFramework::DefaultInit( Driver::Backend::API_TYPE apiType )
    {
        // TODO: For now we expect no fails during init
        Platform::WindowSystemDesc wndDesc;
        CreateWindowSystem( wndDesc, &m_pWindowSystem );

        Driver::Frontend::RenderSystemDesc renderDesc;
        renderDesc.factoryDesc.apiType = apiType;
        renderDesc.factoryDesc.flags   = BGS_FLAG( BIGOS::Driver::Backend::FactoryFlagBits::ENABLE_DEBUG_LAYERS_IF_AVAILABLE );
        CreateRenderSystem( renderDesc, &m_pRenderSystem );

        return Results::OK;
    }

    RESULT BigosFramework::CreateRenderSystem( const Driver::Frontend::RenderSystemDesc& desc, Driver::Frontend::RenderSystem** ppSystem )
    {
        BGS_ASSERT( ppSystem != nullptr, "Render system (ppSystem) must be a valid address." );

        // If render system exist we return it otherwise we need to create.
        if( m_pRenderSystem == nullptr )
        {
            if( BGS_FAILED( Core::Memory::AllocateObject( m_memorySystem.GetSystemHeapAllocator(), &m_pRenderSystem ) ) )
            {
                return Results::NO_MEMORY;
            }

            if( BGS_FAILED( m_pRenderSystem->Create( desc, m_memorySystem.GetSystemHeapAllocator(), this ) ) )
            {
                Core::Memory::FreeObject( m_memorySystem.GetSystemHeapAllocator(), &m_pRenderSystem );
                return Results::FAIL;
            }
        }

        *ppSystem = m_pRenderSystem;

        return Results::OK;
    }

    void BigosFramework::DestroyRenderSystem( Driver::Frontend::RenderSystem** ppSystem )
    {
        BGS_ASSERT( ppSystem != nullptr, "Render system (ppSystem) must be a valid address." );

        if( ( *ppSystem != nullptr ) && ( *ppSystem == m_pRenderSystem ) )
        {
            Driver::Frontend::RenderSystem* pSystem = *ppSystem;
            pSystem->Destroy();
            Core::Memory::FreeObject( m_memorySystem.GetSystemHeapAllocator(), &pSystem );
            m_pRenderSystem = nullptr;
        }
    }

    RESULT BigosFramework::CreateWindowSystem( const Platform::WindowSystemDesc& desc, Platform::WindowSystem** ppSystem )
    {
        BGS_ASSERT( ppSystem != nullptr, "Window system (ppSystem) must be a valid address." );

        // If render system exist we return it otherwise we need to create.
        if( m_pWindowSystem == nullptr )
        {
            if( BGS_FAILED( Core::Memory::AllocateObject( m_memorySystem.GetSystemHeapAllocator(), &m_pWindowSystem ) ) )
            {
                return Results::NO_MEMORY;
            }

            if( BGS_FAILED( m_pWindowSystem->Create( desc, m_memorySystem.GetSystemHeapAllocator(), this ) ) )
            {
                Core::Memory::FreeObject( m_memorySystem.GetSystemHeapAllocator(), &m_pWindowSystem );
                return Results::FAIL;
            }
        }

        *ppSystem = m_pWindowSystem;

        return Results::OK;
    }

    void BigosFramework::DestroyWindowSystem( Platform::WindowSystem** ppSystem )
    {
        BGS_ASSERT( ppSystem != nullptr, "Window system (ppSystem) must be a valid address." );

        if( ( *ppSystem != nullptr ) && ( *ppSystem == m_pWindowSystem ) )
        {
            Platform::WindowSystem* pSystem = *ppSystem;
            pSystem->Destroy();
            Core::Memory::FreeObject( m_memorySystem.GetSystemHeapAllocator(), &pSystem );
            m_pWindowSystem = nullptr;
        }
    }

    RESULT BigosFramework::Create( const BigosFrameworkDesc& desc )
    {
        // Creating core framework systems
        if( BGS_FAILED( m_memorySystem.Create( desc.memorySystemDesc ) ) )
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

        return Results::OK;
    }

    void BigosFramework::Destroy()
    {
        DestroyRenderSystem( &m_pRenderSystem );
        DestroyWindowSystem( &m_pWindowSystem );

        m_inputSystem.Destroy();
        m_eventSystem.Destroy();
        m_memorySystem.Destroy();
    }

} // namespace BIGOS