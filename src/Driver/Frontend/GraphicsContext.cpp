#include "Driver/Frontend/GraphicsContext.h"

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
                : m_frames()
                , m_frameIndex( 0 )
                , m_pParent( nullptr )
                , m_pQueue( nullptr )
                , m_hFence()
                , m_fenceVal( 0 )
                , m_frameCount( 0 )
                , m_frameStarted( false )
            {
            }

            void GraphicsContext::BeginFrame()
            {
                BGS_ASSERT( !m_frameStarted, "Frame must be ended before starting a new one." );

                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();
                FrameResources&   frame      = m_frames[ m_frameIndex ];

                Backend::WaitForFencesDesc waitDesc;
                waitDesc.fenceCount  = 1;
                waitDesc.waitAll     = BIGOS::Core::BGS_TRUE;
                waitDesc.pFences     = &m_hFence;
                waitDesc.pWaitValues = &frame.fenceVal;
                pAPIDevice->WaitForFences( waitDesc, MAX_UINT64 );
                m_fenceVal++;
                frame.fenceVal = m_fenceVal;

                if( BGS_FAILED( pAPIDevice->ResetCommandPool( frame.hCmdPool ) ) )
                {
                    return;
                }

                // Recording commands
                BIGOS::Driver::Backend::BeginCommandBufferDesc beginDesc;
                frame.pCmdBuffer->Begin( beginDesc );

                m_frameStarted = true;
            }

            void GraphicsContext::SubmitFrame()
            {
                BGS_ASSERT( m_frameStarted, "Frame must be started before submitting." );

                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                FrameResources& frame = m_frames[ m_frameIndex ];
                frame.pCmdBuffer->End();

                Backend::QueueSubmitDesc submitDesc;
                submitDesc.waitSemaphoreCount   = 0;
                submitDesc.phWaitSemaphores     = nullptr;
                submitDesc.waitFenceCount       = 0;
                submitDesc.phWaitFences         = nullptr;
                submitDesc.pWaitValues          = nullptr;
                submitDesc.commandBufferCount   = 1;
                submitDesc.ppCommandBuffers     = &frame.pCmdBuffer;
                submitDesc.signalSemaphoreCount = 0;
                submitDesc.phSignalSemaphores   = nullptr;
                submitDesc.signalFenceCount     = 1;
                submitDesc.phSignalFences       = &m_hFence;
                submitDesc.pSignalValues        = &frame.fenceVal;
                m_pQueue->Submit( submitDesc );

                m_frameIndex = ( m_frameIndex + 1 ) % m_frameCount;
                m_frameStarted = false;
            }

            void GraphicsContext::BeginRenderPass( RenderPass* pRenderPass )
            {
                pRenderPass;
            }

            void GraphicsContext::BindPipeline( Pipeline* pPipeline )
            {
                pPipeline;
            }

            void GraphicsContext::Draw()
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

            RESULT GraphicsContext::Create( uint32_t frameCnt, RenderDevice* pDevice )
            {
                BGS_ASSERT( pDevice != nullptr, "Render device (pDevice) must be a valid pointer." );
                m_pParent                    = pDevice;
                m_frameCount                 = frameCnt;
                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                Backend::QueueDesc qDesc;
                qDesc.type     = Backend::QueueTypes::GRAPHICS;
                qDesc.priority = Backend::QueuePriorityTypes::HIGH;
                if( BGS_FAILED( pAPIDevice->CreateQueue( qDesc, &m_pQueue ) ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                Backend::FenceDesc fenceDesc;
                fenceDesc.initialValue = m_fenceVal;
                if( BGS_FAILED( pAPIDevice->CreateFence( fenceDesc, &m_hFence ) ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                if( BGS_FAILED( CreateFrameResources() ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void GraphicsContext::Destroy()
            {
                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                Backend::WaitForFencesDesc waitDesc;
                waitDesc.fenceCount  = 1;
                waitDesc.waitAll     = BIGOS::Core::BGS_TRUE;
                waitDesc.pFences     = &m_hFence;
                waitDesc.pWaitValues = &m_fenceVal;
                pAPIDevice->WaitForFences( waitDesc, MAX_UINT64 );
                if( m_hFence != Backend::FenceHandle() )
                {
                    pAPIDevice->DestroyFence( &m_hFence );
                }

                DestroyFrameResources();

                if( m_pQueue != nullptr )
                {
                    pAPIDevice->DestroyQueue( &m_pQueue );
                }
            }

            RESULT GraphicsContext::CreateFrameResources()
            {
                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                m_frames.resize( m_frameCount );
                for( index_t ndx = 0; ndx < static_cast<index_t>( m_frameCount ); ++ndx )
                {
                    FrameResources&          frame = m_frames[ ndx ];
                    Backend::CommandPoolDesc cmdPoolDesc;
                    cmdPoolDesc.pQueue = m_pQueue;
                    if( BGS_FAILED( pAPIDevice->CreateCommandPool( cmdPoolDesc, &frame.hCmdPool ) ) )
                    {
                        return Results::FAIL;
                    }
                    Backend::CommandBufferDesc cmdBufferDesc;
                    cmdBufferDesc.hCommandPool = frame.hCmdPool;
                    cmdBufferDesc.level        = Backend::CommandBufferLevels::PRIMARY;
                    cmdBufferDesc.pQueue       = m_pQueue;
                    if( BGS_FAILED( pAPIDevice->CreateCommandBuffer( cmdBufferDesc, &frame.pCmdBuffer ) ) )
                    {
                        return Results::FAIL;
                    }
                }

                return Results::OK;
            }

            void GraphicsContext::DestroyFrameResources()
            {
                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                for( index_t ndx = 0; ndx < static_cast<index_t>( m_frameCount ); ++ndx )
                {
                    FrameResources& frame = m_frames[ ndx ];
                    if( frame.pCmdBuffer != nullptr )
                    {
                        pAPIDevice->DestroyCommandBuffer( &frame.pCmdBuffer );
                    }
                    if( frame.hCmdPool != Backend::CommandPoolHandle() )
                    {
                        pAPIDevice->DestroyCommandPool( &frame.hCmdPool );
                    }
                }
            }

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS