#pragma once

#include "Core/CoreTypes.h"

#include "Platform/Event/EventHandler.h"
#include "Platform/Event/IEvent.h"

namespace BIGOS
{
    namespace Platform
    {
        namespace Event
        {
            class BGS_API EventSystem final
            {
                friend class BigosFramework;

            public:
                EventSystem()  = default;
                ~EventSystem() = default;

                void Subscribe( IEventHandlerWraper* pHandler );
                void Unsubscribe( IEventHandlerWraper* pHandler );

                void TriggerEvent( const IEvent& event );

            protected:
                RESULT Create( const EventSystemDesc& desc, BigosFramework* pFramework );
                void   Destroy();

            private:
                EventSystemDesc   m_desc;
                EventHandlerArray m_handlers;
                BigosFramework*   m_pParent;
            };

        } // namespace Event
    }     // namespace Platform
} // namespace BIGOS