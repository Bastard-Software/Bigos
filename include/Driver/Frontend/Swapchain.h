#pragma once

#include "Driver/Frontend/RenderSystemTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            class BGS_API Swapchain final
            {
                friend class RenderDevice;

            public:
                Swapchain();
                ~Swapchain() = default;

                RESULT Resize( Platform::Window* pWindow );

            protected:
                RESULT Create( const SwapchainDesc& desc, RenderDevice* pDevice );
                void   Destroy();

            private:
                Backend::SwapchainDesc m_desc;
                Backend::ISwapchain*   m_pSwapchain;
                RenderDevice*          m_pParent;
            };

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS