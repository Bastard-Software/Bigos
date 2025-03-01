#include "Driver/Frontend/Swapchain.h"

#include "Driver/Frontend/Contexts.h"
#include "Driver/Frontend/RenderDevice.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {
            Swapchain::Swapchain()
                : m_desc()
                , m_pSwapchain( nullptr )
                , m_pParent( nullptr )
            {
            }

            RESULT Swapchain::Resize( Platform::Window* pWindow )
            {
                BGS_ASSERT( pWindow != nullptr, "Window (pWindow) must be a valid pointer." );

                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                // TODO: Wait for queue job completion
                if( m_pSwapchain != nullptr )
                {
                    pAPIDevice->DestroySwapchain( &m_pSwapchain );
                }
                m_desc.pWindow = pWindow;
                return pAPIDevice->CreateSwapchain( m_desc, &m_pSwapchain );
            }

            RESULT Swapchain::Create( const SwapchainDesc& desc, RenderDevice* pDevice )
            {
                BGS_ASSERT( pDevice != nullptr, "Render device (pDevice) must be a valid pointer." );
                m_pParent = pDevice;

                m_desc.format          = desc.format;
                m_desc.backBufferCount = desc.backBufferCount;
                m_desc.format          = desc.format;
                m_desc.vSync           = BGS_FALSE;
                m_desc.pQueue          = m_pParent->GetGraphicsContext()->GetQueue();
                if( BGS_FAILED( Resize( desc.pWindow ) ) )
                {
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void Swapchain::Destroy()
            {
                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                if( m_pSwapchain != nullptr )
                {
                    pAPIDevice->DestroySwapchain( &m_pSwapchain );
                }
            }

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS