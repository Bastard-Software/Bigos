#include "Platform/Window.h"

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
            winClass.hInstance     = NULL;
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
                                               NULL,                                               // instance handle
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
                UnregisterClass( m_desc.pTitle, NULL );
            }
        }

        LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
        {
            switch( msg )
            {
                case WM_CLOSE:
                    break;
                case WM_DESTROY:
                    ::PostQuitMessage( 0 );
                    break;
                default:
                    return DefWindowProc( hWnd, msg, wParam, lParam );
            }

            return 0;
        }

#else
#    error
#endif

    } // namespace Platform
} // namespace BIGOS