#pragma once

#include "VulkanTypes.h"

#include "Driver/Backend/API.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            class BGS_API VulkanSwapchain : public ISwapchain
            {
                friend class VulkanDevice;

            public:
                VulkanSwapchain();

                virtual RESULT Resize( const SwapchainResizeDesc& desc ) override;
                virtual RESULT Present( const SwapchainPresentDesc& desc ) override;
                virtual RESULT GetNextFrame( FrameInfo* pInfo ) override;

            protected:
                RESULT Create( const SwapchainDesc& desc, VulkanDevice* pDevice );
                void   Destroy();

            private:
                RESULT CreateVkSwapchain();
                RESULT GetBackBuffers();

            private:
                VulkanDevice* m_pParent;
                VkSurfaceKHR  m_surface;
                uint32_t      m_semaphoreNdx;
            };
        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS