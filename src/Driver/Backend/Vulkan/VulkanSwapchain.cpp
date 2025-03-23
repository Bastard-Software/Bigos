#define BGS_USE_VULKAN_HANDLES 1

#include "VulkanSwapchain.h"

#include "Driver/Backend/APITypes.h"

#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Memory.h"
#include "Driver/Frontend/RenderSystem.h"
#include "Platform/Window/Window.h"
#include "VulkanCommon.h"
#include "VulkanDevice.h"
#include "VulkanFactory.h"
#include "VulkanQueue.h"
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
                , m_semaphoreNdx( 0 )
            {
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
                presInfo.pImageIndices      = &desc.frameNdx;
                presInfo.pResults           = nullptr;
                if( m_pParent->GetDeviceAPI()->vkQueuePresentKHR( nativeQueue, &presInfo ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }

                return Results::OK;
            }

            RESULT VulkanSwapchain::GetNextFrame( FrameInfo* pInfo )
            {
                BGS_ASSERT( pInfo != nullptr, "Frame info (pInfo) must be a valid address." );

                VkDevice       nativeDevice    = m_pParent->GetHandle().GetNativeHandle();
                VkSwapchainKHR nativeSwapchain = m_handle.GetNativeHandle();
                VkSemaphore    nativeSemaphore = m_backBuffers[ m_semaphoreNdx ].hBackBufferAvailableSemaphore.GetNativeHandle();
                uint32_t       bufferNdx;
                // Timeout 1 ns mimics D3D12 behaviour, additionaly we do not support vulkan fences here.
                VkResult res =
                    m_pParent->GetDeviceAPI()->vkAcquireNextImageKHR( nativeDevice, nativeSwapchain, 1, nativeSemaphore, VK_NULL_HANDLE, &bufferNdx );

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

                    m_pParent->GetDeviceAPI()->vkDestroySwapchainKHR( nativeDevice, nativeSwapchain, nullptr );
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

                    m_pParent->GetDeviceAPI()->vkDestroySwapchainKHR( nativeDevice, nativeSwapchain, nullptr );

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

                const uint32_t queueFamilyNdx = static_cast<VulkanQueue*>( m_desc.pQueue )->GetFamilyIndex();

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
                scInfo.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
                scInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
                scInfo.queueFamilyIndexCount = 1;
                scInfo.pQueueFamilyIndices   = &queueFamilyNdx;
                scInfo.preTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
                scInfo.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
                scInfo.presentMode           = m_desc.vSync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
                scInfo.clipped               = VK_TRUE;
                scInfo.oldSwapchain          = VK_NULL_HANDLE;

                VkDevice       nativeDevice    = m_pParent->GetHandle().GetNativeHandle();
                VkSwapchainKHR nativeSwapchain = VK_NULL_HANDLE;
                if( m_pParent->GetDeviceAPI()->vkCreateSwapchainKHR( nativeDevice, &scInfo, nullptr, &nativeSwapchain ) != VK_SUCCESS )
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
                if( m_pParent->GetDeviceAPI()->vkGetSwapchainImagesKHR( nativeDevice, nativeSwapchain, &m_desc.backBufferCount, images ) )
                {
                    return Results::FAIL;
                }

                if( m_backBuffers.size() == 0 )
                {
                    // Transition from undefined to general layout
                    VulkanQueue* pVulkanQueue = static_cast<VulkanQueue*>( m_desc.pQueue );

                    VkFence           cmdFence = VK_NULL_HANDLE;
                    VkFenceCreateInfo fenceInfo;
                    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                    fenceInfo.pNext = nullptr;
                    fenceInfo.flags = 0;
                    if( m_pParent->GetDeviceAPI()->vkCreateFence( nativeDevice, &fenceInfo, nullptr, &cmdFence ) )
                    {
                        return Results::FAIL;
                    }

                    VkCommandPool           cmdPool = VK_NULL_HANDLE;
                    VkCommandPoolCreateInfo poolInfo;
                    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                    poolInfo.pNext            = nullptr;
                    poolInfo.flags            = 0;
                    poolInfo.queueFamilyIndex = pVulkanQueue->GetFamilyIndex();
                    if( m_pParent->GetDeviceAPI()->vkCreateCommandPool( nativeDevice, &poolInfo, nullptr, &cmdPool ) )
                    {
                        m_pParent->GetDeviceAPI()->vkDestroyFence( nativeDevice, cmdFence, nullptr );
                        return Results::FAIL;
                    }

                    VkCommandBuffer             cmdBuffer = VK_NULL_HANDLE;
                    VkCommandBufferAllocateInfo allocInfo;
                    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                    allocInfo.pNext              = nullptr;
                    allocInfo.commandPool        = cmdPool;
                    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                    allocInfo.commandBufferCount = 1;
                    if( m_pParent->GetDeviceAPI()->vkAllocateCommandBuffers( nativeDevice, &allocInfo, &cmdBuffer ) )
                    {
                        m_pParent->GetDeviceAPI()->vkDestroyCommandPool( nativeDevice, cmdPool, nullptr );
                        m_pParent->GetDeviceAPI()->vkDestroyFence( nativeDevice, cmdFence, nullptr );
                        return Results::FAIL;
                    }

                    VkCommandBufferBeginInfo beginInfo;
                    beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                    beginInfo.pNext            = nullptr;
                    beginInfo.flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                    beginInfo.pInheritanceInfo = nullptr;
                    m_pParent->GetDeviceAPI()->vkBeginCommandBuffer( cmdBuffer, &beginInfo );
                    for( index_t ndx = 0; ndx < m_desc.backBufferCount; ++ndx )
                    {
                        VkImageMemoryBarrier imgBarrier;
                        imgBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                        imgBarrier.pNext               = nullptr;
                        imgBarrier.srcAccessMask       = VK_ACCESS_NONE;
                        imgBarrier.dstAccessMask       = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
                        imgBarrier.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
                        imgBarrier.newLayout           = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                        imgBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        imgBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                        imgBarrier.image               = images[ ndx ];
                        imgBarrier.subresourceRange    = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

                        m_pParent->GetDeviceAPI()->vkCmdPipelineBarrier( cmdBuffer, VK_PIPELINE_STAGE_NONE,
                                                                         VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr,
                                                                         1, &imgBarrier );
                    }
                    m_pParent->GetDeviceAPI()->vkEndCommandBuffer( cmdBuffer );

                    VkSubmitInfo submitInfo;
                    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                    submitInfo.pNext                = nullptr;
                    submitInfo.waitSemaphoreCount   = 0;
                    submitInfo.pWaitSemaphores      = nullptr;
                    submitInfo.pWaitDstStageMask    = nullptr;
                    submitInfo.commandBufferCount   = 1;
                    submitInfo.pCommandBuffers      = &cmdBuffer;
                    submitInfo.signalSemaphoreCount = 0;
                    submitInfo.pSignalSemaphores    = nullptr;

                    if( m_pParent->GetDeviceAPI()->vkQueueSubmit( pVulkanQueue->GetHandle().GetNativeHandle(), 1, &submitInfo, cmdFence ) )
                    {
                        m_pParent->GetDeviceAPI()->vkFreeCommandBuffers( nativeDevice, cmdPool, 1, &cmdBuffer );
                        m_pParent->GetDeviceAPI()->vkDestroyCommandPool( nativeDevice, cmdPool, nullptr );
                        m_pParent->GetDeviceAPI()->vkDestroyFence( nativeDevice, cmdFence, nullptr );
                        return Results::FAIL;
                    }

                    if( m_pParent->GetDeviceAPI()->vkWaitForFences( nativeDevice, 1, &cmdFence, VK_TRUE, UINT64_MAX ) )
                    {
                        m_pParent->GetDeviceAPI()->vkFreeCommandBuffers( nativeDevice, cmdPool, 1, &cmdBuffer );
                        m_pParent->GetDeviceAPI()->vkDestroyCommandPool( nativeDevice, cmdPool, nullptr );
                        m_pParent->GetDeviceAPI()->vkDestroyFence( nativeDevice, cmdFence, nullptr );
                        return Results::FAIL;
                    }

                    m_pParent->GetDeviceAPI()->vkFreeCommandBuffers( nativeDevice, cmdPool, 1, &cmdBuffer );
                    m_pParent->GetDeviceAPI()->vkDestroyCommandPool( nativeDevice, cmdPool, nullptr );
                    m_pParent->GetDeviceAPI()->vkDestroyFence( nativeDevice, cmdFence, nullptr );
                }

                m_backBuffers.resize( m_desc.backBufferCount );

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
    } // namespace Driver
} // namespace BIGOS