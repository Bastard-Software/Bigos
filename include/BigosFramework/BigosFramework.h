#pragma once
#include "Core/CoreTypes.h"
#include "Driver/Frontend/RenderSystemTypes.h"
#include "Platform/Event/EventTypes.h"
#include "Platform/PlatformTypes.h"

#include "Core/Memory/MemorySystem.h"

#ifdef CreateSemaphore
#    undef CreateSemaphore
#endif

namespace BIGOS
{
    struct BigosFrameworkDesc
    {
        Core::Memory::MemorySystemDesc     memorySystemDesc;
        Driver::Frontend::RenderSystemDesc renderSystemDesc;
        Platform::WindowSystemDesc         windowSystemDesc;
    };

    class BigosFramework;
} // namespace BIGOS

extern "C"
{
    BGS_API BIGOS::RESULT CreateBigosFramework( const BIGOS::BigosFrameworkDesc& desc, BIGOS::BigosFramework** ppFramework );

    BGS_API void DestroyBigosFramework( BIGOS::BigosFramework** ppFramework );
};

namespace BIGOS
{
    class BGS_API BigosFramework final
    {
    public:
        BigosFramework();
        ~BigosFramework() = default;

        // TODO: Refactor? We need sth like that
        RESULT DefaultInit( Driver::Backend::API_TYPE apiType );

        RESULT CreateRenderSystem( const Driver::Frontend::RenderSystemDesc& desc, Driver::Frontend::RenderSystem** ppSystem );
        void   DestroyRenderSystem( Driver::Frontend::RenderSystem** ppSystem );

        RESULT CreateWindowSystem( const Platform::WindowSystemDesc& desc, Platform::WindowSystem** ppSystem );
        void   DestroyWindowSystem( Platform::WindowSystem** ppSystem );

        RESULT CreateEventSystem( const Platform::Event::EventSystemDesc& desc, Platform::Event::EventSystem** ppSystem );
        void   DestroyEventSystem( Platform::Event::EventSystem** ppSystem );

        RESULT CreateInputSystem( const Platform::Input::InputSystemDesc& desc, Platform::Input::InputSystem** ppSystem );
        void   DestroyInputSystem( Platform::Input::InputSystem** ppSystem );

        Core::Memory::MemorySystem*     GetMemorySystem() { return &m_memorySystem; }
        Driver::Frontend::RenderSystem* GetRenderSystem() { return m_pRenderSystem; }
        Platform::WindowSystem*         GetWindowSystem() { return m_pWindowSystem; }
        Platform::Event::EventSystem*   GetEventSystem() { return m_pEventSystem; }
        Platform::Input::InputSystem*   GetInputSystem() { return m_pInputSystem; }

        RESULT Create( const BigosFrameworkDesc& desc );
        void   Destroy();

    private:
        Core::Memory::MemorySystem      m_memorySystem;
        Driver::Frontend::RenderSystem* m_pRenderSystem;
        Platform::WindowSystem*         m_pWindowSystem;
        Platform::Event::EventSystem*   m_pEventSystem;
        Platform::Input::InputSystem*   m_pInputSystem;
    };
} // namespace BIGOS