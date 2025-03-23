#pragma once

#include "Core/CoreTypes.h"
#include "Driver/Backend/APITypes.h"

#include "BIGOS/Renderer/Renderer.h"
#include "Core/Utils/Timestep.h"
#include "Platform/Event/ApplicationEvent.h"
#include "Platform/Event/EventHandler.h"

int main( int argc, char** argv );

namespace BIGOS
{
    class BGS_API BGS_API_INTERFACE IApplication
    {
        friend int ::main( int argc, char** argv );

    public:
        virtual ~IApplication() = default;

        virtual RESULT OnCreate()                     = 0;
        virtual void   OnUpdate( Utils::Timestep ts ) = 0;
        virtual void   OnRender( Utils::Timestep ts ) = 0;
        virtual void   OnDestroy()                    = 0;

    protected:
        virtual RESULT Create()  = 0;
        virtual void   Destroy() = 0;
        virtual void   Run()     = 0;
    };

    class BGS_API WindowedApplication : public IApplication
    {
    public:
        WindowedApplication( const char* pName, Driver::Backend::API_TYPE apiType );
        virtual ~WindowedApplication();

        virtual RESULT OnCreate()                     = 0;
        virtual void   OnUpdate( Utils::Timestep ts ) = 0;
        virtual void   OnRender( Utils::Timestep ts ) = 0;
        virtual void   OnDestroy()                    = 0;

        virtual RESULT Create();
        virtual void   Destroy();
        virtual void   Run();

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
    };

    BGS_API IApplication* CreateApplication();

} // namespace BIGOS