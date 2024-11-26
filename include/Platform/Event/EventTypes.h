#pragma once

#include "Core/CoreTypes.h"

#if defined MOUSE_MOVED
#    undef MOUSE_MOVED
#endif

namespace BIGOS
{
    namespace Platform
    {
        namespace Event
        {
            class IEvent;
            class IEventHandlerWraper;
            class EventSystem;
            using EventQueue        = HeapArray<UniquePtr<IEvent>>;
            using EventHandlerArray = HeapArray<IEventHandlerWraper*>;

            template<typename EventType>
            using EventHandler = std::function<void( const EventType& e )>;

            enum class EventTypes
            {
                NONE,
                WINDOW_CLOSE,
                WINDOW_RESIZE,
                WINDOW_FOCUS,
                WINDOW_LOST_FOCUS,
                WINDOW_MOVED,
                APP_TICK,
                APP_UPDATE,
                APP_RENDER,
                KEY_PRESSED,
                KEY_RELEASED,
                KEY_TYPED,
                MOUSE_BUTTON_PRESSED,
                MOUSE_BUTTON_RELEASED,
                MOUSE_MOVED,
                MOUSE_SCROLLED,
                _MAX_ENUM,
            };
            using EVENT_TYPE = EventTypes;

            enum class EventCategoryFlagBits : uint32_t
            {
                NONE          = 0x00000000,
                APPLICATION   = 0x00000001,
                INPUT         = 0x00000002,
                KEYBOARD      = 0x00000004,
                MOUSE         = 0x00000008,
                MAOUSE_BUTTON = 0x00000010,
            };
            using EventCategoryFlags = EventCategoryFlagBits;

            struct EventSystemDesc
            {
                // For future use
            };

        } // namespace Event
    }     // namespace Platform
} // namespace BIGOS