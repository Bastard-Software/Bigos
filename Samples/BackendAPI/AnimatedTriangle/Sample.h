#pragma once

#include "BigosFramework/BigosFramework.h"
#include "Platform/Event/ApplicationEvent.h"
#include "Platform/Event/EventHandler.h"
#include "SampleHelper.h"

using namespace BIGOS::Driver::Backend;
using namespace BIGOS::Platform::Event;

class Sample
{
public:
    Sample( APITypes APIType, uint32_t width, uint32_t height, const char* pName );
    virtual ~Sample();

    virtual BIGOS::RESULT OnInit()    = 0;
    virtual void          OnUpdate()  = 0;
    virtual void          OnRender()  = 0;
    virtual void          OnDestroy() = 0;

    void OnWindowClose( const WindowCloseEvent& e );

    const API_TYPE GetAPIType() const { return m_APIType; }

    bool IsRunning() { return m_running; }

protected:
    const char*                          m_pName;
    uint32_t                             m_width;
    uint32_t                             m_height;
    API_TYPE                             m_APIType;
    bool                                 m_running;
    EventHandlerWraper<WindowCloseEvent> m_windowCloseHandler;
};