#pragma once

#include "Core/CoreTypes.h"

#include "Platform/Event/IEvent.h"
#include "Platform/Input/KeyCodes.h"

namespace BIGOS
{
    namespace Platform
    {
        namespace Event
        {
            class BGS_API KeyPressedEvent : public IEvent
            {
            public:
                KeyPressedEvent( const Input::KeyCode keycode, const uint16_t repeatCnt )
                    : m_keycode( keycode )
                    , m_repeatCnt( repeatCnt )
                {
                }

                Input::KeyCode GetKeyCode() const { return m_keycode; }
                uint16_t       GetRepeatCount() const { return m_repeatCnt; }

                virtual EVENT_TYPE  GetEventType() const override { return EventTypes::KEY_PRESSED; };
                virtual const char* GetName() const override { return "KeyPressed"; };

            private:
                Input::KeyCode m_keycode;
                uint16_t       m_repeatCnt;
            };

            class BGS_API KeyReleasedEvent : public IEvent
            {
            public:
                KeyReleasedEvent( const Input::KeyCode keycode )
                    : m_keycode( keycode )
                {
                }

                Input::KeyCode GetKeyCode() const { return m_keycode; }

                virtual EVENT_TYPE  GetEventType() const override { return EventTypes::KEY_RELEASED; };
                virtual const char* GetName() const override { return "KeyReleased"; };

            private:
                Input::KeyCode m_keycode;
            };

            class BGS_API KeyTypedEvent : public IEvent
            {
            public:
                KeyTypedEvent( const Input::KeyCode keycode )
                    : m_keycode( keycode )
                {
                }

                Input::KeyCode GetKeyCode() const { return m_keycode; }

                virtual EVENT_TYPE  GetEventType() const override { return EventTypes::KEY_TYPED; };
                virtual const char* GetName() const override { return "KeyTyped"; };

            private:
                Input::KeyCode m_keycode;
            };

        } // namespace Event
    }     // namespace Platform
} // namespace BIGOS