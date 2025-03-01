#include "BigosEngine/Application.h"

#include "BigosFramework/BigosFramework.h"
#include "Core/Utils/Timer.h"
#include "Driver/Frontend/RenderDevice.h"
#include "Platform/Event/EventHandler.h"
#include "Platform/Event/EventSystem.h"
#include "Platform/Window.h"
#include "Platform/WindowSystem.h"

namespace BIGOS
{
    Application*    Application::s_pInstance  = nullptr;
    BigosFramework* Application::s_pFramework = nullptr;

    Application::Application( const char* pName, Driver::Backend::API_TYPE apiType )
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

    Application::~Application()
    {
    }

    void Application::PushLayer( Layer* pLayer )
    {
        m_layerStack.PushLayer( pLayer );
        pLayer->OnAttach();
    }

    void Application::PushOverlay( Layer* pLayer )
    {
        pLayer;
    }

    RESULT Application::Create()
    {
        BGS_ASSERT( s_pInstance == nullptr, "Bigos application already exists." );
        BGS_ASSERT( s_pFramework == nullptr, "Bigos framework already exists." );

        s_pInstance = this;

        BIGOS::BigosFrameworkDesc frameworkDesc;
        if( BGS_FAILED( CreateBigosFramework( frameworkDesc, &s_pFramework ) ) )
        {
            return Results::FAIL;
        }

        // Init bigos systems
        if( BGS_FAILED( s_pFramework->DefaultInit( m_apiType ) ) )
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
        if( BGS_FAILED( s_pFramework->GetWindowSystem()->CreateWindow( wndDesc, &m_pWindow ) ) )
        {
            return BIGOS::Results::FAIL;
        }
        m_pWindow->Show();

        if( BGS_FAILED( s_pFramework->DefaultInit( Driver::Backend::APITypes::VULKAN ) ) )
        {
            return Results::FAIL;
        }

        s_pFramework->GetEventSystem()->Subscribe( &m_windowCloseHandler );
        s_pFramework->GetEventSystem()->Subscribe( &m_windowResizeHandler );

        m_renderer.Create( s_pFramework->GetRenderSystem(), m_pWindow );

        return Results::OK;
    }

    void Application::Destroy()
    {
        m_renderer.Destroy();
        DestroyBigosFramework( &s_pFramework );
    }

    void Application::Run()
    {
        BGS_ASSERT( s_pInstance != nullptr );
        BGS_ASSERT( s_pFramework != nullptr );

        Core::Utils::Timer timer;
        uint32_t           fps     = 0;
        float              fpsTime = 0.0f;

        while( m_running )
        {
            float                 time = timer.ElapsedMillis();
            Core::Utils::Timestep ts   = time - m_lastFrameTime;
            m_lastFrameTime            = time;

            m_pWindow->Update();

            for( Layer* pLayer: m_layerStack )
            {
                pLayer->OnUpdate( ts );
            }
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

    void Application::OnWindowClose( const Platform::Event::WindowCloseEvent& e )
    {
        e;
        m_running = BGS_FALSE;
    }

    void Application::OnWindowResize( const Platform::Event::WindowResizeEvent& e )
    {
        const uint32_t width  = e.GetWidth();
        const uint32_t height = e.GetHeight();

        BGS_ASSERT( BGS_SUCCESS( m_renderer.Resize( width, height ) ), "Renderer resize failed!" );
    }

} // namespace BIGOS