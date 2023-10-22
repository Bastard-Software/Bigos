#pragma once

#include "VulkanTypes.h"

#include "Driver/Backend/API.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            class BGS_API VulkanDevice final : public IDevice
            {
                friend class VulkanFactory;

                struct VulkanQueueParams
                {
                    uint32_t            familyIndex;
                    uint32_t            queueIndex;
                    QUEUE_PRIORITY_TYPE priority;
                    bool_t              free;
                };

                struct VulkanQueueFamilyParams
                {
                    HeapArray<VulkanQueueParams> queueParams;
                    VkQueueFamilyProperties      familiyParams;
                };

                using VulkanQueueProperties = HeapArray<VulkanQueueFamilyParams>;
                using VulkanHeapProperties  = VkPhysicalDeviceMemoryProperties2;

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

                virtual RESULT CreateBindingHeapLayout( const BindingHeapLayoutDesc& desc, BindingHeapLayoutHandle* pHandle ) override;
                virtual void   DestroyBindingHeapLayout( BindingHeapLayoutHandle* pHandle ) override;
                virtual RESULT CreateBindingHeap( const BindingHeapDesc& desc, BindingHeapHandle* pHandle ) override;
                virtual void   DestroyBindingHeap( BindingHeapHandle* pHandle ) override;
                virtual void   CopyBinding( const CopyBindingDesc& desc ) override;

                virtual RESULT CreateQueryPool( const QueryPoolDesc& desc, QueryPoolHandle* pHandle ) override;
                virtual void   DestroyQueryPool( QueryPoolHandle* pHandle ) override;

                // Funcions needed for Vulkan backend
            public:
                RESULT FindSuitableQueue( QUEUE_TYPE type, QUEUE_PRIORITY_TYPE prio, uint32_t* familyIndex, uint32_t* queueIndex );
                void   FreeNativeQueue( uint32_t familyIndex, uint32_t queueIndex );

                BGS_FORCEINLINE VulkanFactory* GetParent() const { return m_pParent; }

            protected:
                RESULT Create( const DeviceDesc& desc, VulkanFactory* pFactory );
                void   Destroy();

            private:
                RESULT CreateVkDevice();
                RESULT CreateVkGraphicsPipeline( const GraphicsPipelineDesc& gpDesc, VkPipeline* pNativePipeline );
                RESULT CreateVkComputePipeline( const ComputePipelineDesc& cpDesc, VkPipeline* pNativePipeline );
                RESULT CreateVkBuffer( const ResourceDesc desc, VkBuffer* pBuff );
                RESULT CreateVkImage( const ResourceDesc desc, VkImage* pImg );

                void EnumerateNativeQueues();
                void QueryVkBindingsSize();

                RESULT CheckVkExtensionSupport( uint32_t extCount, const char* const* ppQueriedExts );

            private:
                VulkanQueueProperties m_queueProperties;
                VulkanHeapProperties  m_heapProperties;
                VulkanFactory*        m_pParent     = nullptr;
                uint64_t              m_bindingSize = 0;
                // VolkDeviceTable m_deviceAPI;
            };
        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS