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

                virtual void SetViewports( uint32_t viewportCount, const ViewportDesc* pViewports ) override;
                virtual void SetScissors( uint32_t scissorCount, const ScissorDesc* pScissors ) override;

                virtual void SetPrimitiveTopology( PRIMITIVE_TOPOLOGY topology ) override;

                virtual void Draw( const DrawDesc& desc ) override;
                virtual void DrawIndexed( const DrawDesc& desc ) override;

                virtual void Barrier( uint32_t barrierCount, const BarierDesc* pBarriers ) override;

                virtual void SetPipeline( PipelineHandle handle, PIPELINE_TYPE type ) override;
                virtual void SetBindingHeaps( BindingHeapHandle hShaderResourceHeap, BindingHeapHandle hSamplerHeap ) override;
                virtual void SetBinding( const SetBindingDesc& desc ) override;

            protected:
                RESULT Create( const CommandBufferDesc& desc, D3D12Device* pDevice );
                void   Destroy();

            private:
                D3D12Device* m_pParent;
            };
        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS