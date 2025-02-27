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
                friend class RenderDevice;

            public:
                Pipeline();
                ~Pipeline() = default;

            protected:
                RESULT Create( const GraphicsPipelineDesc& desc, RenderDevice* pDevice );
                RESULT Create( const ComputePipelineDesc& desc, RenderDevice* pDevice );
                void   Destroy();

            private:
                RESULT CreateComputeLayout( Shader* pCS );
                RESULT CreateGraphicsLayout( Shader* pVS, Shader* pPS, Shader* pDS, Shader* pHS, Shader* pGS );

            private:
                Backend::GraphicsPipelineDesc   m_graphicsDesc;
                Backend::ComputePipelineDesc    m_computeDesc;
                RenderDevice*                   m_pParent;
                Backend::BindingSetLayoutHandle m_hShaderResourceLayout;
                Backend::BindingSetLayoutHandle m_hSamplerLayout;
                Backend::PipelineLayoutHandle   m_hPipelineLayout;
                Backend::PipelineHandle         m_hPipeline;
                Backend::PIPELINE_TYPE          m_type;
            };

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS