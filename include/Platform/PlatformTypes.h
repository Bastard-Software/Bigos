#pragma once
#include "Core/CoreTypes.h"

#if( BGS_WINDOWS )
#    include <Windows.h>
using LibraryHandle = HMODULE;
using WindowHandle  = HWND;
using WindowContext = HDC;
#else
#    error
#endif

namespace BIGOS
{
    namespace Platform
    {
        class Window;
        class WindowSystem;

        using WindowArray = HeapArray<Window*>;

        enum class WindowModes : uint8_t
        {
            WINDOW,
            FULLSCREEN,
            FULLSCREEN_WINDOW,
            _MAX_ENUM,
        };
        using WINDOW_MODE = WindowModes;

        struct WindowDesc
        {
            const char* pTitle;
            WINDOW_MODE mode;
            uint32_t    xPosition;
            uint32_t    yPosition;
            uint32_t    width;
            uint32_t    height;
        };

        struct WindowInternals
        {
#if( BGS_WINDOWS )
            WindowHandle  hWnd;
            WindowContext hDC;
#else
#    error
#endif
        };

        struct WindowSystemDesc
        {
            // TODO: Fill
        };

    } // namespace Platform
} // namespace BIGOS