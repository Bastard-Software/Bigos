#pragma once

#include "Core/CoreTypes.h"

#include "Platform/Event/IEvent.h"

namespace BIGOS
{
    namespace Platform
    {
        namespace Event
        {
            class BGS_API EventManager
            {
            public:
                EventManager()  = default;
                ~EventManager() = default;

                void TriggerEvent( const IEvent& event );

            protected:
                RESULT Create( const EventManagerDesc& desc, BigosFramework* pFramework );
                void   Destroy();

            private:
                EventQueue m_queue;
            };

        } // namespace Event
    }     // namespace Platform
} // namespace BIGOS