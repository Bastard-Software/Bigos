#include "Platform/Window/WindowSystem.h"

#include "Platform/PlatformTypes.h"

#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Memory.h"
#include "Platform/Window/Window.h"


namespace BIGOS
{
    namespace Platform
    {
        WindowSystem::WindowSystem()
            : m_windows()
            , m_desc()
            , m_pParent( nullptr )
            , m_pDefaultAllocator( nullptr )    
        {
        }

        RESULT WindowSystem::CreateWindow( const WindowDesc& desc, Window** ppWindow )
        {
            BGS_ASSERT( ppWindow != nullptr, "Window (ppWindow) must be a valid address." );
            BGS_ASSERT( *ppWindow == nullptr, "There is a pointer at the given address. Window (*ppWindow) must be nullptr." );
            if( ( ppWindow == nullptr ) && ( *ppWindow != nullptr ) )
            {
                return Results::FAIL;
            }

            Window* pWindow = nullptr;
            if( BGS_FAILED( Memory::AllocateObject( m_pDefaultAllocator, &pWindow ) ) )
            {
                return Results::NO_MEMORY;
            }
            BGS_ASSERT( pWindow != nullptr );

            if( BGS_FAILED( pWindow->Create( desc, this ) ) )
            {
                Memory::FreeObject( m_pDefaultAllocator, &pWindow );
                return Results::FAIL;
            }

            m_windows.push_back( pWindow );
            *ppWindow = pWindow;

            return Results::OK;
        }

        void WindowSystem::DestroyWindow( Window** ppWindow )
        {
            BGS_ASSERT( ppWindow != nullptr, "Window (ppWindow) must be a valid address." );
            BGS_ASSERT( *ppWindow != nullptr, "Window (*ppWindow) must be a valid pointer." );
            if( ( ppWindow != nullptr ) && ( *ppWindow != nullptr ) )
            {
                Window* pWnd = *ppWindow;
                for( index_t ndx = 0; ndx < m_windows.size(); ++ndx )
                {
                    // TODO: Find window in array using hash
                    if( m_windows[ ndx ] == pWnd )
                    {
                        m_windows[ ndx ] = m_windows.back();
                        m_windows.pop_back();
                        pWnd->Destroy();
                        Memory::FreeObject( m_pDefaultAllocator, &pWnd );
                        ppWindow = nullptr;
                        break;
                    }
                }
            }
        }

        RESULT WindowSystem::Create( const WindowSystemDesc& desc, Core::Memory::IAllocator* pAllocator, BigosEngine* pEngine )
        {
            BGS_ASSERT( pEngine != nullptr, "Bigos engine (pEngine) must be a valid pointer." );
            BGS_ASSERT( pAllocator != nullptr, "Allocator (pAllocator) must be a valid pointer." );
            if( ( pEngine == nullptr ) || ( pAllocator == nullptr ) )
            {
                return Results::FAIL;
            }

            m_pDefaultAllocator = pAllocator;
            m_pParent           = pEngine;
            m_desc              = desc;

            return Results::OK;
        }

        void WindowSystem::Destroy()
        {
            for( index_t ndx = 0; ndx < m_windows.size(); ++ndx )
            {
                Window* pWnd = m_windows[ ndx ];
                pWnd->Destroy();
                Memory::FreeObject( m_pDefaultAllocator, &pWnd );
            }
            m_windows.clear();
            m_pParent           = nullptr;
            m_pDefaultAllocator = nullptr;
        }
    } // namespace Platform
} // namespace BIGOS