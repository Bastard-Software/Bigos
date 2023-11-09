#pragma once
#include "D3D12Types.h"

#include "Driver/Backend/API.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            class BGS_API D3D12CommandBuffer final : public ICommandBuffer
            {
                friend class D3D12Device;

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

                virtual void Barrier( const BarierDesc& desc ) override;

                virtual void SetPipeline( PipelineHandle handle, PIPELINE_TYPE type ) override;
                virtual void SetBindingHeaps( uint32_t heapCount, const BindingHeapHandle* pHandle ) override;
                virtual void SetBindings( const SetBindingsDesc& desc ) override;

                virtual void BeginQuery( QueryPoolHandle handle, uint32_t queryNdx, QUERY_TYPE type ) override;
                virtual void EndQuery( QueryPoolHandle handle, uint32_t queryNdx, QUERY_TYPE type ) override;
                virtual void Timestamp( QueryPoolHandle handle, uint32_t queryNdx ) override;
                virtual void ResetQueryPool( QueryPoolHandle handle, uint32_t firstQuery, uint32_t queryCount ) override;
                virtual void CopyQueryResults( const CopyQueryResultsDesc& desc ) override;

            protected:
                RESULT Create( const CommandBufferDesc& desc, D3D12Device* pDevice );
                void   Destroy();

            private:
                D3D12_CPU_DESCRIPTOR_HANDLE m_boundColorRenderTargets[ Config::Driver::Pipeline::MAX_RENDER_TARGET_COUNT ];
                D3D12_CPU_DESCRIPTOR_HANDLE m_boundDepthStencilTarget;
                uint32_t                    m_colorRenderTargetCount;
                uint32_t                    m_rtvDescSize;
                uint32_t                    m_dsvDescSize;

                D3D12Device* m_pParent;
            };
        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS