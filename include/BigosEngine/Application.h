#pragma once

#include "BigosEngine/BigosTypes.h"

#include "BigosEngine/Renderer/Renderer.h"
#include "BigosFramework/BigosFramework.h"
#include "Core/Utils/Timestep.h"
#include "Platform/Event/ApplicationEvent.h"
#include "Platform/Event/EventHandler.h"

int main( int argc, char** argv );

namespace BIGOS
{
    BGS_API Application* CreateApplication();

    class BGS_API Application
    {
        friend int ::main( int argc, char** argv );

    public:
        virtual RESULT OnCreate()                     = 0;
        virtual void   OnUpdate( Utils::Timestep ts ) = 0;
        virtual void   OnRender( Utils::Timestep ts ) = 0;
        virtual void   OnDestroy()                    = 0;

    public:
        Application( const char* pName, Driver::Backend::API_TYPE apiType );
        virtual ~Application();

        const Renderer& GetRenderer() { return m_renderer; }

        static Application&    Get() { return *s_pInstance; }
        static BigosFramework& GetFramework() { return *s_pFramework; }

    protected:
        RESULT Create();
        void   Destroy();
        void   Run();

    private:
        void OnWindowClose( const Platform::Event::WindowCloseEvent& e );
        void OnWindowResize( const Platform::Event::WindowResizeEvent& e );

    protected:
        Renderer m_renderer;

    private:
        float                     m_lastFrameTime;
        Platform::Window*         m_pWindow;
        const char*               m_pName;
        Driver::Backend::API_TYPE m_apiType;
        bool_t                    m_running;

    private:
        Platform::Event::EventHandlerWraper<Platform::Event::WindowCloseEvent>  m_windowCloseHandler;
        Platform::Event::EventHandlerWraper<Platform::Event::WindowResizeEvent> m_windowResizeHandler;

    private:
        static Application*    s_pInstance;
        static BigosFramework* s_pFramework;
    };
} // namespace BIGOS