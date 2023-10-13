#define BGS_USE_VULKAN_HANDLES 1

#include "VulkanSwapchain.h"

#include "Driver/Backend/APITypes.h"

#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Memory.h"
#include "Driver/Frontend/RenderSystem.h"
#include "Platform/Window.h"
#include "VulkanCommon.h"
#include "VulkanDevice.h"
#include "VulkanFactory.h"
#include "VulkanResource.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {

            VulkanSwapchain::VulkanSwapchain()
                : m_pParent( nullptr )
                , m_surface( nullptr )
                , m_bufferToPresent( 0 )
                , m_semaphoreNdx( 0 )
            {
            }

            RESULT VulkanSwapchain::Resize( const SwapchainResizeDesc& desc )
            {
                desc;
                return Results::OK;
            }

            RESULT VulkanSwapchain::Present( const SwapchainPresentDesc& desc )
            {
                BGS_ASSERT( desc.pWaitSemaphores, "Semaphore array (pWaitSemaphores) must be a valid address." );
                BGS_ASSERT( desc.waitSemaphoreCount <= Config::Driver::Synchronization::MAX_SEMAPHORES_TO_WAIT_COUNT );
                if( ( desc.pWaitSemaphores == nullptr ) ||
                    ( desc.waitSemaphoreCount > Config::Driver::Synchronization::MAX_SEMAPHORES_TO_WAIT_COUNT ) )
                {
                    return Results::FAIL;
                }
                VkSwapchainKHR nativeSwapchain = m_handle.GetNativeHandle();
                VkQueue        nativeQueue     = m_desc.pQueue->GetHandle().GetNativeHandle();
                VkSemaphore    semaphores[ Config::Driver::Synchronization::MAX_SEMAPHORES_TO_WAIT_COUNT ];
                for( index_t ndx = 0; ndx < static_cast<index_t>( desc.waitSemaphoreCount ); ++ndx )
                {
                    semaphores[ ndx ] = desc.pWaitSemaphores[ ndx ].GetNativeHandle();
                }

                VkPresentInfoKHR presInfo;
                presInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
                presInfo.pNext              = nullptr;
                presInfo.waitSemaphoreCount = desc.waitSemaphoreCount;
                presInfo.pWaitSemaphores    = semaphores;
                presInfo.swapchainCount     = 1;
                presInfo.pSwapchains        = &nativeSwapchain;
                presInfo.pImageIndices      = &m_bufferToPresent;
                presInfo.pResults           = nullptr;
                if( vkQueuePresentKHR( nativeQueue, &presInfo ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }
                m_bufferToPresent = ( m_bufferToPresent + 1 ) % m_desc.backBufferCount;

                return Results::OK;
            }

            RESULT VulkanSwapchain::GetNextFrame( FrameInfo* pInfo )
            {
                BGS_ASSERT( pInfo != nullptr, "Frame info (pInfo) must be a valid address." );

                VkDevice       nativeDevice    = m_pParent->GetHandle().GetNativeHandle();
                VkSwapchainKHR nativeSwapchain = m_handle.GetNativeHandle();
                VkSemaphore    nativeSemaphore = m_backBuffers[ m_semaphoreNdx ].hBackBufferAvailableSemaphore.GetNativeHandle();
                uint32_t       bufferNdx;
                // Timeout 0 ns mimics D3D12 behaviour, additionaly we do not support vulkan fences here.
                // TODO: Timeout 0 doesn't work.
                VkResult res = vkAcquireNextImageKHR( nativeDevice, nativeSwapchain, UINT64_MAX, nativeSemaphore, VK_NULL_HANDLE, &bufferNdx );

                pInfo->hBackBufferAvailableSemaphore = m_backBuffers[ m_semaphoreNdx ].hBackBufferAvailableSemaphore;
                pInfo->swapChainBackBufferIndex      = m_semaphoreNdx;

                BGS_ASSERT( res != VK_TIMEOUT );        // Temporary for debug
                BGS_ASSERT( res != VK_SUBOPTIMAL_KHR ); // Temporary for debug
                if( res == VK_SUCCESS )
                {
                    // Adjusting semaphore index after succesfull acquireing
                    m_semaphoreNdx = ( bufferNdx + 1 ) % m_desc.backBufferCount;

                    return Results::OK;
                }
                else if( res == VK_NOT_READY )
                {
                    // Adjusting semaphore index after succesfull acquireing
                    m_semaphoreNdx = ( bufferNdx + 1 ) % m_desc.backBufferCount;

                    return Results::NOT_READY;
                }

                return Results::FAIL;
            }

            RESULT VulkanSwapchain::Create( const SwapchainDesc& desc, VulkanDevice* pDevice )
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

                m_desc    = desc;
                m_pParent = pDevice;

                if( BGS_FAILED( CreateVkSwapchain() ) )
                {
                    return Results::FAIL;
                }
                if( BGS_FAILED( GetBackBuffers() ) )
                {
                    VkDevice       nativeDevice    = m_pParent->GetHandle().GetNativeHandle();
                    VkSwapchainKHR nativeSwapchain = m_handle.GetNativeHandle();
                    VkInstance     nativeFactory   = m_pParent->GetParent()->GetHandle().GetNativeHandle();

                    vkDestroySwapchainKHR( nativeDevice, nativeSwapchain, nullptr );
                    vkDestroySurfaceKHR( nativeFactory, m_surface, nullptr );

                    return Results::FAIL;
                }

                return Results::OK;
            }

            void VulkanSwapchain::Destroy()
            {
                if( m_handle != SwapchainHandle() )
                {
                    VkDevice       nativeDevice    = m_pParent->GetHandle().GetNativeHandle();
                    VkSwapchainKHR nativeSwapchain = m_handle.GetNativeHandle();

                    vkDestroySwapchainKHR( nativeDevice, nativeSwapchain, nullptr );

                    if( m_surface != VK_NULL_HANDLE )
                    {
                        VkInstance nativeFactory = m_pParent->GetParent()->GetHandle().GetNativeHandle();
                        vkDestroySurfaceKHR( nativeFactory, m_surface, nullptr );
                    }

                    for( index_t ndx = 0; ndx < m_backBuffers.size(); ++ndx )
                    {
                        m_pParent->DestroySemaphore( &m_backBuffers[ ndx ].hBackBufferAvailableSemaphore );
                    }
                    VulkanResource* pRes = m_backBuffers[ 0 ].hBackBuffer.GetNativeHandle();
                    Core::Memory::FreeAligned( m_pParent->GetParent()->GetParent()->GetDefaultAllocator(), &pRes );
                }

                m_pParent = nullptr;
                m_handle  = SwapchainHandle();
            }

            RESULT VulkanSwapchain::CreateVkSwapchain()
            {
                // Creating surface
#if( BGS_WINDOWS )
                VkWin32SurfaceCreateInfoKHR surfInfo;
                surfInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
                surfInfo.pNext     = nullptr;
                surfInfo.flags     = 0;
                surfInfo.hwnd      = m_desc.pWindow->GetInternals().hWnd;
                surfInfo.hinstance = m_desc.pWindow->GetInternals().hModule;

                VkInstance nativeFactory = m_pParent->GetParent()->GetHandle().GetNativeHandle();
                if( vkCreateWin32SurfaceKHR( nativeFactory, &surfInfo, nullptr, &m_surface ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }
#else
#    error
#endif // ( BGS_WINDOWS )

                VkSwapchainCreateInfoKHR scInfo;
                scInfo.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
                scInfo.pNext                 = nullptr;
                scInfo.flags                 = 0;
                scInfo.surface               = m_surface;
                scInfo.minImageCount         = m_desc.backBufferCount;
                scInfo.imageFormat           = MapBigosFormatToVulkanFormat( m_desc.format );
                scInfo.imageColorSpace       = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
                scInfo.imageExtent.width     = m_desc.pWindow->GetDesc().width;
                scInfo.imageExtent.height    = m_desc.pWindow->GetDesc().height;
                scInfo.imageArrayLayers      = 1;
                scInfo.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                scInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
                scInfo.queueFamilyIndexCount = 1;
                scInfo.pQueueFamilyIndices   = nullptr;
                scInfo.preTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
                scInfo.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
                scInfo.presentMode           = m_desc.vSync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
                scInfo.clipped               = VK_TRUE;
                scInfo.oldSwapchain          = VK_NULL_HANDLE;

                VkDevice       nativeDevice    = m_pParent->GetHandle().GetNativeHandle();
                VkSwapchainKHR nativeSwapchain = VK_NULL_HANDLE;
                if( vkCreateSwapchainKHR( nativeDevice, &scInfo, nullptr, &nativeSwapchain ) != VK_SUCCESS )
                {
                    vkDestroySurfaceKHR( nativeFactory, m_surface, nullptr );
                    return Results::FAIL;
                }

                m_handle = SwapchainHandle( nativeSwapchain );

                return Results::OK;
            }

            RESULT VulkanSwapchain::GetBackBuffers()
            {
                VkDevice       nativeDevice    = m_pParent->GetHandle().GetNativeHandle();
                VkSwapchainKHR nativeSwapchain = m_handle.GetNativeHandle();
                VkImage        images[ Config::Driver::Swapchain::MAX_BACK_BUFFER_COUNT ];
                m_backBuffers.resize( m_desc.backBufferCount );
                if( vkGetSwapchainImagesKHR( nativeDevice, nativeSwapchain, &m_desc.backBufferCount, images ) )
                {
                    return Results::FAIL;
                }

                SemaphoreDesc   desc;
                VulkanResource* pResArr = nullptr;
                if( BGS_FAILED( Core::Memory::AllocateArray( m_pParent->GetParent()->GetParent()->GetDefaultAllocator(), &pResArr,
                                                             m_desc.backBufferCount, alignof( VulkanResource ) ) ) )
                {
                    return Results::NO_MEMORY;
                }

                for( index_t ndx = 0; ndx < static_cast<index_t>( m_desc.backBufferCount ); ++ndx )
                {
                    VulkanResource& currRes = pResArr[ ndx ];

                    currRes.type                     = VulkanResourceTypes::IMAGE;
                    currRes.image                    = images[ ndx ];
                    currRes.pMemory                  = nullptr;
                    currRes.usage                    = static_cast<ResourceUsageFlags>( ResourceUsageFlagBits::COLOR_RENDER_TARGET );
                    currRes.memoryOffset             = INVALID_OFFSET;
                    m_backBuffers[ ndx ].hBackBuffer = ResourceHandle( &currRes );
                    if( BGS_FAILED( m_pParent->CreateSemaphore( desc, &m_backBuffers[ ndx ].hBackBufferAvailableSemaphore ) ) )
                    {
                        for( index_t ndy = 0; ndy < ndx; ++ndy )
                        {
                            m_pParent->DestroySemaphore( &m_backBuffers[ ndy ].hBackBufferAvailableSemaphore );
                        }
                        Core::Memory::FreeAligned( m_pParent->GetParent()->GetParent()->GetDefaultAllocator(), &pResArr );
                    }
                }

                return Results::OK;
            }

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS