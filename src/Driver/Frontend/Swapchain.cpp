#include "Driver/Frontend/Swapchain.h"

#include "Driver/Frontend/Contexts.h"
#include "Driver/Frontend/RenderDevice.h"
#include "Driver/Frontend/RenderTarget.h"

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
                , m_hCpySemaphores()
                , m_hCmdPools()
                , m_pCmdBuffers()
                , m_frameNdx( 0 )
            {
            }

            RESULT Swapchain::Resize( Platform::Window* pWindow )
            {
                BGS_ASSERT( pWindow != nullptr, "Window (pWindow) must be a valid pointer." );

                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                if( m_pSwapchain != nullptr )
                {
                    Backend::WaitForFencesDesc waitDesc;
                    waitDesc.fenceCount  = 1;
                    waitDesc.waitAll     = BIGOS::Core::BGS_TRUE;
                    waitDesc.pFences     = &m_hFence;
                    waitDesc.pWaitValues = &m_fenceVal;
                    pAPIDevice->WaitForFences( waitDesc, MAX_UINT64 );
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

                // Create present primitives
                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();
                m_hCpySemaphores.resize( m_desc.backBufferCount );
                m_hCmdPools.resize( m_desc.backBufferCount );
                m_pCmdBuffers.resize( m_desc.backBufferCount );
                for( index_t ndx = 0; ndx < static_cast<index_t>( m_desc.backBufferCount ); ++ndx )
                {
                    Backend::SemaphoreDesc semDesc;
                    if( BGS_FAILED( pAPIDevice->CreateSemaphore( semDesc, &m_hCpySemaphores[ ndx ] ) ) )
                    {
                        Destroy();
                        return Results::FAIL;
                    }
                    Backend::CommandPoolDesc poolDesc;
                    poolDesc.pQueue = m_desc.pQueue;
                    if( BGS_FAILED( pAPIDevice->CreateCommandPool( poolDesc, &m_hCmdPools[ ndx ] ) ) )
                    {
                        Destroy();
                        return Results::FAIL;
                    }
                    Backend::CommandBufferDesc buffDesc;
                    buffDesc.hCommandPool = m_hCmdPools[ ndx ];
                    buffDesc.pQueue       = m_desc.pQueue;
                    buffDesc.level        = Backend::CommandBufferLevels::PRIMARY;
                    if( BGS_FAILED( pAPIDevice->CreateCommandBuffer( buffDesc, &m_pCmdBuffers[ ndx ] ) ) )
                    {
                        Destroy();
                        return Results::FAIL;
                    }
                }

                Backend::FenceDesc fenceDesc;
                fenceDesc.initialValue = m_fenceVal;
                if( BGS_FAILED( pAPIDevice->CreateFence( fenceDesc, &m_hFence ) ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void Swapchain::Destroy()
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

                for( index_t ndx = 0; ndx < static_cast<index_t>( m_desc.backBufferCount ); ++ndx )
                {
                    if( m_pCmdBuffers[ ndx ] != nullptr )
                    {
                        pAPIDevice->DestroyCommandBuffer( &m_pCmdBuffers[ ndx ] );
                    }
                    if( m_hCmdPools[ ndx ] != Backend::CommandPoolHandle() )
                    {
                        pAPIDevice->DestroyCommandPool( &m_hCmdPools[ ndx ] );
                    }
                    if( m_hCpySemaphores[ ndx ] != Backend::SemaphoreHandle() )
                    {
                        pAPIDevice->DestroySemaphore( &m_hCpySemaphores[ ndx ] );
                    }
                }

                if( m_pSwapchain != nullptr )
                {
                    pAPIDevice->DestroySwapchain( &m_pSwapchain );
                }
            }

            RESULT Swapchain::Present( RenderTarget* pRenderTarget )
            {
                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                Backend::WaitForFencesDesc waitDesc;
                waitDesc.fenceCount  = 1;
                waitDesc.waitAll     = BIGOS::Core::BGS_FALSE;
                waitDesc.pFences     = &m_hFence;
                waitDesc.pWaitValues = &m_fenceVal;
                if( BGS_FAILED( pAPIDevice->WaitForFences( waitDesc, UINT64_MAX ) ) )
                {
                    return Results::FAIL;
                }
                m_fenceVal++;

                const bool                  useResolve = pRenderTarget->IsMultisampled();
                Backend::PipelineStageFlags cpyStage =
                    useResolve ? BGS_FLAG( Backend::PipelineStageFlagBits::RESOLVE ) : BGS_FLAG( Backend::PipelineStageFlagBits::TRANSFER );
                Backend::AccessFlags cpyBBAccess =
                    useResolve ? BGS_FLAG( Backend::AccessFlagBits::RESOLVE_DST ) : BGS_FLAG( Backend::AccessFlagBits::TRANSFER_DST );
                Backend::TEXTURE_LAYOUT cpyBBLayout = useResolve ? Backend::TextureLayouts::RESOLVE_DST : Backend::TextureLayouts::TRANSFER_DST;
                Backend::AccessFlags    cpyRTAccess =
                    useResolve ? BGS_FLAG( Backend::AccessFlagBits::RESOLVE_SRC ) : BGS_FLAG( Backend::AccessFlagBits::TRANSFER_SRC );
                Backend::TEXTURE_LAYOUT cpyRTLayout = useResolve ? Backend::TextureLayouts::RESOLVE_SRC : Backend::TextureLayouts::TRANSFER_SRC;

                Backend::FrameInfo frameInfo;
                m_pSwapchain->GetNextFrame( &frameInfo );
                const uint32_t bufferNdx = frameInfo.swapChainBackBufferIndex;

                // Copy back buffer barrier
                Backend::TextureBarrierDesc cpyBBBarrier;
                cpyBBBarrier.srcStage     = BGS_FLAG( Backend::PipelineStageFlagBits::RENDER_TARGET );
                cpyBBBarrier.srcAccess    = BGS_FLAG( Backend::AccessFlagBits::GENERAL );
                cpyBBBarrier.srcLayout    = Backend::TextureLayouts::PRESENT;
                cpyBBBarrier.dstStage     = cpyStage;
                cpyBBBarrier.dstAccess    = cpyBBAccess;
                cpyBBBarrier.dstLayout    = cpyBBLayout;
                cpyBBBarrier.hResouce     = m_pSwapchain->GetBackBuffers()[ bufferNdx ].hBackBuffer;
                cpyBBBarrier.textureRange = { BGS_FLAG( Backend::TextureComponentFlagBits::COLOR ), 0, 1, 0, 1 };

                // Present back buffer barrier
                Backend::TextureBarrierDesc presBBBarrier;
                presBBBarrier.srcStage     = cpyStage;
                presBBBarrier.srcAccess    = cpyBBAccess;
                presBBBarrier.srcLayout    = cpyBBLayout;
                presBBBarrier.dstStage     = BGS_FLAG( Backend::PipelineStageFlagBits::RENDER_TARGET );
                presBBBarrier.dstAccess    = BGS_FLAG( Backend::AccessFlagBits::GENERAL );
                presBBBarrier.dstLayout    = Backend::TextureLayouts::PRESENT;
                presBBBarrier.hResouce     = m_pSwapchain->GetBackBuffers()[ bufferNdx ].hBackBuffer;
                presBBBarrier.textureRange = { BGS_FLAG( Backend::TextureComponentFlagBits::COLOR ), 0, 1, 0, 1 };

                if( BGS_FAILED( pAPIDevice->ResetCommandPool( m_hCmdPools[ bufferNdx ] ) ) )
                {
                    return Results::FAIL;
                }

                // Recording commands
                BIGOS::Driver::Backend::BeginCommandBufferDesc beginDesc;
                m_pCmdBuffers[ bufferNdx ]->Begin( beginDesc );

                m_pCmdBuffers[ bufferNdx ]->Barrier( { &cpyBBBarrier, nullptr, nullptr, 1, 0, 0 } );

                ResourceState newRTState( cpyStage, cpyRTAccess, cpyRTLayout );
                if( pRenderTarget->GetState() != newRTState )
                {
                    // Copy render target barrier
                    Backend::TextureBarrierDesc cpyRTBarrier;
                    cpyRTBarrier.srcStage     = pRenderTarget->GetState().GetStage();
                    cpyRTBarrier.srcAccess    = pRenderTarget->GetState().GetAccess();
                    cpyRTBarrier.srcLayout    = pRenderTarget->GetState().GetLayout();
                    cpyRTBarrier.dstStage     = cpyStage;
                    cpyRTBarrier.dstAccess    = cpyRTAccess;
                    cpyRTBarrier.dstLayout    = cpyRTLayout;
                    cpyRTBarrier.hResouce     = pRenderTarget->GetResource();
                    cpyRTBarrier.textureRange = { BGS_FLAG( Backend::TextureComponentFlagBits::COLOR ), 0, 1, 0, 1 };
                    m_pCmdBuffers[ bufferNdx ]->Barrier( { &cpyRTBarrier, nullptr, nullptr, 1, 0, 0 } );
                    pRenderTarget->SetState( newRTState );
                }

                if( useResolve )
                {
                    Backend::ResolveDesc resolveDesc;
                    resolveDesc.hSrcTexture = pRenderTarget->GetResource();
                    resolveDesc.srcRange    = { BGS_FLAG( Backend::TextureComponentFlagBits::COLOR ), 0, 1, 0, 1 };
                    resolveDesc.hDstTexture = m_pSwapchain->GetBackBuffers()[ bufferNdx ].hBackBuffer;
                    resolveDesc.dstRange    = { BGS_FLAG( Backend::TextureComponentFlagBits::COLOR ), 0, 1, 0, 1 };
                    m_pCmdBuffers[ bufferNdx ]->Resolve( resolveDesc );
                }
                else
                {
                    Backend::CopyTextureDesc cpyDesc;
                    cpyDesc.hSrcTexture = pRenderTarget->GetResource();
                    cpyDesc.srcRange    = { BGS_FLAG( Backend::TextureComponentFlagBits::COLOR ), 0, 1, 0, 1 };
                    cpyDesc.srcOffset   = { 0, 0, 0 };
                    cpyDesc.hDstTexture = m_pSwapchain->GetBackBuffers()[ bufferNdx ].hBackBuffer;
                    cpyDesc.dstRange    = { BGS_FLAG( Backend::TextureComponentFlagBits::COLOR ), 0, 1, 0, 1 };
                    cpyDesc.dstOffset   = { 0, 0, 0 };
                    cpyDesc.size        = { pRenderTarget->GetWidth(), pRenderTarget->GetHeight(), 1 };
                    m_pCmdBuffers[ bufferNdx ]->CopyTexture( cpyDesc );
                }

                m_pCmdBuffers[ bufferNdx ]->Barrier( { &presBBBarrier, nullptr, nullptr, 1, 0, 0 } );

                m_pCmdBuffers[ bufferNdx ]->End();

                // Execute commands
                Backend::QueueSubmitDesc submitDesc;
                submitDesc.waitSemaphoreCount   = 1;
                submitDesc.phWaitSemaphores     = &frameInfo.hBackBufferAvailableSemaphore;
                submitDesc.waitFenceCount       = 0;
                submitDesc.phWaitFences         = nullptr;
                submitDesc.pWaitValues          = nullptr;
                submitDesc.commandBufferCount   = 1;
                submitDesc.ppCommandBuffers     = &m_pCmdBuffers[ bufferNdx ];
                submitDesc.signalSemaphoreCount = 1;
                submitDesc.phSignalSemaphores   = &m_hCpySemaphores[ bufferNdx ];
                submitDesc.signalFenceCount     = 1;
                submitDesc.phSignalFences       = &m_hFence;
                submitDesc.pSignalValues        = &m_fenceVal;
                if( BGS_FAILED( m_desc.pQueue->Submit( submitDesc ) ) )
                {
                    return Results::FAIL;
                }

                // Presents frame
                Backend::SwapchainPresentDesc presentDesc;
                presentDesc.waitSemaphoreCount = 1;
                presentDesc.pWaitSemaphores    = &m_hCpySemaphores[ bufferNdx ];
                presentDesc.frameNdx           = bufferNdx;
                if( BGS_FAILED( m_pSwapchain->Present( presentDesc ) ) )
                {
                    return Results::FAIL;
                }

                m_frameNdx = ( m_frameNdx + 1 ) % m_desc.backBufferCount;

                return Results::OK;
            }

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS