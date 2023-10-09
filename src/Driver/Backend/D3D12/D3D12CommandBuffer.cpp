#define BGS_USE_D3D12_HANDLES 1

#include "D3D12CommandBuffer.h"

#include "BigosFramework/Config.h"
#include "D3D12Common.h"
#include "D3D12Device.h"
#include "D3D12Pipeline.h"
#include "D3D12Resource.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            static void SetGraphicsPipelineLayout( ID3D12GraphicsCommandList* pList, ID3D12RootSignature* pSig )
            {
                pList->SetGraphicsRootSignature( pSig );
            }
            static void SetComputePipelineLayout( ID3D12GraphicsCommandList* pList, ID3D12RootSignature* pSig )
            {
                pList->SetComputeRootSignature( pSig );
            }

            void ( *pBindPipelineLayoutFn[] )( ID3D12GraphicsCommandList*, ID3D12RootSignature* ) = { SetGraphicsPipelineLayout,
                                                                                                      SetComputePipelineLayout,
                                                                                                      SetComputePipelineLayout };

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

                ID3D12GraphicsCommandList7* pNativeCommandList7 = nullptr;
                if( FAILED( pNativeCommandList->QueryInterface( &pNativeCommandList7 ) ) )
                {
                    pNativeCommandList->Release();
                    return Results::FAIL;
                }
                pNativeCommandList->Release();
                pNativeCommandList7->Close(); // To mimic vulkan behaviour (vulkan command buffers are closed on creation)

                m_handle = CommandBufferHandle( pNativeCommandList7 );

                return Results::OK;
            }

            void D3D12CommandBuffer::Destroy()
            {
                BGS_ASSERT( m_pParent != nullptr );

                if( ( m_handle != CommandBufferHandle() ) )
                {
                    ID3D12GraphicsCommandList7* pNativeCommandList = m_handle.GetNativeHandle();
                    RELEASE_COM_PTR( pNativeCommandList );
                }

                m_pParent = nullptr;
                m_handle  = CommandBufferHandle();
            }

            void D3D12CommandBuffer::Begin( const BeginCommandBufferDesc& desc )
            {
                ID3D12CommandAllocator*     pNativeAllocator   = m_desc.hCommandPool.GetNativeHandle();
                ID3D12GraphicsCommandList7* pNativeCommandList = m_handle.GetNativeHandle();
                desc;

                pNativeCommandList->Reset( pNativeAllocator, nullptr );
            }

            void D3D12CommandBuffer::End()
            {
                ID3D12GraphicsCommandList7* pNativeCommandList = m_handle.GetNativeHandle();

                pNativeCommandList->Close();
            }

            void D3D12CommandBuffer::Reset()
            {
                ID3D12CommandAllocator*     pNativeAllocator   = m_desc.hCommandPool.GetNativeHandle();
                ID3D12GraphicsCommandList7* pNativeCommandList = m_handle.GetNativeHandle();

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

                ID3D12GraphicsCommandList7* pNativeCommandList = m_handle.GetNativeHandle();

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

                ID3D12GraphicsCommandList7* pNativeCommandList = m_handle.GetNativeHandle();

                pNativeCommandList->RSSetScissorRects( scissorCount, scissors );
            }

            void D3D12CommandBuffer::SetPrimitiveTopology( PRIMITIVE_TOPOLOGY topology )
            {
                ID3D12GraphicsCommandList7* pNativeCommandList = m_handle.GetNativeHandle();

                pNativeCommandList->IASetPrimitiveTopology( MapBigosPrimitiveTopologyToD3D12PrimitiveTopology( topology ) );
            }

            void D3D12CommandBuffer::Draw( const DrawDesc& desc )
            {
                ID3D12GraphicsCommandList7* pNativeCommandList = m_handle.GetNativeHandle();

                pNativeCommandList->DrawInstanced( desc.vertexCount, desc.instanceCount, desc.firstVertex, desc.firstInstance );
            }

            void D3D12CommandBuffer::DrawIndexed( const DrawDesc& desc )
            {
                ID3D12GraphicsCommandList7* pNativeCommandList = m_handle.GetNativeHandle();

                pNativeCommandList->DrawIndexedInstanced( desc.indexCount, desc.instanceCount, desc.firstIndex, desc.vertexOffset,
                                                          desc.firstInstance );
            }

            void D3D12CommandBuffer::Barrier( const BarierDesc& desc )
            {
                BGS_ASSERT( desc.globalBarrierCount <= Config::Driver::Synchronization::MAX_GLOBAL_BARRIER_COUNT,
                            "Global barrier count (desc.globalBarrierCount) must be less than %d.",
                            Config::Driver::Synchronization::MAX_GLOBAL_BARRIER_COUNT );
                BGS_ASSERT( desc.bufferBarrierCount <= Config::Driver::Synchronization::MAX_BUFFER_BARRIER_COUNT,
                            "Buffer barrier count (desc.bufferBarrierCount) must be less than %d.",
                            Config::Driver::Synchronization::MAX_BUFFER_BARRIER_COUNT );
                BGS_ASSERT( desc.textureBarrierCount <= Config::Driver::Synchronization::MAX_TEXTURE_BARRIER_COUNT,
                            "Texture barrier count (desc.textureBarrierCount) must be less than %d.",
                            Config::Driver::Synchronization::MAX_TEXTURE_BARRIER_COUNT );
                BGS_ASSERT( ( desc.bufferBarrierCount != 0 ) || ( desc.textureBarrierCount != 0 ) || ( desc.globalBarrierCount != 0 ),
                            "No valid barriers." );
                BGS_ASSERT( ( desc.pBufferBarriers != nullptr ) || ( desc.pTextureBarriers != nullptr ) || ( desc.pGlobalBarriers != nullptr ),
                            "No valid barriers." );

                D3D12_GLOBAL_BARRIER  globalBarriers[ Config::Driver::Synchronization::MAX_GLOBAL_BARRIER_COUNT ];
                D3D12_BUFFER_BARRIER  bufferBarriers[ Config::Driver::Synchronization::MAX_BUFFER_BARRIER_COUNT ];
                D3D12_TEXTURE_BARRIER textureBarriers[ Config::Driver::Synchronization::MAX_TEXTURE_BARRIER_COUNT ];

                D3D12_BARRIER_GROUP barriers[ 3 ];
                barriers[ 0 ].Type             = D3D12_BARRIER_TYPE_GLOBAL;
                barriers[ 0 ].NumBarriers      = desc.globalBarrierCount;
                barriers[ 0 ].pGlobalBarriers  = globalBarriers;
                barriers[ 1 ].Type             = D3D12_BARRIER_TYPE_BUFFER;
                barriers[ 1 ].NumBarriers      = desc.bufferBarrierCount;
                barriers[ 1 ].pBufferBarriers  = bufferBarriers;
                barriers[ 2 ].Type             = D3D12_BARRIER_TYPE_TEXTURE;
                barriers[ 2 ].NumBarriers      = desc.textureBarrierCount;
                barriers[ 2 ].pTextureBarriers = textureBarriers;

                for( index_t ndx = 0; ndx < static_cast<index_t>( desc.globalBarrierCount ); ++ndx )
                {
                    const GlobalBarrierDesc& currDesc = desc.pGlobalBarriers[ ndx ];
                    D3D12_GLOBAL_BARRIER&    barrier  = globalBarriers[ ndx ];

                    barrier.AccessBefore = MapBigosAccessFlagsToD3D12BarrierAccess( currDesc.srcAccess );
                    barrier.SyncBefore   = MapBigosPipelineStageFlagsToD3D12BarrierSync( currDesc.srcStage );
                    barrier.AccessAfter  = MapBigosAccessFlagsToD3D12BarrierAccess( currDesc.dstAccess );
                    barrier.SyncAfter    = MapBigosPipelineStageFlagsToD3D12BarrierSync( currDesc.dstStage );
                }
                for( index_t ndx = 0; ndx < static_cast<index_t>( desc.bufferBarrierCount ); ++ndx )
                {
                    const BufferBarrierDesc& currDesc = desc.pBufferBarriers[ ndx ];
                    D3D12_BUFFER_BARRIER&    barrier  = bufferBarriers[ ndx ];

                    barrier.AccessBefore = MapBigosAccessFlagsToD3D12BarrierAccess( currDesc.srcAccess );
                    barrier.SyncBefore   = MapBigosPipelineStageFlagsToD3D12BarrierSync( currDesc.srcStage );
                    barrier.AccessAfter  = MapBigosAccessFlagsToD3D12BarrierAccess( currDesc.dstAccess );
                    barrier.SyncAfter    = MapBigosPipelineStageFlagsToD3D12BarrierSync( currDesc.dstStage );
                    barrier.pResource    = currDesc.hResouce.GetNativeHandle()->pNativeResource;
                    barrier.Offset       = currDesc.bufferRange.offset;
                    barrier.Size         = currDesc.bufferRange.size;
                }
                for( index_t ndx = 0; ndx < static_cast<index_t>( desc.textureBarrierCount ); ++ndx )
                {
                    const TextureBarrierDesc& currDesc = desc.pTextureBarriers[ ndx ];
                    D3D12_TEXTURE_BARRIER&    barrier  = textureBarriers[ ndx ];

                    barrier.AccessBefore                      = MapBigosAccessFlagsToD3D12BarrierAccess( currDesc.srcAccess );
                    barrier.SyncBefore                        = MapBigosPipelineStageFlagsToD3D12BarrierSync( currDesc.srcStage );
                    barrier.LayoutBefore                      = MapBigosTextureLayoutToD3D12BarierrLayout( currDesc.srcLayout );
                    barrier.AccessAfter                       = MapBigosAccessFlagsToD3D12BarrierAccess( currDesc.dstAccess );
                    barrier.SyncAfter                         = MapBigosPipelineStageFlagsToD3D12BarrierSync( currDesc.dstStage );
                    barrier.LayoutAfter                       = MapBigosTextureLayoutToD3D12BarierrLayout( currDesc.dstLayout );
                    barrier.Subresources.FirstPlane           = MapBigosTextureComponentFlagsToD3D12PlaneSlice( currDesc.textureRange.components );
                    barrier.Subresources.NumPlanes            = 1;
                    barrier.Subresources.IndexOrFirstMipLevel = currDesc.textureRange.mipLevel;
                    barrier.Subresources.NumMipLevels         = currDesc.textureRange.mipLevelCount;
                    barrier.Subresources.FirstArraySlice      = currDesc.textureRange.arrayLayer;
                    barrier.Subresources.NumArraySlices       = currDesc.textureRange.arrayLayerCount;
                    barrier.Flags                             = D3D12_TEXTURE_BARRIER_FLAG_NONE; // TODO: Handle! Very important!
                }

                ID3D12GraphicsCommandList7* pNativeCommandList = m_handle.GetNativeHandle();

                pNativeCommandList->Barrier( 3, barriers );
            }

            void D3D12CommandBuffer::SetPipeline( PipelineHandle handle, PIPELINE_TYPE type )
            {
                BGS_ASSERT( handle != PipelineHandle(), "Pipeline (handle) must be a valid handle." );

                ID3D12GraphicsCommandList7* pNativeCommandList = m_handle.GetNativeHandle();
                D3D12Pipeline*              pPipeline          = handle.GetNativeHandle();

                pNativeCommandList->SetPipelineState( pPipeline->pPipeline );
                pBindPipelineLayoutFn[ BGS_ENUM_INDEX( type ) ]( pNativeCommandList, pPipeline->pRootSignature );
            }

            void D3D12CommandBuffer::SetBindingHeaps( uint32_t heapCount, const BindingHeapHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Binding heap (pHandle) must be a valid array." );
                BGS_ASSERT( heapCount <= 2, "Binding heap count (heapCount) must be less or equal 2." );

                ID3D12GraphicsCommandList7* pNativeCommandList = m_handle.GetNativeHandle();
                ID3D12DescriptorHeap*       pHeaps[ 2 ];
                for( index_t ndx = 0; ndx < static_cast<index_t>( heapCount ); ++ndx )
                {
                    pHeaps[ ndx ] = pHandle[ ndx ].GetNativeHandle();
                }

                pNativeCommandList->SetDescriptorHeaps( heapCount, pHeaps );
            }

            void D3D12CommandBuffer::SetBinding( const SetBindingDesc& desc )
            {
                desc;
            }

            void D3D12CommandBuffer::BeginQuery( QueryPoolHandle handle, uint32_t queryNdx, QUERY_TYPE type )
            {
                BGS_ASSERT( handle != QueryPoolHandle(), "Query pool (handle) must be a valid handle." );
                BGS_ASSERT( type != QueryTypes::TIMESTAMP, "Query type (type) must not be QueryTypes::TIMESTAMP." );

                ID3D12GraphicsCommandList7* pNativeCommandList = m_handle.GetNativeHandle();

                pNativeCommandList->BeginQuery( handle.GetNativeHandle(), MapBigosQueryTypeToD3D12QueryType( type ), queryNdx );
            }

            void D3D12CommandBuffer::EndQuery( QueryPoolHandle handle, uint32_t queryNdx, QUERY_TYPE type )
            {
                BGS_ASSERT( handle != QueryPoolHandle(), "Query pool (handle) must be a valid handle." );
                BGS_ASSERT( type != QueryTypes::TIMESTAMP, "Query type (type) must not be QueryTypes::TIMESTAMP." );

                ID3D12GraphicsCommandList7* pNativeCommandList = m_handle.GetNativeHandle();

                pNativeCommandList->EndQuery( handle.GetNativeHandle(), MapBigosQueryTypeToD3D12QueryType( type ), queryNdx );
            }

            void D3D12CommandBuffer::Timestamp( QueryPoolHandle handle, uint32_t queryNdx )
            {
                BGS_ASSERT( handle != QueryPoolHandle(), "Query pool (handle) must be a valid handle." );

                ID3D12GraphicsCommandList7* pNativeCommandList = m_handle.GetNativeHandle();

                pNativeCommandList->EndQuery( handle.GetNativeHandle(), D3D12_QUERY_TYPE_TIMESTAMP, queryNdx );
            }

            void D3D12CommandBuffer::ResetQueryPool( QueryPoolHandle, uint32_t, uint32_t )
            {
                // This function does not require implementation in D3D12
            }

            void D3D12CommandBuffer::CopyQueryResults( const CopyQueryResultsDesc& desc )
            {
                BGS_ASSERT( desc.hQueryPool != QueryPoolHandle(), "Query pool (desc.hQueryPool) must be a valid handle." );
                BGS_ASSERT( desc.hBuffer != ResourceHandle(), "Resource (desc.hBuffer) must be a valid handle." );

                ID3D12GraphicsCommandList7* pNativeCommandList = m_handle.GetNativeHandle();

                pNativeCommandList->ResolveQueryData( desc.hQueryPool.GetNativeHandle(), MapBigosQueryTypeToD3D12QueryType( desc.type ),
                                                      desc.firstQuery, desc.queryCount, desc.hBuffer.GetNativeHandle()->pNativeResource,
                                                      desc.bufferOffset );
            }

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS