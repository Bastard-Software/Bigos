#pragma once

#include "Driver/Frontend/RenderSystemTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            class BGS_API Pipeline final
            {
                friend class RenderSystem;

            public:
                Pipeline();
                ~Pipeline() = default;

            protected:
                RESULT Create( const GraphicsPipelineDesc& desc, RenderSystem* pSysytem );
                RESULT Create( const ComputePipelineDesc& desc, RenderSystem* pSysytem );
                void   Destroy();

            private:
                RESULT CreateComputeLayout( Shader* pCS );
                RESULT CreateGraphicsLayout( Shader* pVS, Shader* pPS, Shader* pDS, Shader* pHS, Shader* pGS );

            private:
                Backend::GraphicsPipelineDesc   m_graphicsDesc;
                Backend::ComputePipelineDesc    m_computeDesc;
                RenderSystem*                   m_pParent;
                Backend::BindingSetLayoutHandle m_hShaderResourceLayout;
                Backend::BindingSetLayoutHandle m_hSamplerLayout;
                Backend::PipelineLayoutHandle   m_hPipelineLayout;
                Backend::PipelineHandle         m_hPipeline;
                Backend::PIPELINE_TYPE          m_type;
            };

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS