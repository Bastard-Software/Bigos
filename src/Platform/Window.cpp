#include "Platform/Window.h"

#include "BigosFramework/BigosFramework.h"
#include "Platform/Event/ApplicationEvent.h"
#include "Platform/Event/EventSystem.h"
#include "Platform/Event/KeyEvent.h"
#include "Platform/Event/MouseEvent.h"
#include "Platform/WindowSystem.h"

namespace BIGOS
{
    namespace Platform
    {

#if( BGS_WINDOWS )

        LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

        struct Win32Desc
        {
            RECT  wndRect{};
            DWORD style{};
            DWORD exStyle{};
        };

        static Win32Desc MapBigosWindowDescToWin32Desc( WindowDesc& desc )
        {
            Win32Desc nativeDesc{};
            nativeDesc.exStyle = WS_EX_APPWINDOW;
            nativeDesc.style   = WS_POPUP;

            const HWND  hDesktop = ::GetDesktopWindow();
            HMONITOR    hMon     = ::MonitorFromWindow( hDesktop, MONITOR_DEFAULTTONEAREST );
            MONITORINFO monInf   = { sizeof( monInf ) };
            GetMonitorInfo( hMon, &monInf );

            switch( desc.mode )
            {
                case WindowModes::WINDOW:
                {
                    nativeDesc.exStyle |= WS_EX_WINDOWEDGE;
                    nativeDesc.style |= WS_OVERLAPPEDWINDOW;

                    RECT rect = { static_cast<LONG>( desc.xPosition ), static_cast<LONG>( desc.yPosition ),
                                  static_cast<LONG>( desc.xPosition ) + static_cast<LONG>( desc.width ),
                                  static_cast<LONG>( desc.yPosition ) + static_cast<LONG>( desc.height ) };
                    ::AdjustWindowRectEx( &rect, nativeDesc.style, false, nativeDesc.exStyle );
                    nativeDesc.wndRect = rect;

                    break;
                }
                case WindowModes::FULLSCREEN_WINDOW:
                {
                    nativeDesc.exStyle |= WS_EX_WINDOWEDGE;
                    nativeDesc.style |= WS_OVERLAPPEDWINDOW;

                    desc.width  = static_cast<uint32_t>( monInf.rcWork.right - monInf.rcWork.left );
                    desc.height = static_cast<uint32_t>( monInf.rcWork.bottom - monInf.rcWork.top );

                    RECT rect = monInf.rcWork;
                    ::AdjustWindowRectEx( &rect, nativeDesc.style, false, nativeDesc.exStyle );
                    nativeDesc.wndRect = rect;

                    break;
                }
                case WindowModes::FULLSCREEN:
                {
                    nativeDesc.wndRect = monInf.rcMonitor;

                    desc.width  = static_cast<uint32_t>( monInf.rcMonitor.right );
                    desc.height = static_cast<uint32_t>( monInf.rcMonitor.bottom );

                    break;
                }
            }

            return nativeDesc;
        }

        Window::Window()
            : m_desc()
            , m_internals()
            , m_pParent( nullptr )
        {
        }

        void Window::Show()
        {
            BGS_ASSERT( m_internals.hWnd != nullptr, "Invalid internal window handle." );
            if( m_internals.hWnd != nullptr )
            {
                ::ShowWindow( m_internals.hWnd, SW_SHOW );
            }
        }

        void Window::Update()
        {
            MSG msg;
            while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) > 0 )
            {
                ::TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
        }

        RESULT Window::Create( const WindowDesc& desc, WindowSystem* pWndSystem )
        {
            BGS_ASSERT( pWndSystem != nullptr, "Window system (pWndSystem) must be a valid pointer." );
            if( pWndSystem == nullptr )
            {
                return Results::FAIL;
            }
            m_desc    = desc;
            m_pParent = pWndSystem;

            // Native window class registration
            WNDCLASSEX winClass{};
            winClass.cbSize        = sizeof( WNDCLASSEX );
            winClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
            winClass.lpfnWndProc   = WndProc;
            winClass.cbClsExtra    = NULL;
            winClass.cbWndExtra    = NULL;
            winClass.hInstance     = m_internals.hModule;
            winClass.hIcon         = LoadIcon( NULL, IDI_APPLICATION );
            winClass.hCursor       = LoadCursor( NULL, IDC_ARROW );
            winClass.hbrBackground = reinterpret_cast<HBRUSH>( COLOR_WINDOW + 1 );
            winClass.lpszMenuName  = "";
            winClass.lpszClassName = m_desc.pTitle;

            if( !RegisterClassEx( &winClass ) )
            {
                return Results::FAIL;
            }

            Win32Desc nativeDesc = MapBigosWindowDescToWin32Desc( m_desc );

            // Native window handle creation
            m_internals.hWnd = CreateWindowEx( nativeDesc.exStyle,                                 // window style (optional)
                                               m_desc.pTitle,                                      // window class
                                               m_desc.pTitle,                                      // window title
                                               nativeDesc.style,                                   // window style
                                               nativeDesc.wndRect.left,                            // x position
                                               nativeDesc.wndRect.top,                             // y position
                                               nativeDesc.wndRect.right - nativeDesc.wndRect.left, // window width
                                               nativeDesc.wndRect.bottom - nativeDesc.wndRect.top, // window height
                                               NULL,                                               // parent window
                                               NULL,                                               // menu
                                               m_internals.hModule,                                // instance handle
                                               NULL                                                // additional application data
            );

            if( m_internals.hWnd == nullptr )
            {
                Destroy();
                return Results::FAIL;
            }

            m_internals.hDC = ::GetDC( m_internals.hWnd );
            if( !( m_internals.hDC ) )
            {
                Destroy();
                return Results::FAIL;
            }

            SetWindowLongPtr( m_internals.hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>( this ) );

            return Results::OK;
        }

        void Window::Destroy()
        {
            BGS_ASSERT( m_internals.hWnd != nullptr, "Invalid internal window handle." );
            BGS_ASSERT( m_internals.hDC != nullptr, "Invalid internal context." );
            if( m_internals.hWnd != nullptr )
            {
                if( m_internals.hDC != nullptr )
                {
                    ::ReleaseDC( m_internals.hWnd, m_internals.hDC );
                }
                ::DestroyWindow( m_internals.hWnd );
                UnregisterClass( m_desc.pTitle, m_internals.hModule );
            }
        }

        LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
        {

            Window* pWindow = reinterpret_cast<Window*>( GetWindowLongPtr( hWnd, GWLP_USERDATA ) );
            if( pWindow != nullptr )
            {
                Event::EventSystem* pEventSystem = pWindow->GetParent()->GetParent()->GetEventSystem();
                if( pEventSystem != nullptr )
                {
                    BGS_ASSERT( pEventSystem != nullptr, "No event system to handle events." )

                    switch( msg )
                    {
                        case WM_CLOSE:
                        {
                            Event::WindowCloseEvent e;
                            pEventSystem->TriggerEvent( e );

                            break;
                        }
                        case WM_DESTROY:
                        {
                            Event::WindowCloseEvent e;
                            pEventSystem->TriggerEvent( e );
                            ::PostQuitMessage( 0 );

                            break;
                        }
                        case WM_SETFOCUS:
                        {
                            Event::WindowFocusEvent e;
                            pEventSystem->TriggerEvent( e );

                            break;
                        }
                        case WM_KILLFOCUS:
                        {
                            Event::WindowLostFocusEvent e;
                            pEventSystem->TriggerEvent( e );

                            break;
                        }
                        case WM_KEYDOWN:
                        case WM_SYSKEYDOWN:
                        {
                            Input::KeyCode key       = static_cast<Input::KeyCode>( wParam );
                            uint16_t       repeatCnt = LOWORD( lParam );

                            Event::KeyPressedEvent e( key, repeatCnt );
                            pEventSystem->TriggerEvent( e );

                            break;
                        }
                        case WM_KEYUP:
                        case WM_SYSKEYUP:
                        {
                            Input::KeyCode key = static_cast<Input::KeyCode>( wParam );

                            Event::KeyReleasedEvent e( key );
                            pEventSystem->TriggerEvent( e );

                            break;
                        }
                        case WM_MOUSEMOVE:
                        {
                            uint32_t x = GET_X_LPARAM( lParam );
                            uint32_t y = GET_Y_LPARAM( lParam );

                            Event::MouseMovedEvent e( static_cast<float>( x ), static_cast<float>( y ) );
                            pEventSystem->TriggerEvent( e );

                            break;
                        }
                        case WM_LBUTTONDOWN:
                        {
                            SetCapture( hWnd );
                            Event::MouseButtonPressedEvent e( Input::Mouse::ButtonLeft );
                            pEventSystem->TriggerEvent( e );

                            break;
                        }
                        case WM_RBUTTONDOWN:
                        {
                            SetCapture( hWnd );
                            Event::MouseButtonPressedEvent e( Input::Mouse::ButtonRight );
                            pEventSystem->TriggerEvent( e );

                            break;
                        }
                        case WM_MBUTTONDOWN:
                        {
                            SetCapture( hWnd );
                            Event::MouseButtonPressedEvent e( Input::Mouse::ButtonMiddle );
                            pEventSystem->TriggerEvent( e );

                            break;
                        }
                        case WM_LBUTTONUP:
                        {
                            ReleaseCapture();
                            Event::MouseButtonReleasedEvent e( Input::Mouse::ButtonLeft );
                            pEventSystem->TriggerEvent( e );

                            break;
                        }
                        case WM_RBUTTONUP:
                        {
                            ReleaseCapture();
                            Event::MouseButtonReleasedEvent e( Input::Mouse::ButtonRight );
                            pEventSystem->TriggerEvent( e );

                            break;
                        }
                        case WM_MBUTTONUP:
                        {
                            ReleaseCapture();
                            Event::MouseButtonReleasedEvent e( Input::Mouse::ButtonMiddle );
                            pEventSystem->TriggerEvent( e );

                            break;
                        }
                        case WM_MOUSEWHEEL:
                        {
                            Event::MouseScrolledEvent e( 0.0f, GET_WHEEL_DELTA_WPARAM( wParam ) / 120.0f );
                            pEventSystem->TriggerEvent( e );

                            break;
                        }

                        default:
                            return DefWindowProc( hWnd, msg, wParam, lParam );
                    }
                }
            }

            return DefWindowProc( hWnd, msg, wParam, lParam );
        }

#else
#    error
#endif

    } // namespace Platform
} // namespace BIGOS