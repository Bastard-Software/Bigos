#pragma once

#include "BigosEngine/BigosTypes.h"

#include "BigosEngine/LayerStack.h"
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
        Application( const char* pName );
        virtual ~Application();

        void PushLayer( Layer* pLayer );
        void PushOverlay( Layer* pLayer );

        static Application&    Get() { return *s_pInstance; }
        static BigosFramework& GetFramework() { return *s_pFramework; }

    protected:
        RESULT Create();
        void   Destroy();
        void   Run();

    private:
        void OnWindowClose( const Platform::Event::WindowCloseEvent& e );

    private:
        Platform::Event::EventHandlerWraper<Platform::Event::WindowCloseEvent> m_windowCloseHandler;

    private:
        LayerStack        m_layerStack;
        Platform::Window* m_pWindow;
        const char*       m_pName;
        bool_t            m_running;
        float             m_lastFrameTime;

        Driver::Frontend::RenderDevice*    m_pDefaultDevice;
        Driver::Frontend::GraphicsContext* m_pGraphicsCtx;

    private:
        static Application*    s_pInstance;
        static BigosFramework* s_pFramework;
    };
} // namespace BIGOS