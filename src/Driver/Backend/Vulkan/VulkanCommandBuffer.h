#pragma once
#include "VulkanTypes.h"

#include "Driver/Backend/API.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            class BGS_API VulkanCommandBuffer final : public ICommandBuffer
            {
                friend class VulkanDevice;

            public:
                virtual void Begin( const BeginCommandBufferDesc& desc ) override;
                virtual void End() override;
                virtual void Reset() override;

                virtual void BeginRendering( const BeginRenderingDesc& desc ) override;
                virtual void EndRendering() override;

                virtual void SetViewports( uint32_t viewportCount, const ViewportDesc* pViewports ) override;
                virtual void SetScissors( uint32_t scissorCount, const ScissorDesc* pScissors ) override;

                virtual void SetPrimitiveTopology( PRIMITIVE_TOPOLOGY topology ) override;

                virtual void Draw( const DrawDesc& desc ) override;
                virtual void DrawIndexed( const DrawDesc& desc ) override;

                virtual void Barrier( uint32_t barrierCount, const BarierDesc* pBarriers ) override;

                virtual void SetPipeline( PipelineHandle handle, PIPELINE_TYPE type ) override;

            protected:
                RESULT Create( const CommandBufferDesc& desc, VulkanDevice* pDevice );
                void   Destroy();

            private:
                VulkanDevice* m_pParent;
            };
        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS