#pragma once
#include "APITypes.h"

#ifdef CreateSemaphore
#    undef CreateSemaphore
#endif

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {

            class BGS_API BGS_API_INTERFACE IAdapter
            {
            public:
                virtual ~IAdapter() = default;

                BGS_FORCEINLINE const AdapterInfo& GetInfo() const { return m_info; }

                BGS_FORCEINLINE AdapterHandle GetHandle() const { return m_handle; }

            protected:
                AdapterInfo   m_info;
                AdapterHandle m_handle;
            };

            class BGS_API BGS_API_INTERFACE IFactory
            {
            public:
                virtual ~IFactory() = default;

                virtual RESULT CreateDevice( const DeviceDesc& desc, IDevice** ppDevice ) = 0;
                virtual void   DestroyDevice( IDevice** ppDevice )                        = 0;

                BGS_FORCEINLINE const AdapterArray& GetAdapters() const { return m_adapters; }
                BGS_FORCEINLINE const FactoryDesc&  GetDesc() const { return m_desc; }
                BGS_FORCEINLINE const FactoryHandle GetHandle() const { return m_handle; }

            protected:
                FactoryDesc   m_desc;
                AdapterArray  m_adapters;
                FactoryHandle m_handle;
            };

            class BGS_API BGS_API_INTERFACE IDevice
            {
            public:
                virtual ~IDevice() = default;

                virtual RESULT CreateQueue( const QueueDesc& desc, IQueue** ppQueue ) = 0;
                virtual void   DestroyQueue( IQueue** ppQueue )                       = 0;

                virtual RESULT CreateSwapchain( const SwapchainDesc& desc, ISwapchain** ppSwapchain ) = 0;
                virtual void   DestroySwapchain( ISwapchain** ppSwapchain )                           = 0;

                virtual RESULT CreateCommandPool( const CommandPoolDesc& desc, CommandPoolHandle* pHandle ) = 0;
                virtual void   DestroyCommandPool( CommandPoolHandle* pHandle )                             = 0;
                virtual RESULT ResetCommandPool( CommandPoolHandle handle )                                 = 0;

                virtual RESULT CreateCommandBuffer( const CommandBufferDesc& desc, ICommandBuffer** ppCommandBuffer ) = 0;
                virtual void   DestroyCommandBuffer( ICommandBuffer** ppCommandBuffer )                               = 0;

                virtual RESULT CreateShader( const ShaderDesc& desc, ShaderHandle* pHandle ) = 0;
                virtual void   DestroyShader( ShaderHandle* pHandle )                        = 0;

                virtual RESULT CreatePipelineLayout( const PipelineLayoutDesc& desc, PipelineLayoutHandle* pHandle ) = 0;
                virtual void   DestroyPipelineLayout( PipelineLayoutHandle* pHandle )                                = 0;
                virtual RESULT CreatePipeline( const PipelineDesc& desc, PipelineHandle* pHandle )                   = 0;
                virtual void   DestroyPipeline( PipelineHandle* pHandle )                                            = 0;

                virtual RESULT CreateFence( const FenceDesc& desc, FenceHandle* pHandle )       = 0;
                virtual void   DestroyFence( FenceHandle* pHandle )                             = 0;
                virtual RESULT WaitForFences( const WaitForFencesDesc& desc, uint64_t timeout ) = 0;
                virtual RESULT SignalFence( uint64_t value, FenceHandle handle )                = 0;
                virtual RESULT GetFenceValue( FenceHandle handle, uint64_t* pValue )            = 0;

                virtual RESULT CreateSemaphore( const SemaphoreDesc& desc, SemaphoreHandle* pHandle ) = 0;
                virtual void   DestroySemaphore( SemaphoreHandle* pHandle )                           = 0;

                virtual RESULT AllocateMemory( const AllocateMemoryDesc& desc, MemoryHandle* pHandle ) = 0;
                virtual void   FreeMemory( MemoryHandle* pHandle )                                     = 0;

                virtual RESULT CreateResource( const ResourceDesc& desc, ResourceHandle* pHandle )               = 0;
                virtual void   DestroyResource( ResourceHandle* pHandle )                                        = 0;
                virtual RESULT BindResourceMemory( const BindResourceMemoryDesc& desc )                          = 0;
                virtual void   GetResourceAllocationInfo( ResourceHandle handle, ResourceAllocationInfo* pInfo ) = 0;
                virtual RESULT MapResource( const MapResourceDesc& desc, void** ppResource )                     = 0;
                virtual RESULT UnmapResource( const MapResourceDesc& desc )                                      = 0;

                virtual RESULT CreateResourceView( const ResourceViewDesc& desc, ResourceViewHandle* pHandle ) = 0;
                virtual void   DestroyResourceView( ResourceViewHandle* pHandle )                              = 0;
                virtual RESULT CreateSampler( const SamplerDesc& desc, SamplerHandle* pHandle )                = 0;
                virtual void   DestroySampler( SamplerHandle* pHandle )                                        = 0;

                virtual RESULT CreateBindingSetLayout( const BindingSetLayoutDesc& desc, BindingSetLayoutHandle* pHandle ) = 0;
                virtual void   DestroyBindingSetLayout( BindingSetLayoutHandle* pHandle )                                  = 0;
                virtual RESULT CreateBindingHeap( const BindingHeapDesc& desc, BindingHeapHandle* pHandle )                = 0;
                virtual void   DestroyBindingHeap( BindingHeapHandle* pHandle )                                            = 0;
                virtual void   GetBindingOffset( const GetBindingOffsetDesc& desc, uint64_t* pOffset )                     = 0;
                virtual void   WriteBinding( const WriteBindingDesc& desc ) = 0; // TODO: For now, writes only one binding at the time.

                virtual RESULT CreateQueryPool( const QueryPoolDesc& desc, QueryPoolHandle* pHandle ) = 0;
                virtual void   DestroyQueryPool( QueryPoolHandle* pHandle )                           = 0;

                BGS_FORCEINLINE const DeviceLimits& GetLimits() const { return m_limits; }
                BGS_FORCEINLINE const DeviceDesc&   GetDesc() const { return m_desc; }
                BGS_FORCEINLINE const DeviceHandle  GetHandle() const { return m_handle; }

            protected:
                DeviceLimits m_limits;
                DeviceDesc   m_desc;
                DeviceHandle m_handle;
            };

            class BGS_API BGS_API_INTERFACE IQueue
            {
            public:
                virtual ~IQueue() = default;

                virtual RESULT Submit( const QueueSubmitDesc& desc ) = 0;

                BGS_FORCEINLINE const QueueLimits& GetLimits() const { return m_limits; }
                BGS_FORCEINLINE const QueueDesc&   GetDesc() const { return m_desc; }
                BGS_FORCEINLINE const QueueHandle  GetHandle() const { return m_handle; }

            protected:
                QueueLimits m_limits;
                QueueDesc   m_desc;
                QueueHandle m_handle;
            };

            class BGS_API BGS_API_INTERFACE ISwapchain
            {
            public:
                virtual ~ISwapchain() = default;

                virtual RESULT Resize( const SwapchainResizeDesc& desc )   = 0;
                virtual RESULT Present( const SwapchainPresentDesc& desc ) = 0;
                virtual RESULT GetNextFrame( FrameInfo* pInfo )            = 0;

                BGS_FORCEINLINE const SwapchainDesc&   GetDesc() const { return m_desc; }
                BGS_FORCEINLINE const BackBufferArray& GetBackBuffers() const { return m_backBuffers; }
                BGS_FORCEINLINE const SwapchainHandle  GetHandle() const { return m_handle; }

            protected:
                SwapchainDesc   m_desc;
                SwapchainHandle m_handle;
                BackBufferArray m_backBuffers;
            };

            class BGS_API BGS_API_INTERFACE ICommandBuffer
            {
            public:
                virtual ~ICommandBuffer() = default;

                virtual void Begin( const BeginCommandBufferDesc& desc ) = 0;
                virtual void End()                                       = 0;
                virtual void Reset()                                     = 0;

                virtual void BeginRendering( const BeginRenderingDesc& desc )          = 0;
                virtual void EndRendering()                                            = 0;
                virtual void ClearBoundColorRenderTarget( uint32_t index, const ColorValue& clearValue, uint32_t rectCount,
                                                          const Rect2D* pClearRects )  = 0;
                virtual void ClearBoundDepthStencilTarget( const DepthStencilValue& clearValue, TextureComponentFlags components, uint32_t rectCount,
                                                           const Rect2D* pClearRects ) = 0;

                virtual void SetPipeline( PipelineHandle handle, PIPELINE_TYPE type )                = 0;
                virtual void SetBindingHeaps( uint32_t heapCount, const BindingHeapHandle* pHandle ) = 0;
                virtual void SetBindings( const SetBindingsDesc& desc )                              = 0;

                virtual void PushConstants( const PushConstantsDesc& desc ) = 0;

                virtual void SetViewports( uint32_t viewportCount, const ViewportDesc* pViewports ) = 0;
                virtual void SetScissors( uint32_t scissorCount, const ScissorDesc* pScissors )     = 0;

                virtual void SetPrimitiveTopology( PRIMITIVE_TOPOLOGY topology ) = 0;

                virtual void SetVertexBuffers( uint32_t startBinding, uint32_t bufferCount, const VertexBufferDesc* pVertexBuffers ) = 0;
                virtual void SetIndexBuffer( const IndexBufferDesc& desc )                                                           = 0;

                virtual void Draw( const DrawDesc& desc )        = 0;
                virtual void DrawIndexed( const DrawDesc& desc ) = 0;

                virtual void Dispatch( const DispatchDesc& desc ) = 0;

                virtual void Barrier( const BarierDesc& desc ) = 0;

                virtual void CopyBuffer( const CopyBufferDesc& desc )                 = 0;
                virtual void CopyTexture( const CopyTextureDesc& desc )               = 0;
                virtual void CopyBuferToTexture( const CopyBufferTextureDesc& desc )  = 0;
                virtual void CopyTextureToBuffer( const CopyBufferTextureDesc& desc ) = 0;

                virtual void BeginQuery( QueryPoolHandle handle, uint32_t queryNdx, QUERY_TYPE type )           = 0;
                virtual void EndQuery( QueryPoolHandle handle, uint32_t queryNdx, QUERY_TYPE type )             = 0;
                virtual void Timestamp( QueryPoolHandle handle, uint32_t queryNdx )                             = 0;
                virtual void ResetQueryPool( QueryPoolHandle handle, uint32_t firstQuery, uint32_t queryCount ) = 0;
                virtual void CopyQueryResults( const CopyQueryResultsDesc& desc )                               = 0;

                BGS_FORCEINLINE const CommandBufferDesc&  GetDesc() const { return m_desc; }
                BGS_FORCEINLINE const CommandBufferHandle GetHandle() const { return m_handle; }

            protected:
                CommandBufferDesc   m_desc;
                CommandBufferHandle m_handle;
            };

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS