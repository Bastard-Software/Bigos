#pragma once

#include "Core/CoreTypes.h"

#include "Platform/Event/IEvent.h"
#include "Platform/Input/MouseCodes.h"

namespace BIGOS
{
    namespace Platform
    {
        namespace Event
        {

            class BGS_API MouseMovedEvent : public IEvent
            {
            public:
                MouseMovedEvent( const float x, const float y )
                    : m_mouseX( x )
                    , m_mouseY( y )
                {
                }

                float GetX() const { return m_mouseX; }
                float GetY() const { return m_mouseY; }

                virtual EVENT_TYPE  GetEventType() const override { return EventTypes::MOUSE_MOVED; };
                virtual const char* GetName() const override { return "MouseMoved"; };

            private:
                float m_mouseX;
                float m_mouseY;
            };

            class BGS_API MouseScrolledEvent : public IEvent
            {
            public:
                MouseScrolledEvent( const float x, const float y )
                    : m_offsetX( x )
                    , m_offsetY( y )
                {
                }

                float GetX() const { return m_offsetX; }
                float GetY() const { return m_offsetY; }

                virtual EVENT_TYPE  GetEventType() const override { return EventTypes::MOUSE_SCROLLED; };
                virtual const char* GetName() const override { return "MouseScrolled"; };

            private:
                float m_offsetX;
                float m_offsetY;
            };

            class BGS_API MouseButtonPressedEvent : public IEvent
            {
            public:
                MouseButtonPressedEvent( const Input::MouseCode button )
                    : m_button( button )
                {
                }

                Input::MouseCode GetMouseButton() const { return m_button; }

                virtual EVENT_TYPE  GetEventType() const override { return EventTypes::MOUSE_BUTTON_PRESSED; };
                virtual const char* GetName() const override { return "MouseButtonPressed"; };

            private:
                Input::MouseCode m_button;
            };

            class BGS_API MouseButtonReleasedEvent : public IEvent
            {
            public:
                MouseButtonReleasedEvent( const Input::MouseCode button )
                    : m_button( button )
                {
                }

                Input::MouseCode GetMouseButton() const { return m_button; }

                virtual EVENT_TYPE  GetEventType() const override { return EventTypes::MOUSE_BUTTON_RELEASED; };
                virtual const char* GetName() const override { return "MouseButtonReleased"; };

            private:
                Input::MouseCode m_button;
            };

        } // namespace Event
    }     // namespace Platform
} // namespace BIGOS