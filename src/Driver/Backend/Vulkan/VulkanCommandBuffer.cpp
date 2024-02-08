#define BGS_USE_VULKAN_HANDLES 1

#include "VulkanCommandBuffer.h"

#include "VulkanBindingHeap.h"
#include "VulkanCommon.h"
#include "VulkanDevice.h"
#include "VulkanResource.h"
#include "VulkanResourceView.h"

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

                if( m_pParent->GetDeviceAPI()->vkAllocateCommandBuffers( nativeDevice, &allocateInfo, &nativeCommandBuffer ) != VK_SUCCESS )
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

                    m_pParent->GetDeviceAPI()->vkFreeCommandBuffers( nativeDevice, m_desc.hCommandPool.GetNativeHandle(), 1, &nativeCommandBuffer );
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

                m_pParent->GetDeviceAPI()->vkBeginCommandBuffer( nativeCommandBuffer, &beginInfo );
            }

            void VulkanCommandBuffer::End()
            {
                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                m_pParent->GetDeviceAPI()->vkEndCommandBuffer( nativeCommandBuffer );
            }

            void VulkanCommandBuffer::Reset()
            {
                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                m_pParent->GetDeviceAPI()->vkResetCommandBuffer( nativeCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT );
            }

            void VulkanCommandBuffer::BeginRendering( const BeginRenderingDesc& desc )
            {
                BGS_ASSERT( desc.colorRenderTargetCount <= Config::Driver::Pipeline::MAX_RENDER_TARGET_COUNT,
                            "Render target count (desc.colorRenderTarget) must be less than %d", Config::Driver::Pipeline::MAX_RENDER_TARGET_COUNT );

                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                VkRenderingAttachmentInfo renderTargets[ Config::Driver::Pipeline::MAX_RENDER_TARGET_COUNT ];

                // TODO: Avoid branching
                if( desc.phColorRenderTargetViews != nullptr )
                {
                    for( index_t ndx = 0; ndx < static_cast<index_t>( desc.colorRenderTargetCount ); ++ndx )
                    {
                        VkRenderingAttachmentInfo& currTarget = renderTargets[ ndx ];

                        currTarget.sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
                        currTarget.pNext              = nullptr;
                        currTarget.imageView          = desc.phColorRenderTargetViews[ ndx ].GetNativeHandle()->imageView;
                        currTarget.imageLayout        = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
                        currTarget.resolveMode        = VK_RESOLVE_MODE_NONE;
                        currTarget.resolveImageView   = VK_NULL_HANDLE;
                        currTarget.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                        currTarget.loadOp             = VK_ATTACHMENT_LOAD_OP_LOAD;
                        currTarget.storeOp            = VK_ATTACHMENT_STORE_OP_STORE;
                        currTarget.clearValue         = {}; // Ignored
                    }
                }

                VkRenderingAttachmentInfo  depthStencilTarget;
                VkRenderingAttachmentInfo* pDSV = nullptr;
                // TODO: Avoid branching
                if( desc.hDepthStencilTargetView != ResourceViewHandle() )
                {
                    depthStencilTarget.sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
                    depthStencilTarget.pNext              = nullptr;
                    depthStencilTarget.imageView          = desc.hDepthStencilTargetView.GetNativeHandle()->imageView;
                    depthStencilTarget.imageLayout        = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
                    depthStencilTarget.resolveMode        = VK_RESOLVE_MODE_NONE;
                    depthStencilTarget.resolveImageView   = VK_NULL_HANDLE;
                    depthStencilTarget.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    depthStencilTarget.loadOp             = VK_ATTACHMENT_LOAD_OP_LOAD;
                    depthStencilTarget.storeOp            = VK_ATTACHMENT_STORE_OP_STORE;
                    depthStencilTarget.clearValue         = {}; // Ignored

                    pDSV = &depthStencilTarget;
                }

                VkRenderingInfo renderingInfo;
                renderingInfo.sType                    = VK_STRUCTURE_TYPE_RENDERING_INFO;
                renderingInfo.pNext                    = nullptr;
                renderingInfo.flags                    = 0;
                renderingInfo.renderArea.offset.x      = static_cast<int32_t>( desc.renderArea.offset.x );
                renderingInfo.renderArea.offset.y      = static_cast<int32_t>( desc.renderArea.offset.y );
                renderingInfo.renderArea.extent.width  = desc.renderArea.size.width;
                renderingInfo.renderArea.extent.height = desc.renderArea.size.height;
                renderingInfo.layerCount               = 1;
                renderingInfo.viewMask                 = 0;
                renderingInfo.colorAttachmentCount     = desc.colorRenderTargetCount;
                renderingInfo.pColorAttachments        = renderTargets;
                renderingInfo.pDepthAttachment         = pDSV;
                renderingInfo.pStencilAttachment       = pDSV;

                m_pParent->GetDeviceAPI()->vkCmdBeginRendering( nativeCommandBuffer, &renderingInfo );
            }

            void VulkanCommandBuffer::EndRendering()
            {
                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                m_pParent->GetDeviceAPI()->vkCmdEndRendering( nativeCommandBuffer );
            }

            void VulkanCommandBuffer::ClearBoundColorRenderTarget( uint32_t index, const ColorValue& clearValue, uint32_t rectCount,
                                                                   const Rect2D* pClearRects )
            {
                BGS_ASSERT( rectCount <= Config::Driver::Pipeline::MAX_CLEAR_RECT_COUNT, "Rect count (rectCount) must be less than %d",
                            Config::Driver::Pipeline::MAX_CLEAR_RECT_COUNT );

                VkClearRect rects[ Config::Driver::Pipeline::MAX_CLEAR_RECT_COUNT ];
                for( index_t ndx = 0; ndx < static_cast<index_t>( rectCount ); ++ndx )
                {
                    const Rect2D& currDesc = pClearRects[ ndx ];
                    VkClearRect&  rect     = rects[ ndx ];

                    rect.rect.offset.x      = static_cast<int32_t>( currDesc.offset.x );
                    rect.rect.offset.y      = static_cast<int32_t>( currDesc.offset.y );
                    rect.rect.extent.width  = currDesc.size.width;
                    rect.rect.extent.height = currDesc.size.height;
                    rect.baseArrayLayer     = 0;
                    rect.layerCount         = 1;
                }

                VkClearAttachment target;
                target.aspectMask                    = VK_IMAGE_ASPECT_COLOR_BIT;
                target.clearValue.color.float32[ 0 ] = clearValue.r;
                target.clearValue.color.float32[ 1 ] = clearValue.g;
                target.clearValue.color.float32[ 2 ] = clearValue.b;
                target.clearValue.color.float32[ 3 ] = clearValue.a;
                target.colorAttachment               = index;

                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                m_pParent->GetDeviceAPI()->vkCmdClearAttachments( nativeCommandBuffer, 1, &target, rectCount, rects );
            }

            void VulkanCommandBuffer::ClearBoundDepthStencilTarget( const DepthStencilValue& clearValue, TextureComponentFlags components,
                                                                    uint32_t rectCount, const Rect2D* pClearRects )
            {
                BGS_ASSERT( rectCount <= Config::Driver::Pipeline::MAX_CLEAR_RECT_COUNT, "Rect count (rectCount) must be less than %d",
                            Config::Driver::Pipeline::MAX_CLEAR_RECT_COUNT );
                BGS_ASSERT( !( components & BGS_FLAG( TextureComponentFlagBits::COLOR ) ), "Invalid component flag." )

                VkClearRect rects[ Config::Driver::Pipeline::MAX_CLEAR_RECT_COUNT ];
                for( index_t ndx = 0; ndx < static_cast<index_t>( rectCount ); ++ndx )
                {
                    const Rect2D& currDesc = pClearRects[ ndx ];
                    VkClearRect&  rect     = rects[ ndx ];

                    rect.rect.offset.x      = static_cast<int32_t>( currDesc.offset.x );
                    rect.rect.offset.y      = static_cast<int32_t>( currDesc.offset.y );
                    rect.rect.extent.width  = currDesc.size.width;
                    rect.rect.extent.height = currDesc.size.height;
                    rect.baseArrayLayer     = 0;
                    rect.layerCount         = 1;
                }

                VkClearAttachment target;
                target.aspectMask                      = MapBigosTextureComponentFlagsToVulkanImageAspectFlags( components );
                target.clearValue.depthStencil.depth   = clearValue.depth;
                target.clearValue.depthStencil.stencil = clearValue.stencil;
                target.colorAttachment                 = 0; // Ignored

                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                m_pParent->GetDeviceAPI()->vkCmdClearAttachments( nativeCommandBuffer, 1, &target, rectCount, rects );
            }

            void VulkanCommandBuffer::SetViewports( uint32_t viewportCount, const ViewportDesc* pViewports )
            {
                BGS_ASSERT( viewportCount < Config::Driver::Pipeline::MAX_VIEWPORT_COUNT, "Viewport count (viewportCount) must be less than %d",
                            Config::Driver::Pipeline::MAX_VIEWPORT_COUNT );
                BGS_ASSERT( pViewports != nullptr, "Viewport desc array (pViewport) must be a valid pointer" );

                VkViewport viewports[ Config::Driver::Pipeline::MAX_VIEWPORT_COUNT ];

                for( index_t ndx = 0; ndx < static_cast<index_t>( viewportCount ); ++ndx )
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

                m_pParent->GetDeviceAPI()->vkCmdSetViewportWithCount( nativeCommandBuffer, viewportCount, viewports );
            }

            void VulkanCommandBuffer::SetScissors( uint32_t scissorCount, const ScissorDesc* pScissors )
            {
                BGS_ASSERT( scissorCount < Config::Driver::Pipeline::MAX_SCISSOR_COUNT, "Viewport count (viewportCount) must be less than %d",
                            Config::Driver::Pipeline::MAX_SCISSOR_COUNT );
                BGS_ASSERT( pScissors != nullptr, "Scissor desc array (pScissor) must be a valid pointer" );

                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                m_pParent->GetDeviceAPI()->vkCmdSetScissorWithCount( nativeCommandBuffer, scissorCount,
                                                                     reinterpret_cast<const VkRect2D*>( pScissors ) );
            }

            void VulkanCommandBuffer::SetPrimitiveTopology( PRIMITIVE_TOPOLOGY topology )
            {
                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                m_pParent->GetDeviceAPI()->vkCmdSetPrimitiveTopology( nativeCommandBuffer,
                                                                      MapBigosPrimitiveTopologyToVulkanPrimitiveTopology( topology ) );
            }

            void VulkanCommandBuffer::SetVertexBuffers( uint32_t startBinding, uint32_t bufferCount, const VertexBufferDesc* pVertexBuffers )
            {
                BGS_ASSERT( ( bufferCount != 0 ) && ( pVertexBuffers != nullptr ), "You specified no valid vertex buffers." );
                BGS_ASSERT( bufferCount <= Config::Driver::Pipeline::MAX_INPUT_BINDING_COUNT,
                            "Buffer count (bufferCount) must be less than or equal %d. ", Config::Driver::Pipeline::MAX_INPUT_BINDING_COUNT );
                BGS_ASSERT( startBinding + bufferCount <= Config::Driver::Pipeline::MAX_INPUT_BINDING_COUNT,
                            "Specified binding range (startBinding + bufferCount) exceeds device capabilities of %d.",
                            Config::Driver::Pipeline::MAX_INPUT_BINDING_COUNT );

                VkBuffer     buffers[ Config::Driver::Pipeline::MAX_INPUT_BINDING_COUNT ];
                VkDeviceSize sizes[ Config::Driver::Pipeline::MAX_INPUT_BINDING_COUNT ];
                VkDeviceSize offsets[ Config::Driver::Pipeline::MAX_INPUT_BINDING_COUNT ];
                VkDeviceSize strides[ Config::Driver::Pipeline::MAX_INPUT_BINDING_COUNT ];
                for( index_t ndx = 0; ndx < static_cast<index_t>( bufferCount ); ++ndx )
                {
                    const VertexBufferDesc& currDesc = pVertexBuffers[ ndx ];

                    buffers[ ndx ] = currDesc.hVertexBuffer.GetNativeHandle()->buffer;
                    offsets[ ndx ] = currDesc.offset;
                    sizes[ ndx ]   = currDesc.size;
                    strides[ ndx ] = currDesc.elementStride;
                }

                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                m_pParent->GetDeviceAPI()->vkCmdBindVertexBuffers2( nativeCommandBuffer, startBinding, bufferCount, buffers, offsets, sizes,
                                                                    strides );
            }

            void VulkanCommandBuffer::SetIndexBuffer( const IndexBufferDesc& desc )
            {
                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                m_pParent->GetDeviceAPI()->vkCmdBindIndexBuffer( nativeCommandBuffer, desc.hIndexBuffer.GetNativeHandle()->buffer, desc.offset,
                                                                 MapBigosIndexTypeToVulkanIndexType( desc.indexType ) );
            }

            void VulkanCommandBuffer::Draw( const DrawDesc& desc )
            {
                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                m_pParent->GetDeviceAPI()->vkCmdDraw( nativeCommandBuffer, desc.vertexCount, desc.instanceCount, desc.firstVertex,
                                                      desc.firstInstance );
            }

            void VulkanCommandBuffer::DrawIndexed( const DrawDesc& desc )
            {
                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                m_pParent->GetDeviceAPI()->vkCmdDrawIndexed( nativeCommandBuffer, desc.indexCount, desc.instanceCount, desc.firstIndex,
                                                             desc.vertexOffset, desc.firstInstance );
            }

            void VulkanCommandBuffer::Barrier( const BarierDesc& desc )
            {
                BGS_ASSERT( desc.globalBarrierCount <= Config::Driver::Synchronization::MAX_GLOBAL_BARRIER_COUNT,
                            "Global barrier count (desc.globalBarrierCount) must be less than %d.",
                            Config::Driver::Synchronization::MAX_GLOBAL_BARRIER_COUNT );
                BGS_ASSERT( desc.bufferBarrierCount <= Config::Driver::Synchronization::MAX_BUFFER_BARRIER_COUNT,
                            "Buffer barrier count (desc.bufferBarrierCount) must be less than %d.",
                            Config::Driver::Synchronization::MAX_BUFFER_BARRIER_COUNT );
                BGS_ASSERT( desc.textureBarrierCount <= Config::Driver::Synchronization::MAX_TEXTURE_BARRIER_COUNT,
                            "Texture barrier count (desc.textureBarrierCount) must be less than %d.",
                            Config::Driver::Synchronization::MAX_TEXTURE_BARRIER_COUNT );
                BGS_ASSERT( ( desc.bufferBarrierCount != 0 ) || ( desc.textureBarrierCount != 0 ) || ( desc.globalBarrierCount != 0 ),
                            "No valid barriers." );
                BGS_ASSERT( ( desc.pBufferBarriers != nullptr ) || ( desc.pTextureBarriers != nullptr ) || ( desc.pGlobalBarriers != nullptr ),
                            "No valid barriers." );

                VkMemoryBarrier2       memBarriers[ Config::Driver::Synchronization::MAX_GLOBAL_BARRIER_COUNT ];
                VkBufferMemoryBarrier2 bufferBarriers[ Config::Driver::Synchronization::MAX_BUFFER_BARRIER_COUNT ];
                VkImageMemoryBarrier2  textureBarriers[ Config::Driver::Synchronization::MAX_TEXTURE_BARRIER_COUNT ];
                for( index_t ndx = 0; ndx < static_cast<index_t>( desc.globalBarrierCount ); ++ndx )
                {
                    const GlobalBarrierDesc& currDesc = desc.pGlobalBarriers[ ndx ];
                    VkMemoryBarrier2&        barrier  = memBarriers[ ndx ];

                    barrier.sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
                    barrier.pNext         = nullptr;
                    barrier.srcStageMask  = MapBigosPipelineStageFlagsToVulkanPipelineStageFlags( currDesc.srcStage );
                    barrier.srcAccessMask = MapBigosAccessFlagsToVulkanAccessFlags( currDesc.srcAccess );
                    barrier.dstStageMask  = MapBigosPipelineStageFlagsToVulkanPipelineStageFlags( currDesc.dstStage );
                    barrier.dstAccessMask = MapBigosAccessFlagsToVulkanAccessFlags( currDesc.dstAccess );
                }
                for( index_t ndx = 0; ndx < static_cast<index_t>( desc.bufferBarrierCount ); ++ndx )
                {
                    const BufferBarrierDesc& currDesc = desc.pBufferBarriers[ ndx ];
                    VkBufferMemoryBarrier2&  barrier  = bufferBarriers[ ndx ];

                    barrier.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
                    barrier.pNext               = nullptr;
                    barrier.srcStageMask        = MapBigosPipelineStageFlagsToVulkanPipelineStageFlags( currDesc.srcStage );
                    barrier.srcAccessMask       = MapBigosAccessFlagsToVulkanAccessFlags( currDesc.srcAccess );
                    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.dstStageMask        = MapBigosPipelineStageFlagsToVulkanPipelineStageFlags( currDesc.dstStage );
                    barrier.dstAccessMask       = MapBigosAccessFlagsToVulkanAccessFlags( currDesc.dstAccess );
                    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.buffer              = currDesc.hResouce.GetNativeHandle()->buffer;
                    barrier.offset              = currDesc.bufferRange.offset;
                    barrier.size                = currDesc.bufferRange.size;
                }
                for( index_t ndx = 0; ndx < static_cast<index_t>( desc.textureBarrierCount ); ++ndx )
                {
                    const TextureBarrierDesc& currDesc = desc.pTextureBarriers[ ndx ];
                    VkImageMemoryBarrier2&    barrier  = textureBarriers[ ndx ];

                    barrier.sType                         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
                    barrier.pNext                         = nullptr;
                    barrier.srcStageMask                  = MapBigosPipelineStageFlagsToVulkanPipelineStageFlags( currDesc.srcStage );
                    barrier.srcAccessMask                 = MapBigosAccessFlagsToVulkanAccessFlags( currDesc.srcAccess );
                    barrier.oldLayout                     = MapBigosTextureLayoutToVulkanImageLayout( currDesc.srcLayout );
                    barrier.srcQueueFamilyIndex           = VK_QUEUE_FAMILY_IGNORED;
                    barrier.dstStageMask                  = MapBigosPipelineStageFlagsToVulkanPipelineStageFlags( currDesc.dstStage );
                    barrier.dstAccessMask                 = MapBigosAccessFlagsToVulkanAccessFlags( currDesc.dstAccess );
                    barrier.newLayout                     = MapBigosTextureLayoutToVulkanImageLayout( currDesc.dstLayout );
                    barrier.dstQueueFamilyIndex           = VK_QUEUE_FAMILY_IGNORED;
                    barrier.image                         = currDesc.hResouce.GetNativeHandle()->image;
                    barrier.subresourceRange.aspectMask   = MapBigosTextureComponentFlagsToVulkanImageAspectFlags( currDesc.textureRange.components );
                    barrier.subresourceRange.baseMipLevel = currDesc.textureRange.mipLevel;
                    barrier.subresourceRange.levelCount   = currDesc.textureRange.mipLevelCount;
                    barrier.subresourceRange.baseArrayLayer = currDesc.textureRange.arrayLayer;
                    barrier.subresourceRange.layerCount     = currDesc.textureRange.arrayLayerCount;
                }

                VkDependencyInfo info;
                info.sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
                info.pNext                    = nullptr;
                info.dependencyFlags          = 0;
                info.memoryBarrierCount       = desc.globalBarrierCount;
                info.pMemoryBarriers          = memBarriers;
                info.bufferMemoryBarrierCount = desc.bufferBarrierCount;
                info.pBufferMemoryBarriers    = bufferBarriers;
                info.imageMemoryBarrierCount  = desc.textureBarrierCount;
                info.pImageMemoryBarriers     = textureBarriers;

                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                m_pParent->GetDeviceAPI()->vkCmdPipelineBarrier2( nativeCommandBuffer, &info );
            }

            void VulkanCommandBuffer::CopyBuffer( const CopyBufferDesc& desc )
            {
                BGS_ASSERT( desc.hSrcBuffer != ResourceHandle(), "Buffer (desc.hSrcBuffer) must be a valid handle." );
                BGS_ASSERT( desc.hDstBuffer != ResourceHandle(), "Buffer (desc.hDstBuffer) must be a valid handle." );

                VkBufferCopy buffCpy;
                buffCpy.size      = desc.size;
                buffCpy.dstOffset = desc.dstOffset;
                buffCpy.srcOffset = desc.srcOffset;

                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                m_pParent->GetDeviceAPI()->vkCmdCopyBuffer( nativeCommandBuffer, desc.hSrcBuffer.GetNativeHandle()->buffer,
                                                            desc.hDstBuffer.GetNativeHandle()->buffer, 1, &buffCpy );
            }

            void VulkanCommandBuffer::CopyTexture( const CopyTextureDesc& desc )
            {
                BGS_ASSERT( desc.hSrcTexture != ResourceHandle(), "Texture (desc.hSrcTexture) must be a valid handle." );
                BGS_ASSERT( desc.hDstTexture != ResourceHandle(), "Texture (desc.hDstTexture) must be a valid handle." );

                VkImageCopy texcpy;
                texcpy.srcOffset.x                   = desc.srcOffset.x;
                texcpy.srcOffset.y                   = desc.srcOffset.y;
                texcpy.srcOffset.z                   = desc.srcOffset.z;
                texcpy.dstOffset.x                   = desc.dstOffset.x;
                texcpy.dstOffset.y                   = desc.dstOffset.y;
                texcpy.dstOffset.z                   = desc.dstOffset.z;
                texcpy.extent.width                  = desc.size.width;
                texcpy.extent.height                 = desc.size.height;
                texcpy.extent.depth                  = desc.size.depth;
                texcpy.srcSubresource.aspectMask     = MapBigosTextureComponentFlagsToVulkanImageAspectFlags( desc.srcRange.components );
                texcpy.srcSubresource.mipLevel       = desc.srcRange.mipLevel;
                texcpy.srcSubresource.baseArrayLayer = desc.srcRange.arrayLayer;
                texcpy.srcSubresource.layerCount     = 1; // Mimic D3D12 behavior
                texcpy.dstSubresource.aspectMask     = MapBigosTextureComponentFlagsToVulkanImageAspectFlags( desc.dstRange.components );
                texcpy.dstSubresource.mipLevel       = desc.dstRange.mipLevel;
                texcpy.dstSubresource.baseArrayLayer = desc.dstRange.arrayLayer;
                texcpy.dstSubresource.layerCount     = 1; // Mimic D3D12 behavior

                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                m_pParent->GetDeviceAPI()->vkCmdCopyImage( nativeCommandBuffer, desc.hSrcTexture.GetNativeHandle()->image,
                                                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, desc.hDstTexture.GetNativeHandle()->image,
                                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &texcpy );
            }

            void VulkanCommandBuffer::CopyBuferToTexture( const CopyBufferTextureDesc& desc )
            {
                BGS_ASSERT( desc.hBuffer != ResourceHandle(), "Buffer (desc.hBuffer) must be a valid handle." );
                BGS_ASSERT( desc.hTexture != ResourceHandle(), "Texture (desc.hTexture) must be a valid handle." );
                BGS_ASSERT( desc.bufferOffset % 512 == 0, "Buffer offset (desc.bufferOffset) must be 512 aligned." );

                VkBufferImageCopy buffTexCpy;
                // That means tightly packed
                buffTexCpy.bufferImageHeight               = 0;
                buffTexCpy.bufferRowLength                 = 0;
                buffTexCpy.bufferOffset                    = desc.bufferOffset;
                buffTexCpy.imageOffset.x                   = desc.textureOffset.x;
                buffTexCpy.imageOffset.y                   = desc.textureOffset.y;
                buffTexCpy.imageOffset.z                   = desc.textureOffset.z;
                buffTexCpy.imageExtent.width               = desc.size.width;
                buffTexCpy.imageExtent.height              = desc.size.height;
                buffTexCpy.imageExtent.depth               = desc.size.depth;
                buffTexCpy.imageSubresource.aspectMask     = MapBigosTextureComponentFlagsToVulkanImageAspectFlags( desc.textureRange.components );
                buffTexCpy.imageSubresource.mipLevel       = desc.textureRange.mipLevel;
                buffTexCpy.imageSubresource.baseArrayLayer = desc.textureRange.arrayLayer;
                buffTexCpy.imageSubresource.layerCount     = 1; // Mimic D3D12 behavior

                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                m_pParent->GetDeviceAPI()->vkCmdCopyBufferToImage( nativeCommandBuffer, desc.hBuffer.GetNativeHandle()->buffer,
                                                                   desc.hTexture.GetNativeHandle()->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                                                                   &buffTexCpy );
            }

            void VulkanCommandBuffer::CopyTextureToBuffer( const CopyBufferTextureDesc& desc )
            {
                BGS_ASSERT( desc.hBuffer != ResourceHandle(), "Buffer (desc.hBuffer) must be a valid handle." );
                BGS_ASSERT( desc.hTexture != ResourceHandle(), "Texture (desc.hTexture) must be a valid handle." );
                BGS_ASSERT( desc.bufferOffset % 512 == 0, "Buffer offset (desc.bufferOffset) must be 512 aligned." );

                VkBufferImageCopy buffTexCpy;
                // That means tightly packed
                buffTexCpy.bufferImageHeight               = 0;
                buffTexCpy.bufferRowLength                 = 0;
                buffTexCpy.bufferOffset                    = desc.bufferOffset;
                buffTexCpy.imageOffset.x                   = desc.textureOffset.x;
                buffTexCpy.imageOffset.y                   = desc.textureOffset.y;
                buffTexCpy.imageOffset.z                   = desc.textureOffset.z;
                buffTexCpy.imageExtent.width               = desc.size.width;
                buffTexCpy.imageExtent.height              = desc.size.height;
                buffTexCpy.imageExtent.depth               = desc.size.depth;
                buffTexCpy.imageSubresource.aspectMask     = MapBigosTextureComponentFlagsToVulkanImageAspectFlags( desc.textureRange.components );
                buffTexCpy.imageSubresource.mipLevel       = desc.textureRange.mipLevel;
                buffTexCpy.imageSubresource.baseArrayLayer = desc.textureRange.arrayLayer;
                buffTexCpy.imageSubresource.layerCount     = 1; // Mimic D3D12 behavior

                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                m_pParent->GetDeviceAPI()->vkCmdCopyImageToBuffer( nativeCommandBuffer, desc.hTexture.GetNativeHandle()->image,
                                                                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, desc.hBuffer.GetNativeHandle()->buffer, 1,
                                                                   &buffTexCpy );
            }

            void VulkanCommandBuffer::SetPipeline( PipelineHandle handle, PIPELINE_TYPE type )
            {
                BGS_ASSERT( handle != PipelineHandle(), "Pipeline (handle) must be a valid handle." );

                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                m_pParent->GetDeviceAPI()->vkCmdBindPipeline( nativeCommandBuffer, MapBigosPipelineTypeToVulkanPipelineBindPoint( type ),
                                                              handle.GetNativeHandle() );
            }

            void VulkanCommandBuffer::SetBindingHeaps( uint32_t heapCount, const BindingHeapHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Binding heap (pHandle) must be a valid array." );
                BGS_ASSERT( heapCount <= 2, "Binding heap count (heapCount) must be less or equal 2." );

                VkCommandBuffer                  nativeCommandBuffer = m_handle.GetNativeHandle();
                VkDescriptorBufferBindingInfoEXT heaps[ 2 ];
                for( index_t ndx = 0; ndx < static_cast<index_t>( heapCount ); ++ndx )
                {
                    VulkanBindingHeap*                pDesc    = pHandle[ ndx ].GetNativeHandle();
                    VkDescriptorBufferBindingInfoEXT& currBuff = heaps[ ndx ];

                    currBuff.sType   = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
                    currBuff.pNext   = nullptr;
                    currBuff.address = pDesc->address;
                    currBuff.usage   = pDesc->flags;
                }

                m_pParent->GetDeviceAPI()->vkCmdBindDescriptorBuffersEXT( nativeCommandBuffer, heapCount, heaps );
            }

            void VulkanCommandBuffer::SetBindings( const SetBindingsDesc& desc )
            {
                BGS_ASSERT( desc.hPipelineLayout != PipelineLayoutHandle(), "Pipeline layout (handle) must be a valid handle." );
                BGS_ASSERT( desc.setSpaceNdx < Config::Driver::Pipeline::MAX_BINDING_SET_COUNT,
                            "Set space index (desc.setSpaceNdx) must be less than %d.", Config::Driver::Pipeline::MAX_BINDING_SET_COUNT );
                BGS_ASSERT( desc.heapNdx < 2, "Binding heap index (desc.heapNdx) must be less than 2." );

                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                m_pParent->GetDeviceAPI()->vkCmdSetDescriptorBufferOffsetsEXT(
                    nativeCommandBuffer, MapBigosPipelineTypeToVulkanPipelineBindPoint( desc.type ), desc.hPipelineLayout.GetNativeHandle(),
                    desc.setSpaceNdx, 1, &desc.heapNdx, &desc.baseBindingOffset );
            }

            void VulkanCommandBuffer::BeginQuery( QueryPoolHandle handle, uint32_t queryNdx, QUERY_TYPE type )
            {
                BGS_ASSERT( handle != QueryPoolHandle(), "Query pool (handle) must be a valid handle." );
                BGS_ASSERT( type != QueryTypes::TIMESTAMP, "Query type (type) must not be QueryTypes::TIMESTAMP." );

                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                m_pParent->GetDeviceAPI()->vkCmdBeginQuery( nativeCommandBuffer, handle.GetNativeHandle(), queryNdx, 0 );
            }

            void VulkanCommandBuffer::EndQuery( QueryPoolHandle handle, uint32_t queryNdx, QUERY_TYPE type )
            {
                BGS_ASSERT( handle != QueryPoolHandle(), "Query pool (handle) must be a valid handle." );
                BGS_ASSERT( type != QueryTypes::TIMESTAMP, "Query type (type) must not be QueryTypes::TIMESTAMP." );

                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                m_pParent->GetDeviceAPI()->vkCmdEndQuery( nativeCommandBuffer, handle.GetNativeHandle(), queryNdx );
            }

            void VulkanCommandBuffer::Timestamp( QueryPoolHandle handle, uint32_t queryNdx )
            {
                BGS_ASSERT( handle != QueryPoolHandle(), "Query pool (handle) must be a valid handle." );

                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                // pipelineStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT - that means that timestamp will be written when all previous commands reach
                // bottom of the pipe stage. That is exactly the same as in D3D12.
                m_pParent->GetDeviceAPI()->vkCmdWriteTimestamp( nativeCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, handle.GetNativeHandle(),
                                                                queryNdx );
            }

            void VulkanCommandBuffer::ResetQueryPool( QueryPoolHandle handle, uint32_t firstQuery, uint32_t queryCount )
            {
                BGS_ASSERT( handle != QueryPoolHandle(), "Query pool (handle) must be a valid handle." );

                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                m_pParent->GetDeviceAPI()->vkCmdResetQueryPool( nativeCommandBuffer, handle.GetNativeHandle(), firstQuery, queryCount );
            }

            void VulkanCommandBuffer::CopyQueryResults( const CopyQueryResultsDesc& desc )
            {
                BGS_ASSERT( desc.hQueryPool != QueryPoolHandle(), "Query pool (desc.hQueryPool) must be a valid handle." );
                BGS_ASSERT( desc.hBuffer != ResourceHandle(), "Resource (desc.hBuffer) must be a valid handle." );

                VkCommandBuffer nativeCommandBuffer = m_handle.GetNativeHandle();

                // Hardcoded params:
                //      stride = sizeof( uint64_t )      - To mimic D3D12 behavior (we always use uint64_t as query)
                //      flags  = VK_QUERY_RESULT_64_BIT  - To mimic D3D12 behavior (we always use uint64_t as query)
                m_pParent->GetDeviceAPI()->vkCmdCopyQueryPoolResults( nativeCommandBuffer, desc.hQueryPool.GetNativeHandle(), desc.firstQuery,
                                                                      desc.queryCount, desc.hBuffer.GetNativeHandle()->buffer, desc.bufferOffset,
                                                                      sizeof( uint64_t ), VK_QUERY_RESULT_64_BIT );
            }

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS