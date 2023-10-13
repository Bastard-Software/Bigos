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
                virtual void ClearBoundColorRenderTarget( uint32_t index, const ColorValue& clearValue, uint32_t rectCount,
                                                          const Rect2D* pClearRects ) override;
                virtual void ClearBoundDepthStencilTarget( const DepthStencilValue& clearValue, TextureComponentFlags components, uint32_t rectCount,
                                                           const Rect2D* pClearRects ) override;

                virtual void SetViewports( uint32_t viewportCount, const ViewportDesc* pViewports ) override;
                virtual void SetScissors( uint32_t scissorCount, const ScissorDesc* pScissors ) override;

                virtual void SetPrimitiveTopology( PRIMITIVE_TOPOLOGY topology ) override;

                virtual void SetVertexBuffers( uint32_t startBinding, uint32_t bufferCount, const VertexBufferDesc* pVertexBuffers ) override;
                virtual void SetIndexBuffer( const IndexBufferDesc& desc ) override;

                virtual void Draw( const DrawDesc& desc ) override;
                virtual void DrawIndexed( const DrawDesc& desc ) override;

                virtual void Barrier( const BarierDesc& barrier ) override;

                virtual void SetPipeline( PipelineHandle handle, PIPELINE_TYPE type ) override;
                virtual void SetBindingHeaps( uint32_t heapCount, const BindingHeapHandle* pHandle ) override;
                virtual void SetBinding( const SetBindingDesc& desc ) override;

                virtual void BeginQuery( QueryPoolHandle handle, uint32_t queryNdx, QUERY_TYPE type ) override;
                virtual void EndQuery( QueryPoolHandle handle, uint32_t queryNdx, QUERY_TYPE type ) override;
                virtual void Timestamp( QueryPoolHandle handle, uint32_t queryNdx ) override;
                virtual void ResetQueryPool( QueryPoolHandle handle, uint32_t firstQuery, uint32_t queryCount ) override;
                virtual void CopyQueryResults( const CopyQueryResultsDesc& desc ) override;

            protected:
                RESULT Create( const CommandBufferDesc& desc, VulkanDevice* pDevice );
                void   Destroy();

            private:
                VulkanDevice* m_pParent;
            };
        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS