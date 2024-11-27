#include "Platform/Input/InputSystem.h"

#include "Core/Memory/Memory.h"
#include "Platform/Event/EventSystem.h"

namespace BIGOS
{
    namespace Platform
    {
        namespace Input
        {
            InputSystem::InputSystem()
                : m_desc()
                , m_pParent( nullptr )
                , m_keyState()
                , m_mouseButtonState()
                , m_mouseX( 0.0f )
                , m_mouseY( 0.0f )
                , m_mouseScrolledX( 0.0f )
                , m_mouseScrolledY( 0.0f )
                , m_keyPressedHandler( [ this ]( const Event::KeyPressedEvent& e ) { OnKeyPressed( e ); } )
                , m_keyReleasedHandler( [ this ]( const Event::KeyReleasedEvent& e ) { OnKeyReleased( e ); } )
                , m_mouseButtonPressedHandler( [ this ]( const Event::MouseButtonPressedEvent& e ) { OnMouseButtonPressed( e ); } )
                , m_mouseButtonReleasedHandler( [ this ]( const Event::MouseButtonReleasedEvent& e ) { OnMouseButtonReleased( e ); } )
                , m_mouseMovedHandler( [ this ]( const Event::MouseMovedEvent& e ) { OnMouseMoved( e ); } )
                , m_mouseScrolledHandler( [ this ]( const Event::MouseScrolledEvent& e ) { OnMouseScrolled( e ); } )
            {
            }

            bool_t InputSystem::IsKeyPressed( KeyCode key ) const
            {
                BGS_ASSERT( key < MAX_KEYS );

                return m_keyState[ key ];
            }

            bool_t InputSystem::IsMouseButtonPressed( MouseCode button ) const
            {
                BGS_ASSERT( button < MAX_BUTTONS );

                return m_mouseButtonState[ button ];
            }

            float InputSystem::GetMousePositionX() const
            {
                return m_mouseX;
            }

            float InputSystem::GetMousePositionY() const
            {
                return m_mouseY;
            }

            float InputSystem::GetMouseScrollOffsetX() const
            {
                return m_mouseScrolledX;
            }

            float InputSystem::GetMouseScrollOffsetY() const
            {
                return m_mouseScrolledY;
            }

            RESULT InputSystem::Create( const InputSystemDesc& desc, BigosFramework* pFramework )
            {
                BGS_ASSERT( pFramework != nullptr, "Bigos framework (pFramework) must be a valid pointer." );
                BGS_ASSERT( desc.pEventSystem != nullptr, "Event system (desc.pEventSystem) must be a valid pointer." );

                m_desc    = desc;
                m_pParent = pFramework;

                // Subscribing handlers to event system
                // TODO: Do we want to allow creating input system without event system?
                m_desc.pEventSystem->Subscribe( &m_keyPressedHandler );
                m_desc.pEventSystem->Subscribe( &m_keyReleasedHandler );
                m_desc.pEventSystem->Subscribe( &m_mouseButtonPressedHandler );
                m_desc.pEventSystem->Subscribe( &m_mouseButtonReleasedHandler );
                m_desc.pEventSystem->Subscribe( &m_mouseMovedHandler );
                m_desc.pEventSystem->Subscribe( &m_mouseScrolledHandler );

                // Clearing input
                ClearKeys();
                ClearButtons();

                return Results::OK;
            }

            void InputSystem::Destroy()
            {
                ClearKeys();
                ClearButtons();
                m_pParent = nullptr;
            }

            void InputSystem::OnKeyPressed( const Event::KeyPressedEvent& e )
            {
                const KeyCode key = e.GetKeyCode();
                m_keyState[ key ] = BGS_TRUE;
            }

            void InputSystem::OnKeyReleased( const Event::KeyReleasedEvent& e )
            {
                const KeyCode key = e.GetKeyCode();
                m_keyState[ key ] = BGS_FALSE;
            }

            void InputSystem::OnMouseButtonPressed( const Event::MouseButtonPressedEvent& e )
            {
                const MouseCode button       = e.GetMouseButton();
                m_mouseButtonState[ button ] = BGS_TRUE;
            }

            void InputSystem::OnMouseButtonReleased( const Event::MouseButtonReleasedEvent& e )
            {
                const MouseCode button       = e.GetMouseButton();
                m_mouseButtonState[ button ] = BGS_FALSE;
            }

            void InputSystem::OnMouseMoved( const Event::MouseMovedEvent& e )
            {
                m_mouseX = e.GetX();
                m_mouseY = e.GetY();
            }

            void InputSystem::OnMouseScrolled( const Event::MouseScrolledEvent& e )
            {
                m_mouseScrolledX = e.GetX();
                m_mouseScrolledY = e.GetY();
            }

            void InputSystem::ClearKeys()
            {
                Core::Memory::Set( &m_keyState, 0, sizeof( m_keyState ) );
            }

            void InputSystem::ClearButtons()
            {
                Core::Memory::Set( &m_mouseButtonState, 0, sizeof( m_mouseButtonState ) );
            }

        } // namespace Input
    }     // namespace Platform
} // namespace BIGOS