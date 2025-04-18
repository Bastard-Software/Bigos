#pragma once
#include "Platform/PlatformTypes.h"

namespace BIGOS
{
    namespace Platform
    {
        class BGS_API Window
        {
            friend class WindowSystem;
            friend LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

        public:
            Window();
            ~Window() = default;

            void Show();
            void Update();

            void SetTitle( const char* pTitle );

            BGS_FORCEINLINE const WindowDesc&      GetDesc() const { return m_desc; }
            BGS_FORCEINLINE const WindowInternals& GetInternals() const { return m_internals; }
            BGS_FORCEINLINE WindowSystem*          GetParent() { return m_pParent; }

        protected:
            RESULT Create( const WindowDesc& desc, WindowSystem* pWndSystem );
            void   Destroy();

            void SetDescSize( uint32_t width, uint32_t height );

        private:
            WindowDesc      m_desc;
            WindowInternals m_internals;
            WindowSystem*   m_pParent;
        };
    } // namespace Platform
} // namespace BIGOS