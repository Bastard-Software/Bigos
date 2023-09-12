#define BGS_USE_D3D12_HANDLES 1

#include "D3D12CommandBuffer.h"

#include "BigosFramework/Config.h"
#include "D3D12Common.h"
#include "D3D12Device.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            RESULT D3D12CommandBuffer::Create( const CommandBufferDesc& desc, D3D12Device* pDevice )
            {
                BGS_ASSERT( pDevice != nullptr, "Device (pDevice) must be a valid pointer." );
                BGS_ASSERT( desc.pQueue != nullptr, "Queue (desc.pQueue) must be a valid pointer." );
                if( ( pDevice == nullptr ) || ( desc.pQueue == nullptr ) )
                {
                    return Results::FAIL;
                }

                m_pParent = pDevice;
                m_desc    = desc;

                ID3D12Device*              pNativeDevice           = m_pParent->GetHandle().GetNativeHandle();
                ID3D12CommandAllocator*    pNativeCommandAllocator = m_desc.hCommandPool.GetNativeHandle();
                ID3D12GraphicsCommandList* pNativeCommandList      = nullptr;
                D3D12_COMMAND_LIST_TYPE    type                    = m_desc.level == CommandBufferLevels::SECONDARY
                                                                         ? D3D12_COMMAND_LIST_TYPE_BUNDLE
                                                                         : MapBigosQueueTypeToD3D12CommandListType( m_desc.pQueue->GetDesc().type );

                if( FAILED( pNativeDevice->CreateCommandList( 0, type, pNativeCommandAllocator, nullptr, IID_PPV_ARGS( &pNativeCommandList ) ) ) )
                {
                    return Results::FAIL;
                }

                ID3D12GraphicsCommandList4* pNativeCommandList4 = nullptr;
                if( FAILED( pNativeCommandList->QueryInterface( &pNativeCommandList4 ) ) )
                {
                    pNativeCommandList->Release();
                    return Results::FAIL;
                }
                pNativeCommandList->Release();
                pNativeCommandList4->Close(); // To mimic vulkan behaviour (vulkan command buffers are closed on creation)

                m_handle = CommandBufferHandle( pNativeCommandList4 );

                return Results::OK;
            }

            void D3D12CommandBuffer::Destroy()
            {
                BGS_ASSERT( m_pParent != nullptr );

                if( ( m_handle != CommandBufferHandle() ) )
                {
                    ID3D12GraphicsCommandList4* pNativeCommandList = m_handle.GetNativeHandle();
                    RELEASE_COM_PTR( pNativeCommandList );
                }

                m_pParent = nullptr;
                m_handle  = CommandBufferHandle();
            }

            void D3D12CommandBuffer::Begin( const BeginCommandBufferDesc& desc )
            {
                ID3D12CommandAllocator*     pNativeAllocator   = m_desc.hCommandPool.GetNativeHandle();
                ID3D12GraphicsCommandList4* pNativeCommandList = m_handle.GetNativeHandle();
                desc;

                pNativeCommandList->Reset( pNativeAllocator, nullptr );
            }

            void D3D12CommandBuffer::End()
            {
                ID3D12GraphicsCommandList4* pNativeCommandList = m_handle.GetNativeHandle();

                pNativeCommandList->Close();
            }

            void D3D12CommandBuffer::Reset()
            {
                ID3D12CommandAllocator*     pNativeAllocator   = m_desc.hCommandPool.GetNativeHandle();
                ID3D12GraphicsCommandList4* pNativeCommandList = m_handle.GetNativeHandle();

                pNativeCommandList->Reset( pNativeAllocator, nullptr );
            }

            void D3D12CommandBuffer::BeginRendering( const BeginRenderingDesc& desc )
            {
                /* TODO: Handle
                    - set render targets
                    - clear render targets
                */
                desc;
            }

            void D3D12CommandBuffer::EndRendering()
            {
                // TODO: Nop?
            }

            void D3D12CommandBuffer::SetViewports( uint32_t viewportCount, const ViewportDesc* pViewports )
            {
                BGS_ASSERT( viewportCount < Config::Driver::Pipeline::MAX_VIEWPORT_COUNT, "Viewport count (viewportCount) must be less than %d",
                            Config::Driver::Pipeline::MAX_VIEWPORT_COUNT );
                BGS_ASSERT( pViewports != nullptr, "Viewport desc array (pViewport) must be a valid pointer" );

                ID3D12GraphicsCommandList4* pNativeCommandList = m_handle.GetNativeHandle();

                pNativeCommandList->RSSetViewports( viewportCount, reinterpret_cast<const D3D12_VIEWPORT*>( pViewports ) );
            }

            void D3D12CommandBuffer::SetScissors( uint32_t scissorCount, const ScissorDesc* pScissors )
            {
                BGS_ASSERT( scissorCount < Config::Driver::Pipeline::MAX_SCISSOR_COUNT, "Viewport count (viewportCount) must be less than %d",
                            Config::Driver::Pipeline::MAX_SCISSOR_COUNT );
                BGS_ASSERT( pScissors != nullptr, "Scissor desc array (pScissor) must be a valid pointer" );

                D3D12_RECT scissors[ Config::Driver::Pipeline::MAX_SCISSOR_COUNT ];

                for( index_t ndx = 0; ndx < static_cast<index_t>( scissorCount ); ++ndx )
                {
                    const ScissorDesc& currDesc    = pScissors[ ndx ];
                    D3D12_RECT&        currScissor = scissors[ ndx ];

                    currScissor.left   = static_cast<LONG>( currDesc.upperLeftX );
                    currScissor.top    = static_cast<LONG>( currDesc.upperLeftY );
                    currScissor.right  = static_cast<LONG>( currDesc.upperLeftX + currDesc.width );
                    currScissor.bottom = static_cast<LONG>( currDesc.upperLeftY + currDesc.height );
                }

                ID3D12GraphicsCommandList4* pNativeCommandList = m_handle.GetNativeHandle();

                pNativeCommandList->RSSetScissorRects( scissorCount, scissors );
            }

            void D3D12CommandBuffer::SetPrimitiveTopology( PRIMITIVE_TOPOLOGY topology )
            {
                ID3D12GraphicsCommandList4* pNativeCommandList = m_handle.GetNativeHandle();

                pNativeCommandList->IASetPrimitiveTopology( MapBigosPrimitiveTopologyToD3D12PrimitiveTopology( topology ) );
            }

            void D3D12CommandBuffer::Draw( const DrawDesc& desc )
            {
                ID3D12GraphicsCommandList4* pNativeCommandList = m_handle.GetNativeHandle();

                pNativeCommandList->DrawInstanced( desc.vertexCount, desc.instanceCount, desc.firstVertex, desc.firstInstance );
            }

            void D3D12CommandBuffer::DrawIndexed( const DrawDesc& desc )
            {
                ID3D12GraphicsCommandList4* pNativeCommandList = m_handle.GetNativeHandle();

                pNativeCommandList->DrawIndexedInstanced( desc.indexCount, desc.instanceCount, desc.firstIndex, desc.vertexOffset,
                                                          desc.firstInstance );
            }

            void D3D12CommandBuffer::Barrier( uint32_t barrierCount, const BarierDesc* pBarriers )
            {
                // TODO: not trivial
                barrierCount;
                pBarriers;
            }

            void D3D12CommandBuffer::SetPipeline( PipelineHandle handle, PIPELINE_TYPE type )
            {
                // TODO: After pipelines
                handle;
                type;
            }

            void D3D12CommandBuffer::SetBindingHeaps( BindingHeapHandle hShaderResourceHeap, BindingHeapHandle hSamplerHeap )
            {
                hShaderResourceHeap;
                hSamplerHeap;
            }

            void D3D12CommandBuffer::SetBinding( const SetBindingDesc& desc )
            {
                desc;
            }

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS