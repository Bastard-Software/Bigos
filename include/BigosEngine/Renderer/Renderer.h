#pragma once

#include "BigosEngine/BigosTypes.h"

#include "Driver/Frontend/RenderSystemTypes.h"

namespace BIGOS
{
    class BGS_API Renderer
    {
        friend class Application;

    public:
        Renderer();
        ~Renderer() = default;

    protected:
        RESULT Create(Driver::Frontend::RenderSystem* pSystem);
        void   Destroy();

    private:
        Driver::Frontend::RenderSystem* m_pRenderSystem;
        Driver::Frontend::RenderDevice* m_pDevice;
        Driver::Frontend::RenderTarget* m_pColorRT;
        Driver::Frontend::RenderTarget* m_pDepthRT;
    };
} // namespace BIGOS