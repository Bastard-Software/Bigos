#include "BIGOS/Application.h"

#include "BIGOS/BIGOS.h"
#include "BIGOS/BigosEngine.h"
#include "Core/Utils/Timer.h"
#include "Platform/Window/Window.h"

namespace BIGOS
{
    RESULT IApplication::Create()
    {
        g_pEngine = new BigosEngine();

        OnCreate();

        return Results::OK;
    }

    void IApplication::Destroy()
    {
        OnDestroy();

        delete g_pEngine;
    }

    void IApplication::Run()
    {
    }

    WindowedApplication::WindowedApplication( const char* pName, Driver::Backend::API_TYPE apiType )
        : m_pName( pName )
        , m_apiType( apiType )
        , m_running( BGS_TRUE )
        , m_pWindow( nullptr )
        , m_windowCloseHandler( [ this ]( const Platform::Event::WindowCloseEvent& e ) { OnWindowClose( e ); } )
        , m_windowResizeHandler( [ this ]( const Platform::Event::WindowResizeEvent& e ) { OnWindowResize( e ); } )
        , m_lastFrameTime( 0.0f )
        , m_renderer()
    {
    }

    WindowedApplication::~WindowedApplication()
    {
    }

    RESULT WindowedApplication::Create()
    {
        BigosEngineDesc frameworkDesc;
        if( BGS_FAILED( CreateBigosEngine( frameworkDesc, &g_pEngine ) ) )
        {
            return Results::FAIL;
        }

        BIGOS::Driver::Frontend::DriverDesc driverDesc;
        driverDesc.apiType = m_apiType;
        driverDesc.debug   = true;
        if( BGS_FAILED( g_pEngine->GetRenderSystem().InitializeDriver( driverDesc ) ) )
        {
            return Results::FAIL;
        }

        // Create window
        BIGOS::Platform::WindowDesc wndDesc;
        wndDesc.pTitle    = m_pName;
        wndDesc.mode      = BIGOS::Platform::WindowModes::WINDOW;
        wndDesc.xPosition = 100;
        wndDesc.yPosition = 100;
        wndDesc.width     = 1600;
        wndDesc.height    = 900;
        if( BGS_FAILED( g_pEngine->GetWindowSystem().CreateWindow( wndDesc, &m_pWindow ) ) )
        {
            return BIGOS::Results::FAIL;
        }
        m_pWindow->Show();

        g_pEngine->GetEventSystem().Subscribe( &m_windowCloseHandler );
        g_pEngine->GetEventSystem().Subscribe( &m_windowResizeHandler );

        m_renderer.Create( &g_pEngine->GetRenderSystem(), m_pWindow );

        if( BGS_FAILED( OnCreate() ) )
        {
            Destroy();
            return Results::FAIL;
        }

        return Results::OK;
    }

    void WindowedApplication::Destroy()
    {
        OnDestroy();

        m_renderer.Destroy();
        DestroyBigosEngine( &g_pEngine );
    }

    void WindowedApplication::Run()
    {
        BGS_ASSERT( g_pEngine != nullptr );

        Core::Utils::Timer timer;
        uint32_t           fps     = 0;
        float              fpsTime = 0.0f;

        while( m_running )
        {
            float                 time = timer.ElapsedMillis();
            Core::Utils::Timestep ts   = time - m_lastFrameTime;
            m_lastFrameTime            = time;

            // Application updates
            m_pWindow->Update();
            OnUpdate( ts );

            // Application render
            OnRender( ts );
            fps++;

            // Display fps on window title
            if( timer.Elapsed() - fpsTime > 1.0f )
            {
                std::string title = m_pName + std::string( " (FPS: " ) + std::to_string( fps ) + std::string( ")" );
                m_pWindow->SetTitle( title.c_str() );
                fpsTime += 1.0f;
                fps = 0;
            }
        }
    }

    void WindowedApplication::OnWindowClose( const Platform::Event::WindowCloseEvent& e )
    {
        e;
        m_running = BGS_FALSE;
    }

    void WindowedApplication::OnWindowResize( const Platform::Event::WindowResizeEvent& e )
    {
        const uint32_t width  = e.GetWidth();
        const uint32_t height = e.GetHeight();

        BGS_ASSERT( BGS_SUCCESS( m_renderer.Resize( width, height ) ), "Renderer resize failed!" );
    }
} // namespace BIGOS