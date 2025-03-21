#include "Driver/Frontend/Contexts.h"

#include "Driver/Backend/API.h"
#include "Driver/Frontend/RenderDevice.h"
#include "Driver/Frontend/Swapchain.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            GraphicsContext::GraphicsContext()
                : m_pParent( nullptr )
                , m_pQueue( nullptr )
                , m_hFence()
                , m_fenceVal( 0 )
            {
            }

            RESULT GraphicsContext::Present( Swapchain* pSwapchain, RenderTarget* pRenderTarget )
            {
                BGS_ASSERT( pSwapchain != nullptr, "Swapchain (pSwapchain) must be a valid pointer." );
                BGS_ASSERT( pRenderTarget != nullptr, "Render target (pRenderTarget) must be a valid pointer." );

                // TODO: Validation
                //       - check if render target and back buffer has same size
                return pSwapchain->Present( pRenderTarget );
            }

            RESULT GraphicsContext::Create( RenderDevice* pDevice )
            {
                BGS_ASSERT( pDevice != nullptr, "Render device (pDevice) must be a valid pointer." );
                m_pParent                    = pDevice;
                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                Backend::QueueDesc qDesc;
                qDesc.type     = Backend::QueueTypes::GRAPHICS;
                qDesc.priority = Backend::QueuePriorityTypes::HIGH;
                if( BGS_FAILED( pAPIDevice->CreateQueue( qDesc, &m_pQueue ) ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void GraphicsContext::Destroy()
            {
                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                if( m_pQueue != nullptr )
                {
                    pAPIDevice->DestroyQueue( &m_pQueue );
                }
            }

            ComputeContext::ComputeContext()
                : m_pParent( nullptr )
                , m_pQueue( nullptr )
                , m_hCmdPool()
                , m_pCmdBuffer( nullptr )
            {
            }

            RESULT ComputeContext::Create( RenderDevice* pDevice )
            {
                BGS_ASSERT( pDevice != nullptr, "Render device (pDevice) must be a valid pointer." );
                m_pParent                    = pDevice;
                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                Backend::QueueDesc qDesc;
                qDesc.type     = Backend::QueueTypes::COMPUTE;
                qDesc.priority = Backend::QueuePriorityTypes::HIGH;
                if( BGS_FAILED( pAPIDevice->CreateQueue( qDesc, &m_pQueue ) ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                Backend::CommandPoolDesc poolDesc;
                poolDesc.pQueue = m_pQueue;
                if( BGS_FAILED( pAPIDevice->CreateCommandPool( poolDesc, &m_hCmdPool ) ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                Backend::CommandBufferDesc buffDesc;
                buffDesc.hCommandPool = m_hCmdPool;
                buffDesc.level        = Backend::CommandBufferLevels::PRIMARY;
                buffDesc.pQueue       = m_pQueue;
                if( BGS_FAILED( pAPIDevice->CreateCommandBuffer( buffDesc, &m_pCmdBuffer ) ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void ComputeContext::Destroy()
            {
                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                if( m_pCmdBuffer != nullptr )
                {
                    pAPIDevice->DestroyCommandBuffer( &m_pCmdBuffer );
                }
                if( m_hCmdPool != Backend::CommandPoolHandle() )
                {
                    pAPIDevice->DestroyCommandPool( &m_hCmdPool );
                }
                if( m_pQueue != nullptr )
                {
                    pAPIDevice->DestroyQueue( &m_pQueue );
                }
            }

            CopyContext::CopyContext()
                : m_pParent( nullptr )
                , m_pQueue( nullptr )
                , m_hCmdPool()
                , m_pCmdBuffer( nullptr )
            {
            }

            RESULT CopyContext::Create( RenderDevice* pDevice )
            {
                BGS_ASSERT( pDevice != nullptr, "Render device (pDevice) must be a valid pointer." );
                m_pParent                    = pDevice;
                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                Backend::QueueDesc qDesc;
                qDesc.type     = Backend::QueueTypes::COMPUTE;
                qDesc.priority = Backend::QueuePriorityTypes::HIGH;
                if( BGS_FAILED( pAPIDevice->CreateQueue( qDesc, &m_pQueue ) ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                Backend::CommandPoolDesc poolDesc;
                poolDesc.pQueue = m_pQueue;
                if( BGS_FAILED( pAPIDevice->CreateCommandPool( poolDesc, &m_hCmdPool ) ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                Backend::CommandBufferDesc buffDesc;
                buffDesc.hCommandPool = m_hCmdPool;
                buffDesc.level        = Backend::CommandBufferLevels::PRIMARY;
                buffDesc.pQueue       = m_pQueue;
                if( BGS_FAILED( pAPIDevice->CreateCommandBuffer( buffDesc, &m_pCmdBuffer ) ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void CopyContext::Destroy()
            {
                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                if( m_pCmdBuffer != nullptr )
                {
                    pAPIDevice->DestroyCommandBuffer( &m_pCmdBuffer );
                }
                if( m_hCmdPool != Backend::CommandPoolHandle() )
                {
                    pAPIDevice->DestroyCommandPool( &m_hCmdPool );
                }
                if( m_pQueue != nullptr )
                {
                    pAPIDevice->DestroyQueue( &m_pQueue );
                }
            }

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS