#include "Platform/Event/EventSystem.h"

#include "Platform/Event/EventHandler.h"

namespace BIGOS
{
    namespace Platform
    {
        namespace Event
        {
            void EventSystem::Subscribe( IEventHandlerWraper* pHandler )
            {
                BGS_ASSERT( pHandler != nullptr, "IEventHandlerWraper (pHandler) must be a valis pointer." );

                for( index_t ndx = 0; ndx < m_handlers.size(); ++ndx )
                {
                    if( m_handlers[ ndx ] == pHandler )
                    {
                        BGS_ASSERT( false, "Handler is already in subscriber list." );
                        return;
                    }
                }

                m_handlers.push_back( pHandler );
            }

            void EventSystem::Unsubscribe( IEventHandlerWraper* pHandler )
            {
                BGS_ASSERT( pHandler != nullptr, "IEventHandlerWraper (pHandler) must be a valis pointer." );

                for( index_t ndx = 0; ndx < m_handlers.size(); ++ndx )
                {
                    if( m_handlers[ ndx ] == pHandler )
                    {
                        m_handlers[ ndx ] = m_handlers.back();
                        m_handlers.pop_back();
                    }
                }

                BGS_ASSERT( false, "Handler not fopund in subscriber list." );
            }

            void EventSystem::TriggerEvent( const IEvent& event )
            {
                for( index_t ndx = 0; ndx < m_handlers.size(); ++ndx )
                {
                    m_handlers[ ndx ]->Exec( event );
                }
            }

            RESULT EventSystem::Create( const EventSystemDesc& desc, BigosFramework* pFramework )
            {
                BGS_ASSERT( pFramework != nullptr, "Bigos framework (pFramework) must be a valid pointer." );

                m_desc    = desc;
                m_pParent = pFramework;
                m_handlers.resize( 0 );

                return Results::OK;
            }

            void EventSystem::Destroy()
            {
                m_handlers.clear();
                m_pParent = nullptr;
            }

        } // namespace Event
    }     // namespace Platform
} // namespace BIGOS