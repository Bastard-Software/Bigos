#pragma once

#include "Core/CoreTypes.h"

#include "Core/Memory/MemorySystem.h"
#include "Driver/Frontend/RenderSystem.h"
#include "Platform/Event/EventSystem.h"
#include "Platform/Input/InputSystem.h"
#include "Platform/Window/WindowSystem.h"
#include "glm/glm.hpp"

namespace BIGOS
{
    struct BigosEngineDesc
    {
        Core::Memory::MemorySystemDesc     memorySystemDesc;
        Platform::Event::EventSystemDesc   eventSystemDesc;
        Platform::Input::InputSystemDesc   inputSystemDesc;
        Driver::Frontend::RenderSystemDesc renderSystemDesc;
        Platform::WindowSystemDesc         windowSystemDesc;
    };

    class BGS_API BigosEngine final
    {
    public:
        BigosEngine();
        ~BigosEngine() = default;

        Core::Memory::MemorySystem&     GetMemorySystem() { return m_memorySystem; }
        Driver::Frontend::RenderSystem& GetRenderSystem() { return m_renderSystem; }
        Platform::WindowSystem&         GetWindowSystem() { return m_windowSystem; }
        Platform::Event::EventSystem&   GetEventSystem() { return m_eventSystem; }
        Platform::Input::InputSystem&   GetInputSystem() { return m_inputSystem; }

        RESULT Create( const BigosEngineDesc& desc );
        void   Destroy();

    private:
        Core::Memory::MemorySystem     m_memorySystem;
        Platform::Event::EventSystem   m_eventSystem;
        Platform::Input::InputSystem   m_inputSystem;
        Platform::WindowSystem         m_windowSystem;
        Driver::Frontend::RenderSystem m_renderSystem;
    };

} // namespace BIGOS

extern "C"
{
    BGS_API BIGOS::RESULT CreateBigosEngine( const BIGOS::BigosEngineDesc& desc, BIGOS::BigosEngine** ppFramework );

    BGS_API void DestroyBigosEngine( BIGOS::BigosEngine** ppFramework );
};
