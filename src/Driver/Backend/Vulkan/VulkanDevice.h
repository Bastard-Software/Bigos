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

                // Funcions needed for Vulkan backend
            public:
                RESULT FindSuitableQueue( QUEUE_TYPE type, QUEUE_PRIORITY_TYPE prio, uint32_t* familyIndex, uint32_t* queueIndex );
                void   FreeNativeQueue( uint32_t familyIndex, uint32_t queueIndex );

            protected:
                RESULT Create( const DeviceDesc& desc, VulkanFactory* pFactory );
                void   Destroy();

            private:
                RESULT CreateVkDevice();

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