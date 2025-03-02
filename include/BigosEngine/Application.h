#pragma once

#include "BigosEngine/BigosTypes.h"

#include "BigosEngine/LayerStack.h"
#include "BigosEngine/Renderer/Renderer.h"
#include "BigosFramework/BigosFramework.h"
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
        Application( const char* pName, Driver::Backend::API_TYPE apiType );
        virtual ~Application();

        void PushLayer( Layer* pLayer );
        void PushOverlay( Layer* pLayer );

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

    private:
        Platform::Event::EventHandlerWraper<Platform::Event::WindowCloseEvent>  m_windowCloseHandler;
        Platform::Event::EventHandlerWraper<Platform::Event::WindowResizeEvent> m_windowResizeHandler;

    private:
        LayerStack                m_layerStack;
        Platform::Window*         m_pWindow;
        const char*               m_pName;
        Driver::Backend::API_TYPE m_apiType;
        bool_t                    m_running;
        float                     m_lastFrameTime;

        Renderer m_renderer;

    private:
        static Application*    s_pInstance;
        static BigosFramework* s_pFramework;
    };
} // namespace BIGOS