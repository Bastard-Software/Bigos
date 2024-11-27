#pragma once

#include "Core/CoreTypes.h"
#include "Platform/Event/EventTypes.h"

#include "Platform/Event/IEvent.h"

namespace BIGOS
{
    namespace Platform
    {
        namespace Event
        {
            class BGS_API BGS_API_INTERFACE IEventHandlerWraper
            {
            public:
                virtual ~IEventHandlerWraper() = default;

                void Exec( const IEvent& e ) { Call( e ); }

            private:
                virtual void Call( const IEvent& e ) = 0;
            };

            template<typename EventType>
            class BGS_API EventHandlerWraper : public IEventHandlerWraper
            {
            public:
                EventHandlerWraper( const EventHandler<EventType>& handler )
                    : m_handler( handler ){};

            private:
                virtual void Call( const IEvent& e ) override
                {
                    if( e.GetEventType() == EventType::GetStaticEventType() )
                    {
                        m_handler( static_cast<const EventType&>( e ) );
                    }
                }

            private:
                EventHandler<EventType> m_handler;
            };

        } // namespace Event
    }     // namespace Platform
} // namespace BIGOS