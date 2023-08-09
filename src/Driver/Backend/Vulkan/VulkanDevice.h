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

            public:
                virtual RESULT CreateQueue( const QueueDesc& desc, IQueue** ppQueue ) override;
                virtual void   DestroyQueue( IQueue** ppQueue ) override;

                virtual RESULT CreateCommandPool( const CommandPoolDesc& desc, CommandPoolHandle* pHandle ) override;
                virtual void   DestroyCommandPool( CommandPoolHandle* pHandle ) override;
                virtual RESULT ResetCommandPool( CommandPoolHandle handle ) override;

                virtual RESULT CreateCommandBuffer( const CommandBufferDesc& desc, ICommandBuffer** ppCommandBuffer ) override;
                virtual void   DestroyCommandBuffer( ICommandBuffer** ppCommandBuffer ) override;

                virtual RESULT CreateShader( const ShaderDesc& desc, ShaderHandle* pHandle ) override;
                virtual void   DestroyShader( ShaderHandle* pHandle ) override;

                virtual RESULT CreatePipeline( const PipelineDesc& desc, PipelineHandle* pHandle ) override;
                virtual void   DestroyPipeline( PipelineHandle* pHandle ) override;

                virtual RESULT CreateFence( const FenceDesc& desc, FenceHandle* pHandle ) override;
                virtual void   DestroyFence( FenceHandle* pHandle ) override;
                virtual RESULT WaitForFences( const WaitForFencesDesc& desc, uint64_t timeout ) override;
                virtual RESULT SignalFence( uint64_t value, FenceHandle handle ) override;
                virtual RESULT GetFenceValue( FenceHandle handle, uint64_t* pValue ) override;

                virtual RESULT CreateSemaphore( const SemaphoreDesc& desc, SemaphoreHandle* pHandle ) override;
                virtual void   DestroySemaphore( SemaphoreHandle* pHandle ) override;

                // Funcions needed for Vulkan backend
            public:
                RESULT FindSuitableQueue( QUEUE_TYPE type, QUEUE_PRIORITY_TYPE prio, uint32_t* familyIndex, uint32_t* queueIndex );
                void   FreeNativeQueue( uint32_t familyIndex, uint32_t queueIndex );

            protected:
                RESULT Create( const DeviceDesc& desc, VulkanFactory* pFactory );
                void   Destroy();

            private:
                RESULT CreateVkDevice();
                RESULT CreateVkGraphicsPipeline( const GraphicsPipelineDesc& gpDesc, VkPipeline* pNativePipeline );

                void EnumerateNativeQueues();

                RESULT CheckVkExtensionSupport( uint32_t extCount, const char* const* ppQueriedExts );

            private:
                VulkanQueueProperties m_queueProperties;

                VulkanFactory* m_pParent = nullptr;

                // VolkDeviceTable m_deviceAPI;
            };
        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS