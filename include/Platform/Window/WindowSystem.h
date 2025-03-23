#pragma once
#include "Core/Memory/MemoryTypes.h"
#include "Platform/PlatformTypes.h"

#ifdef CreateWindow
#    undef CreateWindow
#endif

namespace BIGOS
{
    namespace Platform
    {
        class BGS_API WindowSystem final
        {
            friend class BigosEngine;

        public:
            WindowSystem();
            ~WindowSystem() = default;

            RESULT CreateWindow( const WindowDesc& desc, Window** ppWindow );
            void   DestroyWindow( Window** ppWindow );

            // TODO: Get created windows by param e.x. index, title
            BGS_FORCEINLINE const WindowSystemDesc& GetDesc() const { return m_desc; }
            BGS_FORCEINLINE BigosEngine*            GetParent() { return m_pParent; }

        protected:
            RESULT Create( const WindowSystemDesc& desc, Core::Memory::IAllocator* pAllocator, BigosEngine* pFramework );
            void   Destroy();

        private:
            WindowArray         m_windows;
            WindowSystemDesc    m_desc;
            BigosEngine*        m_pParent;
            Memory::IAllocator* m_pDefaultAllocator;
        };
    } // namespace Platform
} // namespace BIGOS