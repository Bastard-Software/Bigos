#pragma once

#include "Core/CoreTypes.h"
#include "Platform/PlatformTypes.h"

#include "Platform/Event/EventHandler.h"
#include "Platform/Event/KeyEvent.h"
#include "Platform/Event/MouseEvent.h"
#include "Platform/Input/KeyCodes.h"
#include "Platform/Input/MouseCodes.h"

namespace BIGOS
{
    namespace Platform
    {
        namespace Input
        {
            constexpr uint32_t MAX_KEYS    = 512;
            constexpr uint32_t MAX_BUTTONS = 16;

            class BGS_API InputSystem final
            {
                friend class BigosFramework;

            public:
                InputSystem();
                ~InputSystem() = default;

                bool_t IsKeyPressed( KeyCode key ) const;
                bool_t IsMouseButtonPressed( MouseCode button ) const;

                float GetMousePositionX() const;
                float GetMousePositionY() const;

                float GetMouseScrollOffsetX() const;
                float GetMouseScrollOffsetY() const;

            protected:
                RESULT Create( const InputSystemDesc& desc, BigosFramework* pFramework );
                void   Destroy();

            private:
                void OnKeyPressed( const Event::KeyPressedEvent& e );
                void OnKeyReleased( const Event::KeyReleasedEvent& e );
                void OnMouseButtonPressed( const Event::MouseButtonPressedEvent& e );
                void OnMouseButtonReleased( const Event::MouseButtonReleasedEvent& e );
                void OnMouseMoved( const Event::MouseMovedEvent& e );
                void OnMouseScrolled( const Event::MouseScrolledEvent& e );

                void ClearKeys();
                void ClearButtons();

            private:
                InputSystemDesc m_desc;
                BigosFramework* m_pParent;
                // Input states
                bool_t m_keyState[ MAX_KEYS ];
                bool_t m_mouseButtonState[ MAX_BUTTONS ];
                float  m_mouseX;
                float  m_mouseY;
                float  m_mouseScrolledX;
                float  m_mouseScrolledY;
                // Event handlers
                Event::EventHandlerWraper<Event::KeyPressedEvent>          m_keyPressedHandler;
                Event::EventHandlerWraper<Event::KeyReleasedEvent>         m_keyReleasedHandler;
                Event::EventHandlerWraper<Event::MouseButtonPressedEvent>  m_mouseButtonPressedHandler;
                Event::EventHandlerWraper<Event::MouseButtonReleasedEvent> m_mouseButtonReleasedHandler;
                Event::EventHandlerWraper<Event::MouseMovedEvent>          m_mouseMovedHandler;
                Event::EventHandlerWraper<Event::MouseScrolledEvent>       m_mouseScrolledHandler;
            };
        } // namespace Input
    }     // namespace Platform
} // namespace BIGOS