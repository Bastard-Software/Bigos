#pragma once
#include "D3D12Types.h"

#include "D3D12BindingAllocators.h"
#include "Driver/Backend/API.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {

            class BGS_API D3D12Device final : public IDevice
            {
                friend class D3D12Factory;

                struct D3D12DescriptorSystem
                {
                    ID3D12DescriptorHeap* pHeap;
                    PoolAllocator         allocator;
                };

                using D3D12HeapProperties        = D3D12_HEAP_PROPERTIES[ BGS_ENUM_COUNT( MemoryHeapTypes ) ];
                using D3D12GlobalDescriptorHeaps = D3D12DescriptorSystem[ 4 ];

            public:
                virtual RESULT CreateQueue( const QueueDesc& desc, IQueue** ppQueue ) override;
                virtual void   DestroyQueue( IQueue** ppQueue ) override;

                virtual RESULT CreateSwapchain( const SwapchainDesc& desc, ISwapchain** ppSwapchain ) override;
                virtual void   DestroySwapchain( ISwapchain** ppSwapchain ) override;

                virtual RESULT CreateCommandPool( const CommandPoolDesc& desc, CommandPoolHandle* pHandle ) override;
                virtual void   DestroyCommandPool( CommandPoolHandle* pHandle ) override;
                virtual RESULT ResetCommandPool( CommandPoolHandle handle ) override;

                virtual RESULT CreateCommandBuffer( const CommandBufferDesc& desc, ICommandBuffer** ppCommandBuffer ) override;
                virtual void   DestroyCommandBuffer( ICommandBuffer** ppCommandBuffer ) override;

                virtual RESULT CreateShader( const ShaderDesc& desc, ShaderHandle* pHandle ) override;
                virtual void   DestroyShader( ShaderHandle* pHandle ) override;

                virtual RESULT CreatePipelineLayout( const PipelineLayoutDesc& desc, PipelineLayoutHandle* pHandle ) override;
                virtual void   DestroyPipelineLayout( PipelineLayoutHandle* pHandle ) override;
                virtual RESULT CreatePipeline( const PipelineDesc& desc, PipelineHandle* pHandle ) override;
                virtual void   DestroyPipeline( PipelineHandle* pHandle ) override;

                virtual RESULT CreateFence( const FenceDesc& desc, FenceHandle* pHandle ) override;
                virtual void   DestroyFence( FenceHandle* pHandle ) override;
                virtual RESULT WaitForFences( const WaitForFencesDesc& desc, uint64_t timeout ) override;
                virtual RESULT SignalFence( uint64_t value, FenceHandle handle ) override;
                virtual RESULT GetFenceValue( FenceHandle handle, uint64_t* pValue ) override;

                virtual RESULT CreateSemaphore( const SemaphoreDesc& desc, SemaphoreHandle* pHandle ) override;
                virtual void   DestroySemaphore( SemaphoreHandle* pHandle ) override;

                virtual RESULT AllocateMemory( const AllocateMemoryDesc& desc, MemoryHandle* pHandle ) override;
                virtual void   FreeMemory( MemoryHandle* pHandle ) override;

                virtual RESULT CreateResource( const ResourceDesc& desc, ResourceHandle* pHandle ) override;
                virtual void   DestroyResource( ResourceHandle* pHandle ) override;
                virtual RESULT BindResourceMemory( const BindResourceMemoryDesc& desc ) override;
                virtual void   GetResourceAllocationInfo( ResourceHandle handle, ResourceAllocationInfo* pInfo ) override;
                virtual RESULT MapResource( const MapResourceDesc& desc, void** ppResource ) override;
                virtual RESULT UnmapResource( const MapResourceDesc& desc ) override;

                virtual RESULT CreateResourceView( const ResourceViewDesc& desc, ResourceViewHandle* pHandle ) override;
                virtual void   DestroyResourceView( ResourceViewHandle* pHandle ) override;
                virtual RESULT CreateSampler( const SamplerDesc& desc, SamplerHandle* pHandle ) override;
                virtual void   DestroySampler( SamplerHandle* pHandle ) override;

                virtual RESULT CreateCommandLayout( const CommandLayoutDesc& desc, CommandLayoutHandle* pHandle ) override;
                virtual void   DestroyCommandLayout( CommandLayoutHandle* pHandle ) override;

                virtual RESULT CreateBindingSetLayout( const BindingSetLayoutDesc& desc, BindingSetLayoutHandle* pHandle ) override;
                virtual void   DestroyBindingSetLayout( BindingSetLayoutHandle* pHandle ) override;
                virtual RESULT CreateBindingHeap( const BindingHeapDesc& desc, BindingHeapHandle* pHandle ) override;
                virtual void   DestroyBindingHeap( BindingHeapHandle* pHandle ) override;
                virtual void   GetBindingOffset( const GetBindingOffsetDesc& desc, uint64_t* pOffset ) override;
                virtual void   WriteBinding( const WriteBindingDesc& desc ) override;

                virtual RESULT CreateQueryPool( const QueryPoolDesc& desc, QueryPoolHandle* pHandle ) override;
                virtual void   DestroyQueryPool( QueryPoolHandle* pHandle ) override;

                // Function needed for D3D12 backend
            public:
                D3D12Factory*         GetParent() const { return m_pParent; }
                ID3D12DescriptorHeap* GetDescriptorHeap( D3D12_DESCRIPTOR_HEAP_TYPE type ) { return m_descHeaps[ BGS_ENUM_INDEX( type ) ].pHeap; }

            protected:
                RESULT Create( const DeviceDesc& desc, D3D12Factory* pParent );
                void   Destroy();

            private:
                RESULT   CreateD3D12Device();
                RESULT   CreateD3D12GraphicsPipeline( const GraphicsPipelineDesc& gpDesc, D3D12Pipeline** ppPipeline );
                RESULT   CreateD3D12ComputePipeline( const ComputePipelineDesc& cpDesc, D3D12Pipeline** ppPipeline );
                RESULT   CreateD3D12GlobalDescriptorHeaps();
                uint32_t CreateD3D12RTV( const TextureViewDesc& desc );
                uint32_t CreateD3D12DSV( const TextureViewDesc& desc );
                uint32_t CreateD3D12CBV( const BufferViewDesc& desc );
                uint32_t CreateD3D12SRV( const TextureViewDesc& desc );
                uint32_t CreateD3D12SRV( const TexelBufferViewDesc& desc );
                uint32_t CreateD3D12SRV( const BufferViewDesc& desc );
                uint32_t CreateD3D12UAV( const TextureViewDesc& desc );
                uint32_t CreateD3D12UAV( const TexelBufferViewDesc& desc );
                uint32_t CreateD3D12UAV( const BufferViewDesc& desc );
                void     CreateD3D12MemoryHeapProperties( const AllocateMemoryDesc& desc, D3D12_HEAP_PROPERTIES* pProps );
                void     QueryD3D12BindingsSize();

            private:
                D3D12HeapProperties        m_heapProperties;
                D3D12GlobalDescriptorHeaps m_descHeaps;
                uint64_t                   m_bindingSizes[ BGS_ENUM_COUNT( BindingTypes ) ];
                D3D12Factory*              m_pParent = nullptr;
            };

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS