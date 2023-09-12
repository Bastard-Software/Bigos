#define BGS_USE_VULKAN_HANDLES 1

#include "VulkanCommandBuffer.h"

#include "VulkanCommon.h"
#include "VulkanDevice.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            RESULT VulkanCommandBuffer::Create( const CommandBufferDesc& desc, VulkanDevice* pDevice )
            {
                BGS_ASSERT( pDevice != nullptr, "Device (pDevice) must be a valid pointer." );
                BGS_ASSERT( desc.pQueue != nullptr, "Queue (desc.pQueue) must be a valid pointer." );
                if( ( pDevice == nullptr ) || ( desc.pQueue == nullptr ) )
                {
                    return Results::FAIL;
                }

                m_pParent = pDevice;
                m_desc    = desc;

                VkCommandBufferAllocateInfo allocateInfo;
                allocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocateInfo.pNext              = nullptr;
                allocateInfo.commandPool        = m_desc.hCommandPool.GetNativeHandle();
                allocateInfo.commandBufferCount = 1; // To mimic D3D12 behaviour
                allocateInfo.level              = MapBigosCommandBufferLevelToVulkanCommandBufferLevel( m_desc.level );

                VkDevice        nativeDevice        = m_pParent->GetHandle().GetNativeHandle();
                VkCommandBuffer nativeCommandBuffer = VK_NULL_HANDLE;

                if( vkAllocateCommandBuffers( nativeDevice, &allocateInfo, &nativeCommandBuffer ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }

                m_handle = CommandBufferHandle( nativeCommandBuffer );

                return Results::OK;
            }

            void VulkanCommandBuffer::Destroy()
            {
                BGS_ASSERT( m_pParent != nullptr );

                if( ( m_handle != CommandBufferHandle() ) )
                {
                    VkDevice        nativeDevice        = m_pParent->GetHandle().GetNativeHandle();
                    VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                    vkFreeCommandBuffers( nativeDevice, m_desc.hCommandPool.GetNativeHandle(), 1, &nativeCommandBuffer );
                }

                m_pParent = nullptr;
                m_handle  = CommandBufferHandle();
            }

            void VulkanCommandBuffer::Begin( const BeginCommandBufferDesc& desc )
            {
                VkCommandBufferBeginInfo beginInfo;
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.pNext = nullptr;
                beginInfo.flags =
                    VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // TODO: Is that flag enough to mimic D3D12 behaviour? Or should that be handled?
                beginInfo.pInheritanceInfo = nullptr;            // TODO: Handle. Needed for secondary command buffers
                desc;

                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                vkBeginCommandBuffer( nativeCommandBuffer, &beginInfo );
            }

            void VulkanCommandBuffer::End()
            {
                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                vkEndCommandBuffer( nativeCommandBuffer );
            }

            void VulkanCommandBuffer::Reset()
            {
                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                vkResetCommandBuffer( nativeCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT );
            }

            void VulkanCommandBuffer::BeginRendering( const BeginRenderingDesc& desc )
            {
                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();
                // TODO: Handle
                VkRenderingInfo renderingInfo;
                desc;

                vkCmdBeginRendering( nativeCommandBuffer, &renderingInfo );
            }

            void VulkanCommandBuffer::EndRendering()
            {
                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                vkCmdEndRendering( nativeCommandBuffer );
            }

            void VulkanCommandBuffer::SetViewports( uint32_t viewportCount, const ViewportDesc* pViewports )
            {
                BGS_ASSERT( viewportCount < Config::Driver::Pipeline::MAX_VIEWPORT_COUNT, "Viewport count (viewportCount) must be less than %d",
                            Config::Driver::Pipeline::MAX_VIEWPORT_COUNT );
                BGS_ASSERT( pViewports != nullptr, "Viewport desc array (pViewport) must be a valid pointer" );

                VkViewport viewports[ Config::Driver::Pipeline::MAX_VIEWPORT_COUNT ];

                for( index_t ndx = 0; ndx < static_cast<uint32_t>( viewportCount ); ++ndx )
                {
                    const ViewportDesc& currDesc = pViewports[ ndx ];
                    VkViewport&         currVP   = viewports[ ndx ];

                    // Note that we need to revert viewport: y = height, height = -height
                    currVP.x        = currDesc.upperLeftX;
                    currVP.y        = currDesc.height;
                    currVP.width    = currDesc.width;
                    currVP.height   = -currDesc.height;
                    currVP.minDepth = currDesc.minDepth;
                    currVP.maxDepth = currDesc.maxDepth;
                }

                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                vkCmdSetViewport( nativeCommandBuffer, 0, viewportCount, viewports );
            }

            void VulkanCommandBuffer::SetScissors( uint32_t scissorCount, const ScissorDesc* pScissors )
            {
                BGS_ASSERT( scissorCount < Config::Driver::Pipeline::MAX_SCISSOR_COUNT, "Viewport count (viewportCount) must be less than %d",
                            Config::Driver::Pipeline::MAX_SCISSOR_COUNT );
                BGS_ASSERT( pScissors != nullptr, "Scissor desc array (pScissor) must be a valid pointer" );

                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                vkCmdSetScissor( nativeCommandBuffer, 0, scissorCount, reinterpret_cast<const VkRect2D*>( pScissors ) );
            }

            void VulkanCommandBuffer::SetPrimitiveTopology( PRIMITIVE_TOPOLOGY topology )
            {
                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                vkCmdSetPrimitiveTopology( nativeCommandBuffer, MapBigosPrimitiveTopologyToVulkanPrimitiveTopology( topology ) );
            }

            void VulkanCommandBuffer::Draw( const DrawDesc& desc )
            {
                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                vkCmdDraw( nativeCommandBuffer, desc.vertexCount, desc.instanceCount, desc.firstVertex, desc.firstInstance );
            }

            void VulkanCommandBuffer::DrawIndexed( const DrawDesc& desc )
            {
                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                vkCmdDrawIndexed( nativeCommandBuffer, desc.indexCount, desc.instanceCount, desc.firstIndex, desc.vertexOffset, desc.firstInstance );
            }

            void VulkanCommandBuffer::Barrier( uint32_t barrierCount, const BarierDesc* pBarriers )
            {
                // TODO: Not trivial
                barrierCount;
                pBarriers;
            }

            void VulkanCommandBuffer::SetPipeline( PipelineHandle handle, PIPELINE_TYPE type )
            {
                // TODO: Need pipelines
                handle;
                type;
            }

            void VulkanCommandBuffer::SetBindingHeaps( BindingHeapHandle hShaderResourceHeap, BindingHeapHandle hSamplerHeap )
            {
                hShaderResourceHeap;
                hSamplerHeap;
            }

            void VulkanCommandBuffer::SetBinding( const SetBindingDesc& desc )
            {
                desc;
            }

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS