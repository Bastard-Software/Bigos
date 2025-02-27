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
        RESULT Create( Driver::Frontend::RenderSystem* pSystem );
        void   Destroy();

    private:
        Driver::Frontend::RenderSystem* m_pRenderSystem;
        Driver::Frontend::RenderDevice* m_pDevice;
        Driver::Frontend::RenderTarget* m_pColorRT;
        Driver::Frontend::RenderTarget* m_pDepthRT;
        Driver::Frontend::Buffer*       m_pVertexBuffer;
        Driver::Frontend::Buffer*       m_pIndexBuffer;
        Driver::Frontend::Buffer*       m_pConstantBuffer;
        Driver::Frontend::Buffer*       m_pStorageBuffer;
        Driver::Frontend::Shader*       m_pVS;
        Driver::Frontend::Shader*       m_pPS;
        Driver::Frontend::Pipeline*     m_pGraphicsPipeline;
        Driver::Frontend::Shader*       m_pCS;
        Driver::Frontend::Pipeline*     m_pComputePipeline;
        Driver::Frontend::Texture*      m_pSampledTexture;
        Driver::Frontend::Texture*      m_pStorageTexture;
        Driver::Frontend::Texture*      m_pSkyTexture;
    };
} // namespace BIGOS