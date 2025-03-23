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
                friend class BigosEngine;

            public:
                EventSystem();
                ~EventSystem() = default;

                void Subscribe( IEventHandlerWraper* pHandler );
                void Unsubscribe( IEventHandlerWraper* pHandler );

                void TriggerEvent( const IEvent& event );

            protected:
                RESULT Create( const EventSystemDesc& desc, BigosEngine* pEngine );
                void   Destroy();

            private:
                EventSystemDesc   m_desc;
                EventHandlerArray m_handlers;
                BigosEngine*      m_pParent;
            };

        } // namespace Event
    } // namespace Platform
} // namespace BIGOS