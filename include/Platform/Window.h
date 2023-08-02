#pragma once
#include "Platform/PlatformTypes.h"

namespace BIGOS
{
    namespace Platform
    {
        class BGS_API Window
        {
            friend class WindowSystem;

        public:
            Window()  = default;
            ~Window() = default;

            // TODO: Hide, minimize?
            void Show();
            void Update();

            BGS_FORCEINLINE const WindowDesc&      GetDesc() const { return m_desc; }
            BGS_FORCEINLINE const WindowInternals& GetInternals() const { return m_internals; }

        protected:
            RESULT Create( const WindowDesc& desc, WindowSystem* pWndSystem );
            void   Destroy();

        private:
            WindowDesc      m_desc;
            WindowInternals m_internals;
            WindowSystem*   m_pParent;
        };
    } // namespace Platform
} // namespace BIGOS