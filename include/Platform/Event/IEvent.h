#pragma once

#include "Core/CoreTypes.h"
#include "Platform/Event/EventTypes.h"

namespace BIGOS
{
    namespace Platform
    {
        namespace Event
        {
            class BGS_API BGS_API_INTERFACE IEvent
            {
            public:
                ~IEvent() = default;

                virtual EVENT_TYPE  GetEventType() const = 0;
                virtual const char* GetName() const      = 0;

            public:
                bool handled = false;
            };
        } // namespace Event
    }     // namespace Platform
} // namespace BIGOS