#define BGS_USE_D3D12_HANDLES 1

#include "D3D12Swapchain.h"

#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Memory.h"
#include "D3D12Common.h"
#include "D3D12Device.h"
#include "D3D12Factory.h"
#include "D3D12Resource.h"
#include "Driver/Frontend/RenderSystem.h"
#include "Platform/Window.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            D3D12Swapchain::D3D12Swapchain()
                : m_pParent( nullptr )
                , m_swapInterval( 0 )
            {
            }

            RESULT D3D12Swapchain::Present( const SwapchainPresentDesc& desc )
            {
                BGS_ASSERT( desc.pWaitSemaphores, "Semaphore array (pWaitSemaphores) must be a valid address." );
                BGS_ASSERT( desc.waitSemaphoreCount <= Config::Driver::Synchronization::MAX_SEMAPHORES_TO_WAIT_COUNT );
                if( ( desc.pWaitSemaphores == nullptr ) ||
                    ( desc.waitSemaphoreCount > Config::Driver::Synchronization::MAX_SEMAPHORES_TO_WAIT_COUNT ) )
                {
                    return Results::FAIL;
                }
                IDXGISwapChain3*    pSwapchain = m_handle.GetNativeHandle();
                ID3D12CommandQueue* pQueue     = m_desc.pQueue->GetHandle().GetNativeHandle();
                ID3D12Fence*        pFence     = nullptr;
                // Waiting for semaphores and reseting them after
                for( index_t ndx = 0; ndx < static_cast<index_t>( desc.waitSemaphoreCount ); ++ndx )
                {
                    pFence = desc.pWaitSemaphores[ ndx ].GetNativeHandle();
                    if( FAILED( pQueue->Wait( pFence, static_cast<uint64_t>( D3D12SemaphoreStates::SIGNALED ) ) ) )
                    {
                        return Results::FAIL;
                    }
                    if( FAILED( pQueue->Signal( pFence, static_cast<uint64_t>( D3D12SemaphoreStates::NOT_SIGNALED ) ) ) )
                    {
                        return Results::FAIL;
                    }
                }

                // Setting present semaphore to not ready, presenting and setting to ready
                pFence = m_backBuffers[ desc.frameNdx ].hBackBufferAvailableSemaphore.GetNativeHandle();
                if( FAILED( pQueue->Signal( pFence, static_cast<uint64_t>( D3D12SemaphoreStates::NOT_SIGNALED ) ) ) )
                {
                    return Results::FAIL;
                }
                if( FAILED( pSwapchain->Present( m_swapInterval, 0 ) ) )
                {
                    return Results::FAIL;
                }
                if( FAILED( pQueue->Signal( pFence, static_cast<uint64_t>( D3D12SemaphoreStates::SIGNALED ) ) ) )
                {
                    return Results::FAIL;
                }
                return Results::OK;
            }

            RESULT D3D12Swapchain::GetNextFrame( FrameInfo* pInfo )
            {
                BGS_ASSERT( pInfo != nullptr, "Frame info (pInfo) must be a valid address." );

                IDXGISwapChain3* pSwapchain          = m_handle.GetNativeHandle();
                const uint32_t   ndx                 = pSwapchain->GetCurrentBackBufferIndex();
                pInfo->swapChainBackBufferIndex      = ndx;
                pInfo->hBackBufferAvailableSemaphore = m_backBuffers[ ndx ].hBackBufferAvailableSemaphore;

                return Results::OK;
            }

            RESULT D3D12Swapchain::Create( const SwapchainDesc& desc, D3D12Device* pDevice )
            {
                BGS_ASSERT( pDevice != nullptr, "Device (pDevice) must be a valid pointer." );
                BGS_ASSERT( desc.pWindow != nullptr, "Window (desc.pWindow) must be a valid pointer." );
                BGS_ASSERT( desc.pQueue != nullptr, "Queue (desc.pQueue) must be a valid pointer." );
                BGS_ASSERT( desc.pQueue->GetDesc().type == QueueTypes::GRAPHICS, "Queue (desc.pQueue) must be a graphics queue." );
                // TODO: Better validation
                if( ( desc.pWindow == nullptr ) || ( desc.pQueue == nullptr ) || ( desc.pQueue->GetDesc().type != QueueTypes::GRAPHICS ) ||
                    ( pDevice == nullptr ) )
                {
                    return Results::FAIL;
                }

                m_desc         = desc;
                m_pParent      = pDevice;
                m_swapInterval = desc.vSync;

                if( BGS_FAILED( CreateD3D12Swapchain() ) )
                {
                    return Results::FAIL;
                }

                if( BGS_FAILED( GetD3D12BackBuffers() ) )
                {
                    IDXGISwapChain3* pNativeSwapchain = m_handle.GetNativeHandle();
                    RELEASE_COM_PTR( pNativeSwapchain );
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void D3D12Swapchain::Destroy()
            {
                if( m_handle != SwapchainHandle() )
                {
                    IDXGISwapChain3* pNativeSwapchain = m_handle.GetNativeHandle();
                    RELEASE_COM_PTR( pNativeSwapchain );

                    for( index_t ndx = 0; ndx < m_backBuffers.size(); ++ndx )
                    {
                        m_pParent->DestroySemaphore( &m_backBuffers[ ndx ].hBackBufferAvailableSemaphore );
                        RELEASE_COM_PTR( m_backBuffers[ ndx ].hBackBuffer.GetNativeHandle()->pNativeResource );
                    }
                    D3D12Resource* pRes = m_backBuffers[ 0 ].hBackBuffer.GetNativeHandle();
                    Core::Memory::FreeAligned( m_pParent->GetParent()->GetParent()->GetDefaultAllocator(), &pRes );
                }

                m_pParent = nullptr;
                m_handle  = SwapchainHandle();
            }

            RESULT D3D12Swapchain::CreateD3D12Swapchain()
            {
                DXGI_SWAP_CHAIN_DESC1 scDesc;
                scDesc.BufferCount        = m_desc.backBufferCount;
                scDesc.Width              = m_desc.pWindow->GetDesc().width;
                scDesc.Height             = m_desc.pWindow->GetDesc().height;
                scDesc.Format             = MapBigosFormatToD3D12Format( m_desc.format ); // TODO: Ms docs read and validate
                scDesc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                scDesc.Flags              = 0;
                scDesc.Stereo             = FALSE;
                scDesc.SampleDesc.Count   = 1;
                scDesc.SampleDesc.Quality = 0;
                scDesc.Scaling            = DXGI_SCALING_STRETCH;
                scDesc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
                scDesc.AlphaMode          = DXGI_ALPHA_MODE_UNSPECIFIED; // TODO: Check if that is correct
                scDesc.Flags              = m_desc.vSync ? 0 : DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

                IDXGIFactory4*   pNativeFactory   = m_pParent->GetParent()->GetHandle().GetNativeHandle();
                IDXGISwapChain1* pNativeSwapChain = nullptr;
                if( FAILED( pNativeFactory->CreateSwapChainForHwnd( m_desc.pQueue->GetHandle().GetNativeHandle(), m_desc.pWindow->GetInternals().hWnd,
                                                                    &scDesc, nullptr, nullptr, &pNativeSwapChain ) ) )
                {
                    return Results::FAIL;
                }

                IDXGISwapChain3* pNativeSwapChain3 = nullptr;
                if( FAILED( pNativeSwapChain->QueryInterface( &pNativeSwapChain3 ) ) )
                {
                    pNativeSwapChain->Release();
                    return Results::FAIL;
                }
                pNativeSwapChain->Release();

                if( m_desc.pWindow->GetDesc().mode == Platform::WindowModes::FULLSCREEN )
                {
                    if( FAILED( pNativeSwapChain3->SetFullscreenState( TRUE, NULL ) ) )
                    {
                        pNativeSwapChain3->Release();
                        return Results::FAIL;
                    }
                }

                m_handle = SwapchainHandle( pNativeSwapChain3 );

                return Results::OK;
            }

            RESULT D3D12Swapchain::GetD3D12BackBuffers()
            {
                SemaphoreDesc    sd;
                IDXGISwapChain3* pNativeSwapchain = m_handle.GetNativeHandle();
                m_backBuffers.resize( m_desc.backBufferCount );

                D3D12Resource*      pBackBuffers = nullptr;
                ID3D12Fence*        pFence       = nullptr;
                ID3D12CommandQueue* pQueue       = m_desc.pQueue->GetHandle().GetNativeHandle();

                if( BGS_FAILED( Core::Memory::AllocateArray( m_pParent->GetParent()->GetParent()->GetDefaultAllocator(), &pBackBuffers,
                                                             m_desc.backBufferCount, alignof( D3D12Resource ) ) ) )
                {
                    return Results::FAIL;
                }

                for( index_t ndx = 0; ndx < static_cast<index_t>( m_desc.backBufferCount ); ++ndx )
                {
                    if( FAILED( pNativeSwapchain->GetBuffer( static_cast<uint32_t>( ndx ), IID_PPV_ARGS( &pBackBuffers[ ndx ].pNativeResource ) ) ) )
                    {
                        Core::Memory::FreeAligned( &pBackBuffers );
                        return Results::FAIL;
                    }

                    // Creating semaphores holding current status of back buffer presentation
                    if( BGS_FAILED( m_pParent->CreateSemaphore( sd, &m_backBuffers[ ndx ].hBackBufferAvailableSemaphore ) ) )
                    {
                        for( index_t ndy = 0; ndy < ndx; ++ndy )
                        {
                            m_pParent->DestroySemaphore( &m_backBuffers[ ndy ].hBackBufferAvailableSemaphore );
                        }
                        Core::Memory::FreeAligned( &pBackBuffers );
                        return Results::FAIL;
                    }
                    // At the start, all of images are ready to draw
                    pFence = m_backBuffers[ ndx ].hBackBufferAvailableSemaphore.GetNativeHandle();
                    if( FAILED( pQueue->Signal( pFence, static_cast<uint64_t>( D3D12SemaphoreStates::SIGNALED ) ) ) )
                    {
                        for( index_t ndy = 0; ndy < ndx; ++ndy )
                        {
                            m_pParent->DestroySemaphore( &m_backBuffers[ ndy ].hBackBufferAvailableSemaphore );
                        }
                        Core::Memory::FreeAligned( m_pParent->GetParent()->GetParent()->GetDefaultAllocator(), &pBackBuffers );
                        return Results::FAIL;
                    }

                    pBackBuffers[ ndx ].desc         = pBackBuffers[ ndx ].pNativeResource->GetDesc();
                    m_backBuffers[ ndx ].hBackBuffer = ResourceHandle( &pBackBuffers[ ndx ] );
                }

                return Results::OK;
            }

        } // namespace Backend
    } // namespace Driver
} // namespace BIGOS