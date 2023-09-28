#define BGS_USE_VULKAN_HANDLES 1

#include "VulkanDevice.h"

#include "BigosFramework/Config.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Memory.h"
#include "Core/Utils/String.h"
#include "Driver/Frontend/RenderSystem.h"
#include "VulkanBindingHeap.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommon.h"
#include "VulkanFactory.h"
#include "VulkanMemory.h"
#include "VulkanQueue.h"
#include "VulkanResource.h"
#include "VulkanResourceView.h"
#include "VulkanSwapchain.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            class VkDeviceFeaturesHelper final
            {
            public:
                VkDeviceFeaturesHelper( const DeviceDesc& desc )
                    : m_devDesc( desc )
                    , m_devCoreFeatures()
                    , m_dev11Features()
                    , m_dev12Features()
                    , m_dev13Features()
                    , m_customBorderColorFeatures()
                    , m_descriptorBufferFeatures()
                    , m_extensions()
                    , m_pNext( nullptr )
                {
                    m_devDesc = desc;

                    // Vulkan 1.0 features (core)
                    m_devCoreFeatures.pipelineStatisticsQuery = VK_TRUE;
                    m_devCoreFeatures.samplerAnisotropy       = VK_TRUE;
                    m_devCoreFeatures.multiDrawIndirect       = VK_TRUE;

                    // Vulkan 1.1 features
                    m_dev11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
                    m_dev11Features.pNext = &m_dev12Features;

                    // Vulkan 1.2 features
                    m_dev12Features.sType                    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
                    m_dev12Features.pNext                    = &m_dev13Features;
                    m_dev12Features.drawIndirectCount        = VK_TRUE;
                    m_dev12Features.samplerMirrorClampToEdge = VK_TRUE;
                    m_dev12Features.timelineSemaphore        = VK_TRUE; // Crucial for synchronization
                    m_dev12Features.bufferDeviceAddress      = VK_TRUE; // Needed for descriptor buffer ext

                    // Vulkan 1.3 features
                    m_dev13Features.sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
                    m_dev13Features.pNext            = &m_customBorderColorFeatures;
                    m_dev13Features.synchronization2 = VK_TRUE; // Crucial for synchronization
                    m_dev13Features.dynamicRendering = VK_TRUE; // Crucial for rendering

                    // Custom border color features
                    m_customBorderColorFeatures.sType                          = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT;
                    m_customBorderColorFeatures.pNext                          = &m_descriptorBufferFeatures;
                    m_customBorderColorFeatures.customBorderColors             = VK_TRUE;
                    m_customBorderColorFeatures.customBorderColorWithoutFormat = VK_TRUE;

                    // Descriptor buffers
                    m_descriptorBufferFeatures.sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT;
                    m_descriptorBufferFeatures.pNext            = nullptr;
                    m_descriptorBufferFeatures.descriptorBuffer = VK_TRUE;

                    m_pNext = &m_dev11Features;

                    // Extensions
                    m_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME,
                                     VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME };
                    // TODO: Add logic
                }

                VkDeviceFeaturesHelper() = default;

                void*                     GetExtChain() const { return m_pNext; }
                VkPhysicalDeviceFeatures* GetDeviceFeatures() { return &m_devCoreFeatures; }
                const char* const*        GetExtNames() const { return m_extensions.data(); }
                uint32_t                  GetExtCount() const { return static_cast<uint32_t>( m_extensions.size() ); }

            private:
                DeviceDesc m_devDesc;

                VkPhysicalDeviceFeatures                     m_devCoreFeatures;
                VkPhysicalDeviceVulkan11Features             m_dev11Features;
                VkPhysicalDeviceVulkan12Features             m_dev12Features;
                VkPhysicalDeviceVulkan13Features             m_dev13Features;
                VkPhysicalDeviceCustomBorderColorFeaturesEXT m_customBorderColorFeatures;
                VkPhysicalDeviceDescriptorBufferFeaturesEXT  m_descriptorBufferFeatures;

                HeapArray<const char*> m_extensions;

                void* m_pNext;
            };

            static VkPipelineShaderStageCreateInfo CreateShaderStage( ShaderStageDesc shaderStageDesc, VkShaderStageFlagBits stage )
            {
                VkPipelineShaderStageCreateInfo shaderStageInfo;
                shaderStageInfo.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shaderStageInfo.flags               = 0;
                shaderStageInfo.pNext               = nullptr;
                shaderStageInfo.stage               = stage;
                shaderStageInfo.module              = shaderStageDesc.hShader.GetNativeHandle();
                shaderStageInfo.pName               = shaderStageDesc.pEntryPoint;
                shaderStageInfo.pSpecializationInfo = nullptr;

                return shaderStageInfo;
            }

            static uint32_t FindVulkanMemTypeNdx( const VkPhysicalDeviceMemoryProperties* pMemProps, uint32_t memBits,
                                                  VkMemoryPropertyFlags memProps )
            {

                for( index_t ndx = 0; ndx < static_cast<index_t>( pMemProps->memoryTypeCount ); ++ndx )
                {
                    const uint32_t              memTypeBits       = ( 1 << ndx );
                    const bool                  isRequiredMemType = memBits & memTypeBits;
                    const VkMemoryPropertyFlags props             = pMemProps->memoryTypes[ ndx ].propertyFlags;
                    const bool                  hasRequiredProps  = ( props & memProps ) == memProps;
                    if( isRequiredMemType && hasRequiredProps )
                    {
                        return static_cast<uint32_t>( ndx );
                    }
                }
                return MAX_UINT32;
            }

            static MemoryAccessFlags MapBigosMemoryHeapTypeToMemoryAccessFlags( MEMORY_HEAP_TYPE type )
            {
                static const MemoryAccessFlags translateTable[ BGS_ENUM_COUNT( MemoryHeapTypes ) ] = {
                    static_cast<uint32_t>( MemoryAccessFlagBits::GPU_DEVICE_ACCESS ), // DEFAULT
                    static_cast<uint32_t>( MemoryAccessFlagBits::CPU_WRITES_TO_DEVICE ) |
                        static_cast<uint32_t>( MemoryAccessFlagBits::GPU_READS_FROM_DEVICE ), // UPLOAD
                    static_cast<uint32_t>( MemoryAccessFlagBits::CPU_READS_FROM_DEVICE ),     // READBACK
                    0,                                                                        // CUSTOM
                };
                return translateTable[ BGS_ENUM_INDEX( type ) ];
            }

            RESULT VulkanDevice::CreateQueue( const QueueDesc& desc, IQueue** ppQueue )
            {
                BGS_ASSERT( ppQueue != nullptr, "Queue (ppQueue) must be a valid address." );
                BGS_ASSERT( *ppQueue == nullptr, "There is a pointer at the given address. Queue (*ppQueue) must be nullptr." );
                if( ( ppQueue == nullptr ) && ( *ppQueue != nullptr ) )
                {
                    return Results::FAIL;
                }

                VulkanQueue* pQueue = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( m_pParent->GetParent()->GetDefaultAllocator(), &pQueue ) ) )
                {
                    return Results::NO_MEMORY;
                }
                BGS_ASSERT( pQueue != nullptr );

                if( BGS_FAILED( pQueue->Create( desc, this ) ) )
                {
                    Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), &pQueue );
                    return Results::FAIL;
                }

                ( *ppQueue ) = pQueue;

                return Results::OK;
            }

            void VulkanDevice::DestroyQueue( IQueue** ppQueue )
            {
                BGS_ASSERT( ppQueue != nullptr, "Queue (ppQueue) must be a valid address." );
                BGS_ASSERT( *ppQueue != nullptr, "Queue (*ppQueue) must be a valid pointer." );
                if( ( ppQueue != nullptr ) && ( *ppQueue != nullptr ) )
                {
                    static_cast<VulkanQueue*>( *ppQueue )->Destroy();
                    Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), ppQueue );
                }
            }

            RESULT VulkanDevice::CreateSwapchain( const SwapchainDesc& desc, ISwapchain** ppSwapchain )
            {
                BGS_ASSERT( ppSwapchain != nullptr, "Swapchain (ppQueue) must be a valid address." );
                BGS_ASSERT( *ppSwapchain == nullptr, "There is a pointer at the given address. Swapchain (*ppSwapchain) must be nullptr." );
                BGS_ASSERT( desc.pWindow != nullptr, "Window (desc.pWindow) must be a valid pointer." );
                BGS_ASSERT( desc.pQueue != nullptr, "Queue (desc.pQueue) must be a valid pointer." );
                BGS_ASSERT( desc.pQueue->GetDesc().type == QueueTypes::GRAPHICS, "Queue (desc.pQueue) must be a graphics queue." );
                // TODO: Better validation
                if( ( ppSwapchain == nullptr ) || ( *ppSwapchain != nullptr ) || ( desc.pWindow == nullptr ) || ( desc.pQueue == nullptr ) ||
                    ( desc.pQueue->GetDesc().type != QueueTypes::GRAPHICS ) )
                {
                    return Results::FAIL;
                }

                VulkanSwapchain* pSwapchain = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( m_pParent->GetParent()->GetDefaultAllocator(), &pSwapchain ) ) )
                {
                    return Results::NO_MEMORY;
                }
                BGS_ASSERT( pSwapchain != nullptr );

                if( BGS_FAILED( pSwapchain->Create( desc, this ) ) )
                {
                    Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), &pSwapchain );
                    return Results::FAIL;
                }

                *ppSwapchain = pSwapchain;

                return Results::OK;
            }

            void VulkanDevice::DestroySwapchain( ISwapchain** ppSwapchain )
            {
                BGS_ASSERT( ppSwapchain != nullptr, "Swapchain (ppSwapchain) must be a valid address." );
                BGS_ASSERT( *ppSwapchain != nullptr, "Swapchain (*ppSwapchain) must be a valid pointer." );
                if( ( ppSwapchain != nullptr ) && ( *ppSwapchain != nullptr ) )
                {
                    static_cast<VulkanSwapchain*>( *ppSwapchain )->Destroy();
                    Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), ppSwapchain );
                }
            }

            RESULT VulkanDevice::CreateCommandPool( const CommandPoolDesc& desc, CommandPoolHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Command pool (pHandle) must be a valid address." );
                BGS_ASSERT( desc.pQueue != nullptr, "Queue (desc.pQueue) must be a valid pointer." );
                if( desc.pQueue == nullptr )
                {
                    return Results::FAIL;
                }

                VkCommandPoolCreateInfo poolInfo;
                poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                poolInfo.pNext            = nullptr;
                poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // To mimic D3D12 behaviour
                poolInfo.queueFamilyIndex = static_cast<VulkanQueue*>( desc.pQueue )->GetFamilyIndex();

                VkDevice      nativeDevice      = m_handle.GetNativeHandle();
                VkCommandPool nativeCommandPool = VK_NULL_HANDLE;

                if( vkCreateCommandPool( nativeDevice, &poolInfo, nullptr, &nativeCommandPool ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }

                *pHandle = CommandPoolHandle( nativeCommandPool );

                return Results::OK;
            }

            void VulkanDevice::DestroyCommandPool( CommandPoolHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Command pool handle (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != CommandPoolHandle(), "Command pool handle (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != CommandPoolHandle() ) )
                {
                    VkDevice      nativeDevice      = m_handle.GetNativeHandle();
                    VkCommandPool nativeCommandPool = pHandle->GetNativeHandle();

                    vkDestroyCommandPool( nativeDevice, nativeCommandPool, nullptr );

                    *pHandle = CommandPoolHandle();
                }
            }

            RESULT VulkanDevice::ResetCommandPool( CommandPoolHandle handle )
            {
                BGS_ASSERT( handle != CommandPoolHandle(), "Command pool handle (handle) must be a valid handle." );
                if( handle == CommandPoolHandle() )
                {
                    return Results::FAIL;
                }

                VkDevice      nativeDevice      = m_handle.GetNativeHandle();
                VkCommandPool nativeCommandPool = handle.GetNativeHandle();
                if( vkResetCommandPool( nativeDevice, nativeCommandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }

                return Results::OK;
            }

            RESULT VulkanDevice::CreateCommandBuffer( const CommandBufferDesc& desc, ICommandBuffer** ppCommandBuffer )
            {
                BGS_ASSERT( ppCommandBuffer != nullptr, "Command buffer (ppCommandBuffer) must be a valid address." );
                BGS_ASSERT( *ppCommandBuffer == nullptr,
                            "There is a pointer at the given address. Command buffer (*ppCommandBuffer) must be nullptr." );
                if( ( ppCommandBuffer == nullptr ) && ( *ppCommandBuffer != nullptr ) )
                {
                    return Results::FAIL;
                }

                VulkanCommandBuffer* pCommandBuffer = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( m_pParent->GetParent()->GetDefaultAllocator(), &pCommandBuffer ) ) )
                {
                    return Results::NO_MEMORY;
                }
                BGS_ASSERT( pCommandBuffer != nullptr );

                if( BGS_FAILED( pCommandBuffer->Create( desc, this ) ) )
                {
                    Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), &pCommandBuffer );
                    return Results::FAIL;
                }

                *ppCommandBuffer = pCommandBuffer;

                return Results::OK;
            }

            void VulkanDevice::DestroyCommandBuffer( ICommandBuffer** ppCommandBuffer )
            {
                BGS_ASSERT( ppCommandBuffer != nullptr, "Command buffer (ppCommandBuffer) must be a valid address." );
                BGS_ASSERT( *ppCommandBuffer != nullptr, "Command buffer (*ppCommandBuffer) must be a valid pointer." );
                if( ( ppCommandBuffer != nullptr ) && ( *ppCommandBuffer != nullptr ) )
                {
                    static_cast<VulkanCommandBuffer*>( *ppCommandBuffer )->Destroy();
                    Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), ppCommandBuffer );
                }
            }

            RESULT VulkanDevice::CreateShader( const ShaderDesc& desc, ShaderHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Shader (pHandle) must be a valid address." );
                BGS_ASSERT( desc.pByteCode != nullptr, "Source code (desc.pByteCode) must be a valid pointer." );
                if( desc.pByteCode == nullptr )
                {
                    return Results::FAIL;
                }

                VkShaderModuleCreateInfo shaderInfo;
                shaderInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                shaderInfo.pNext    = nullptr;
                shaderInfo.flags    = 0;
                shaderInfo.codeSize = desc.codeSize;
                shaderInfo.pCode    = reinterpret_cast<const uint32_t*>( desc.pByteCode );

                VkDevice       nativeDevice = m_handle.GetNativeHandle();
                VkShaderModule nativeShader = pHandle->GetNativeHandle();

                if( vkCreateShaderModule( nativeDevice, &shaderInfo, nullptr, &nativeShader ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }

                *pHandle = ShaderHandle( nativeShader );

                return Results::OK;
            }

            void VulkanDevice::DestroyShader( ShaderHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Shader (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != ShaderHandle(), "Shader (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != ShaderHandle() ) )
                {
                    VkDevice       nativeDevice = m_handle.GetNativeHandle();
                    VkShaderModule nativeShader = pHandle->GetNativeHandle();

                    vkDestroyShaderModule( nativeDevice, nativeShader, nullptr );

                    *pHandle = ShaderHandle();
                }
            }

            RESULT VulkanDevice::CreatePipelineLayout( const PipelineLayoutDesc& desc, PipelineLayoutHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Pipeline layout (pHandle) must be a valid address." );
                BGS_ASSERT( desc.constantRangeCount <= BGS_ENUM_COUNT( ShaderVisibilities ),
                            "Push constant range count (desc.constantRangeCount) must be less than %d.", BGS_ENUM_COUNT( ShaderVisibilities ) );
                BGS_ASSERT( desc.hBindigHeapLayout != BindingHeapLayoutHandle(),
                            "Binding heap layout (desc.hBindigHeapLayout) must be valid handle." )
                if( ( desc.constantRangeCount > BGS_ENUM_COUNT( ShaderVisibilities ) ) || ( desc.hBindigHeapLayout == BindingHeapLayoutHandle() ) )
                {
                    return Results::FAIL;
                }

                VkDevice         nativeDevice = m_handle.GetNativeHandle();
                VkPipelineLayout nativeLayout = VK_NULL_HANDLE;

                VkPushConstantRange   constantRanges[ BGS_ENUM_COUNT( ShaderVisibilities ) ]; // each for one of shader visibility
                VkDescriptorSetLayout nativeSetLayout = desc.hBindigHeapLayout.GetNativeHandle();

                for( index_t ndx = 0; static_cast<uint32_t>( ndx ) < desc.constantRangeCount; ++ndx )
                {
                    const PushConstantRangeDesc& currDesc = desc.pConstantRanges[ ndx ];
                    VkPushConstantRange&         range    = constantRanges[ ndx ];

                    // Mimicing D3D12 behaviour as constants are 32 bits there
                    range.offset     = 4 * currDesc.baseConstantSlot;
                    range.size       = 4 * currDesc.constantCount;
                    range.stageFlags = MapBigosShaderVisibilityToVulkanShaderStageFlags( currDesc.visibility );
                }

                VkPipelineLayoutCreateInfo layoutInfo;
                layoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                layoutInfo.pNext                  = nullptr;
                layoutInfo.flags                  = 0;
                layoutInfo.setLayoutCount         = 1;
                layoutInfo.pSetLayouts            = &nativeSetLayout;
                layoutInfo.pushConstantRangeCount = desc.constantRangeCount;
                layoutInfo.pPushConstantRanges    = constantRanges;

                if( vkCreatePipelineLayout( nativeDevice, &layoutInfo, nullptr, &nativeLayout ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }

                *pHandle = PipelineLayoutHandle( nativeLayout );

                return Results::OK;
            }

            void VulkanDevice::DestroyPipelineLayout( PipelineLayoutHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Pipeline layout (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != PipelineLayoutHandle(), "Pipeline layout (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != PipelineLayoutHandle() ) )
                {
                    VkDevice         nativeDevice = m_handle.GetNativeHandle();
                    VkPipelineLayout nativeLayout = pHandle->GetNativeHandle();

                    vkDestroyPipelineLayout( nativeDevice, nativeLayout, nullptr );

                    *pHandle = PipelineLayoutHandle();
                }
            }

            RESULT VulkanDevice::CreatePipeline( const PipelineDesc& desc, PipelineHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Pipeline (pHandle) must be a valid address." );
                if( pHandle == nullptr )
                {
                    return Results::FAIL;
                }

                VkPipeline nativePipeline = VK_NULL_HANDLE;

                switch( desc.type )
                {
                    case PipelineTypes::GRAPHICS:
                    {
                        const GraphicsPipelineDesc& gpDesc = static_cast<const GraphicsPipelineDesc&>( desc );

                        if( BGS_FAILED( CreateVkGraphicsPipeline( gpDesc, &nativePipeline ) ) )
                        {
                            return Results::FAIL;
                        }

                        break;
                    }
                    case PipelineTypes::COMPUTE:
                    {
                        // TODO: Implement
                        break;
                    }
                    case PipelineTypes::RAY_TRACING:
                    {
                        // TODO: Implement
                        return Results::FAIL;
                    }
                }

                *pHandle = PipelineHandle( nativePipeline );

                return Results::OK;
            }

            void VulkanDevice::DestroyPipeline( PipelineHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Pipeline (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != PipelineHandle(), "Pipeline (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != PipelineHandle() ) )
                {
                    VkDevice   nativeDevice   = m_handle.GetNativeHandle();
                    VkPipeline nativePipeline = pHandle->GetNativeHandle();

                    vkDestroyPipeline( nativeDevice, nativePipeline, nullptr );

                    *pHandle = PipelineHandle();
                }
            }

            RESULT VulkanDevice::CreateFence( const FenceDesc& desc, FenceHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Fence (pHandle) must be a valid address." );

                VkSemaphoreTypeCreateInfo timelineInfo;
                timelineInfo.sType         = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
                timelineInfo.pNext         = nullptr;
                timelineInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
                timelineInfo.initialValue  = desc.initialValue;

                VkSemaphoreCreateInfo semaphoreInfo;
                semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                semaphoreInfo.pNext = &timelineInfo;
                semaphoreInfo.flags = 0;

                // Timeline semaphores works the same way as ID3D12Fence
                VkSemaphore nativeFence  = VK_NULL_HANDLE;
                VkDevice    nativeDevice = m_handle.GetNativeHandle();

                if( vkCreateSemaphore( nativeDevice, &semaphoreInfo, nullptr, &nativeFence ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }

                *pHandle = FenceHandle( nativeFence );

                return Results::OK;
            }

            void VulkanDevice::DestroyFence( FenceHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Fence (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != FenceHandle(), "Fence (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != FenceHandle() ) )
                {
                    VkDevice    nativeDevice = m_handle.GetNativeHandle();
                    VkSemaphore nativeFence  = pHandle->GetNativeHandle();

                    vkDestroySemaphore( nativeDevice, nativeFence, nullptr );

                    *pHandle = FenceHandle();
                }
            }

            RESULT VulkanDevice::WaitForFences( const WaitForFencesDesc& desc, uint64_t timeout )
            {
                BGS_ASSERT( desc.pFences != nullptr, "Fence (desc.pFences) must be a valid pointer." );
                BGS_ASSERT( desc.pWaitValues != nullptr, "Uint array (desc.pWaitValues) must be a valid address." );
                BGS_ASSERT( desc.fenceCount < Config::Driver::Synchronization::MAX_FENCES_TO_WAIT_COUNT,
                            "Fence count (desc.fenceCount) must be less than %d", Config::Driver::Synchronization::MAX_FENCES_TO_WAIT_COUNT );
                if( ( desc.pFences == nullptr ) || ( desc.pWaitValues == nullptr ) ||
                    ( desc.fenceCount >= Config::Driver::Synchronization::MAX_FENCES_TO_WAIT_COUNT ) )
                {
                    return Results::FAIL;
                }

                uint64_t    waitVals[ Config::Driver::Synchronization::MAX_FENCES_TO_WAIT_COUNT ];
                VkSemaphore fences[ Config::Driver::Synchronization::MAX_FENCES_TO_WAIT_COUNT ];
                for( index_t ndx = 0; ndx < static_cast<index_t>( desc.fenceCount ); ++ndx )
                {
                    waitVals[ ndx ] = desc.pWaitValues[ ndx ];
                    fences[ ndx ]   = desc.pFences[ ndx ].GetNativeHandle();
                }

                VkSemaphoreWaitInfo waitInfo;
                waitInfo.sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
                waitInfo.pNext          = nullptr;
                waitInfo.flags          = desc.waitAll == BGS_TRUE ? 0 : VK_SEMAPHORE_WAIT_ANY_BIT;
                waitInfo.semaphoreCount = desc.fenceCount;
                waitInfo.pSemaphores    = fences;
                waitInfo.pValues        = waitVals;

                VkDevice nativeDevice = m_handle.GetNativeHandle();
                VkResult result       = vkWaitSemaphores( nativeDevice, &waitInfo, timeout );

                if( result == VK_TIMEOUT )
                {
                    return Results::TIMEOUT;
                }
                else if( result != VK_SUCCESS )
                {
                    return Results::FAIL;
                }

                return Results::OK;
            }

            RESULT VulkanDevice::SignalFence( uint64_t value, FenceHandle handle )
            {
                BGS_ASSERT( handle != FenceHandle(), "Fence (handle) must be a valid handle." );
                if( handle == FenceHandle() )
                {
                    return Results::FAIL;
                }

                VkSemaphore nativeFence  = handle.GetNativeHandle();
                VkDevice    nativeDevice = m_handle.GetNativeHandle();

                VkSemaphoreSignalInfo signalInfo;
                signalInfo.sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
                signalInfo.pNext     = nullptr;
                signalInfo.semaphore = nativeFence;
                signalInfo.value     = value;

                if( vkSignalSemaphore( nativeDevice, &signalInfo ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }

                return Results::OK;
            }

            RESULT VulkanDevice::GetFenceValue( FenceHandle handle, uint64_t* pValue )
            {
                BGS_ASSERT( handle != FenceHandle(), "Fence (handle) must be a valid handle." );
                if( handle == FenceHandle() )
                {
                    return Results::FAIL;
                }

                VkSemaphore nativeFence  = handle.GetNativeHandle();
                VkDevice    nativeDevice = m_handle.GetNativeHandle();

                if( vkGetSemaphoreCounterValue( nativeDevice, nativeFence, pValue ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }

                return Results::OK;
            }

            RESULT VulkanDevice::CreateSemaphore( const SemaphoreDesc& desc, SemaphoreHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Semaphore (pHandle) must be a valid address." );
                desc;

                VkSemaphoreCreateInfo semaphoreInfo;
                semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                semaphoreInfo.pNext = nullptr;
                semaphoreInfo.flags = 0;

                VkDevice    nativeDevice    = m_handle.GetNativeHandle();
                VkSemaphore nativeSemaphore = VK_NULL_HANDLE;

                if( vkCreateSemaphore( nativeDevice, &semaphoreInfo, nullptr, &nativeSemaphore ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }

                *pHandle = SemaphoreHandle( nativeSemaphore );

                return Results::OK;
            }

            void VulkanDevice::DestroySemaphore( SemaphoreHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Semaphore (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != SemaphoreHandle(), "Semaphore (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != SemaphoreHandle() ) )
                {
                    VkDevice    nativeDevice    = m_handle.GetNativeHandle();
                    VkSemaphore nativeSemaphore = pHandle->GetNativeHandle();

                    vkDestroySemaphore( nativeDevice, nativeSemaphore, nullptr );

                    *pHandle = SemaphoreHandle();
                }
            }

            RESULT VulkanDevice::AllocateMemory( const AllocateMemoryDesc& desc, MemoryHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Memory (pHandle) must be a valid address." );

                VkDevice      nativeDevice = m_handle.GetNativeHandle();
                VulkanMemory* pNativeMem   = nullptr;

                MemoryAccessFlags access = desc.access;
                if( desc.heapType != MemoryHeapTypes::CUSTOM )
                {
                    access = MapBigosMemoryHeapTypeToMemoryAccessFlags( desc.heapType );
                }

                VkMemoryPropertyFlags nativePropsFlags = MapBigosMemoryAccessFlagsToVulkanMemoryPropertFlags( access );

                const VkPhysicalDeviceMemoryProperties& nativeProps = m_heapProperties.memoryProperties;
                const int32_t                           ndx         = FindVulkanMemTypeNdx( &nativeProps, MAX_UINT32, nativePropsFlags );
                if( ndx < 0 )
                {
                    return Results::NOT_FOUND;
                }
                // TODO: Handle heap capacity

                if( BGS_FAILED( Core::Memory::AllocateObject( m_pParent->GetParent()->GetDefaultAllocator(), &pNativeMem ) ) )
                {
                    return Results::NO_MEMORY;
                }

                VkMemoryAllocateInfo allocInfo;
                allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.pNext           = nullptr;
                allocInfo.allocationSize  = desc.size;
                allocInfo.memoryTypeIndex = ndx;

                if( vkAllocateMemory( nativeDevice, &allocInfo, nullptr, &pNativeMem->nativeMemory ) != VK_SUCCESS )
                {
                    Core::Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), &pNativeMem );
                    return Results::FAIL;
                }

                // Maping memory for futer use i9n D3D12 behaviour emulation
                if( nativePropsFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT )
                {
                    if( vkMapMemory( nativeDevice, pNativeMem->nativeMemory, 0, VK_WHOLE_SIZE, 0, &pNativeMem->pHostMemory ) != VK_SUCCESS )
                    {
                        vkFreeMemory( nativeDevice, pNativeMem->nativeMemory, nullptr );
                        Core::Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), &pNativeMem );
                        return Results::FAIL;
                    }
                }

                *pHandle = MemoryHandle( pNativeMem );

                return Results::OK;
            }

            void VulkanDevice::FreeMemory( MemoryHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Memory (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != MemoryHandle(), "Memory (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != MemoryHandle() ) )
                {
                    VkDevice      nativeDevice = m_handle.GetNativeHandle();
                    VulkanMemory* pNativeMem   = pHandle->GetNativeHandle();

                    vkFreeMemory( nativeDevice, pNativeMem->nativeMemory, nullptr );

                    Core::Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), &pNativeMem );

                    *pHandle = MemoryHandle();
                }
            }

            RESULT VulkanDevice::CreateResource( const ResourceDesc& desc, ResourceHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Memory (pHandle) must be a valid address." );
                // TODO: Think about validation, because things ale getting complex

                VulkanResource* pRes = nullptr;
                if( BGS_FAILED( Core::Memory::AllocateObject( m_pParent->GetParent()->GetDefaultAllocator(), &pRes ) ) )
                {
                    return Results::NO_MEMORY;
                }

                if( desc.resourceType == ResourceTypes::BUFFER )
                {
                    VkBuffer nativeBuffer = VK_NULL_HANDLE;
                    if( BGS_FAILED( CreateVkBuffer( desc, &nativeBuffer ) ) )
                    {
                        return Results::FAIL;
                    }
                    pRes->type   = VulkanResourceTypes::BUFFER;
                    pRes->buffer = nativeBuffer;
                }
                else // Images
                {
                    VkImage nativeImage = VK_NULL_HANDLE;
                    if( BGS_FAILED( CreateVkImage( desc, &nativeImage ) ) )
                    {
                        return Results::FAIL;
                    }
                    pRes->type  = VulkanResourceTypes::IMAGE;
                    pRes->image = nativeImage;
                }
                pRes->pMemory      = nullptr;
                pRes->memoryOffset = INVALID_OFFSET;
                pRes->usage        = desc.resourceUsage;
                *pHandle           = ResourceHandle( pRes );

                return Results::OK;
            }

            void VulkanDevice::DestroyResource( ResourceHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Resource (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != ResourceHandle(), "Resource (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != ResourceHandle() ) )
                {
                    VkDevice        nativeDevice = m_handle.GetNativeHandle();
                    VulkanResource* pNativeRes   = pHandle->GetNativeHandle();

                    if( pNativeRes->type == VulkanResourceTypes::BUFFER )
                    {
                        vkDestroyBuffer( nativeDevice, pNativeRes->buffer, nullptr );
                    }
                    else // Images
                    {
                        vkDestroyImage( nativeDevice, pNativeRes->image, nullptr );
                    }

                    Core::Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), &pNativeRes );

                    *pHandle = ResourceHandle();
                }
            }

            RESULT VulkanDevice::BindResourceMemory( const BindResourceMemoryDesc& desc )
            {
                BGS_ASSERT( desc.hResource != ResourceHandle(), "Resource (desc.hResource) must be a valid handle." )
                BGS_ASSERT( desc.hResource.GetNativeHandle() != nullptr, "Resource (desc.hResource) must hold valid internal resource." );
                BGS_ASSERT( desc.hMemory != MemoryHandle(), "Memory (desc.hMemory) must be a valid handle." )
                BGS_ASSERT( desc.hMemory.GetNativeHandle() != nullptr, "Memory (desc.hMemory) must hold valid internal memory." );
                if( ( desc.hResource == ResourceHandle() ) && ( desc.hResource.GetNativeHandle() == nullptr ) && ( desc.hMemory == MemoryHandle() ) &&
                    ( desc.hMemory.GetNativeHandle() == nullptr ) )
                {
                    return Results::FAIL;
                }

                VkDevice        nativeDevice = m_handle.GetNativeHandle();
                VulkanResource* pRes         = desc.hResource.GetNativeHandle();
                VkDeviceMemory  nativeMem    = desc.hMemory.GetNativeHandle()->nativeMemory;
                if( pRes->type == VulkanResourceTypes::BUFFER )
                {
                    if( vkBindBufferMemory( nativeDevice, pRes->buffer, nativeMem, desc.memoryOffset ) != VK_SUCCESS )
                    {
                        return Results::FAIL;
                    }
                }
                else // Images
                {
                    if( vkBindImageMemory( nativeDevice, pRes->image, nativeMem, desc.memoryOffset ) != VK_SUCCESS )
                    {
                        return Results::FAIL;
                    }
                }

                pRes->memoryOffset = desc.memoryOffset;
                pRes->pMemory      = desc.hMemory.GetNativeHandle();

                return Results::OK;
            }

            void VulkanDevice::GetResourceAllocationInfo( ResourceHandle handle, ResourceAllocationInfo* pInfo )
            {
                BGS_ASSERT( handle != ResourceHandle(), "Resource (handle) must be a valid handle." )
                BGS_ASSERT( handle.GetNativeHandle() != nullptr, "Resource (handle) must hold valid internal resource." );
                BGS_ASSERT( pInfo != nullptr, "Resource allocation info (pInfo) must be a valid address." );
                if( ( handle != ResourceHandle() ) && ( handle.GetNativeHandle() != nullptr ) && ( pInfo != nullptr ) )
                {
                    VkDevice        nativeDevice = m_handle.GetNativeHandle();
                    VulkanResource* pRes         = handle.GetNativeHandle();

                    VkMemoryRequirements memRequirements;

                    if( pRes->type == VulkanResourceTypes::IMAGE )
                    {
                        vkGetImageMemoryRequirements( nativeDevice, pRes->image, &memRequirements );
                    }
                    else // Buffer
                    {
                        vkGetBufferMemoryRequirements( nativeDevice, pRes->buffer, &memRequirements );
                    }

                    pInfo->alignment = memRequirements.alignment;
                    pInfo->size      = memRequirements.size;
                }
            }

            RESULT VulkanDevice::MapResource( const MapResourceDesc& desc, void** ppResource )
            {
                BGS_ASSERT( desc.hResource != ResourceHandle(), "Resource (hResource) must be a valid handle." );
                BGS_ASSERT( desc.hResource.GetNativeHandle() != nullptr, "Resource (hResource) must hold valid internal resource." );
                BGS_ASSERT( ppResource != nullptr, "Memory (ppResource) must be a valid address." );
                if( ( desc.hResource == ResourceHandle() ) && ( ppResource == nullptr ) )
                {
                    return Results::FAIL;
                }

                VkDevice        nativeDevice = m_handle.GetNativeHandle();
                VulkanResource* pRes         = desc.hResource.GetNativeHandle();
                VkDeviceSize    offset       = pRes->memoryOffset;
                VkDeviceSize    size;
                if( pRes->type == VulkanResourceTypes::IMAGE )
                {
                    VkImage             nativeImage = pRes->image;
                    VkSubresourceLayout subresourceLayout;
                    VkImageSubresource  subresource;
                    subresource.aspectMask = MapBigosTextureComponentFlagsToVulkanImageAspectFlags( desc.textureRange.components );
                    subresource.arrayLayer = desc.textureRange.arrayLayer;
                    subresource.mipLevel   = desc.textureRange.mipLevel;

                    vkGetImageSubresourceLayout( nativeDevice, nativeImage, &subresource, &subresourceLayout );

                    // Note that subresourceLayout.offset is offset from begining of resource
                    offset += subresourceLayout.offset;
                    size = subresourceLayout.size;
                }
                else
                {
                    offset += desc.bufferRange.offset;
                    size = desc.bufferRange.size;
                }

                VkMappedMemoryRange nativeMemRange;
                nativeMemRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
                nativeMemRange.pNext  = nullptr;
                nativeMemRange.memory = pRes->pMemory->nativeMemory;
                nativeMemRange.size   = size;
                nativeMemRange.offset = offset;

                if( vkInvalidateMappedMemoryRanges( nativeDevice, 1, &nativeMemRange ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }

                byte_t* pMem = static_cast<byte_t*>( pRes->pMemory->pHostMemory );
                pMem += offset;
                *ppResource = static_cast<void*>( pMem );

                return Results::OK;
            }

            RESULT VulkanDevice::UnmapResource( const MapResourceDesc& desc )
            {
                BGS_ASSERT( desc.hResource != ResourceHandle(), "Resource (hResource) must be a valid handle." );
                BGS_ASSERT( desc.hResource.GetNativeHandle() != nullptr, "Resource (hResource) must hold valid internal resource." );
                if( desc.hResource == ResourceHandle() )
                {
                    return Results::FAIL;
                }

                VkDevice        nativeDevice = m_handle.GetNativeHandle();
                VulkanResource* pRes         = desc.hResource.GetNativeHandle();
                VkDeviceSize    offset       = pRes->memoryOffset;
                VkDeviceSize    size;
                if( pRes->type == VulkanResourceTypes::IMAGE )
                {
                    VkImage             nativeImage = pRes->image;
                    VkSubresourceLayout subresourceLayout;
                    VkImageSubresource  subresource;
                    subresource.aspectMask = MapBigosTextureComponentFlagsToVulkanImageAspectFlags( desc.textureRange.components );
                    subresource.arrayLayer = desc.textureRange.arrayLayer;
                    subresource.mipLevel   = desc.textureRange.mipLevel;

                    vkGetImageSubresourceLayout( nativeDevice, nativeImage, &subresource, &subresourceLayout );

                    // Note that subresourceLayout.offset is offset from begining of resource
                    offset += subresourceLayout.offset;
                    size = subresourceLayout.size;
                }
                else
                {
                    offset += desc.bufferRange.offset;
                    size = desc.bufferRange.size;
                }

                VkMappedMemoryRange nativeMemRange;
                nativeMemRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
                nativeMemRange.pNext  = nullptr;
                nativeMemRange.memory = pRes->pMemory->nativeMemory;
                nativeMemRange.size   = size;
                nativeMemRange.offset = offset;

                if( vkFlushMappedMemoryRanges( nativeDevice, 1, &nativeMemRange ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }

                return Results::OK;
            }

            RESULT VulkanDevice::CreateResourceView( const ResourceViewDesc& desc, ResourceViewHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Resource view (pHandle) must be a valid address." );
                if( ( pHandle == nullptr ) ) // TODO: Validation
                {
                    return Results::FAIL;
                }

                VulkanResourceView* pResView = nullptr;
                if( BGS_FAILED( Core::Memory::AllocateObject( m_pParent->GetParent()->GetDefaultAllocator(), &pResView ) ) )
                {
                    return Results::NO_MEMORY;
                }
                pResView->descriptorData.pData = nullptr;
                pResView->descriptorData.size  = 0;

                VkDevice nativeDevice = m_handle.GetNativeHandle();
                if( desc.hResource.GetNativeHandle()->type == VulkanResourceTypes::IMAGE )
                {
                    const TextureViewDesc& texDesc    = static_cast<const TextureViewDesc&>( desc );
                    VkImageView            nativeView = VK_NULL_HANDLE;

                    VkImageViewCreateInfo viewInfo;
                    viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                    viewInfo.pNext                           = nullptr;
                    viewInfo.flags                           = 0;
                    viewInfo.image                           = texDesc.hResource.GetNativeHandle()->image;
                    viewInfo.viewType                        = MapBigosTextureTypeToVulkanImageViewType( texDesc.textureType );
                    viewInfo.format                          = MapBigosFormatToVulkanFormat( texDesc.format );
                    viewInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
                    viewInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
                    viewInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
                    viewInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
                    viewInfo.subresourceRange.aspectMask     = MapBigosTextureComponentFlagsToVulkanImageAspectFlags( texDesc.range.components );
                    viewInfo.subresourceRange.baseMipLevel   = texDesc.range.mipLevel;
                    viewInfo.subresourceRange.levelCount     = texDesc.range.mipLevelCount;
                    viewInfo.subresourceRange.baseArrayLayer = texDesc.range.arrayLayer;
                    viewInfo.subresourceRange.layerCount     = texDesc.range.arrayLayerCount;

                    if( vkCreateImageView( nativeDevice, &viewInfo, nullptr, &nativeView ) != VK_SUCCESS )
                    {
                        Core::Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), &pResView );
                        return Results::FAIL;
                    }
                    pResView->imageView = nativeView;
                    pResView->type      = VulkanResourceTypes::IMAGE;

                    // Retriving descriptor data for image view based descriptor (not needed for rtv and dsv)
                    if( desc.usage & BGS_FLAG( ResourceViewUsageFlagBits::SAMPLED_TEXTURE ) )
                    {
                        VulkanDescriptorInfo& currInfo = pResView->descriptorData;

                        VkDescriptorImageInfo imageInfo;
                        imageInfo.imageView   = nativeView;
                        imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

                        VkDescriptorGetInfoEXT getInfo;
                        getInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
                        getInfo.pNext              = nullptr;
                        getInfo.type               = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                        getInfo.data.pSampledImage = &imageInfo;

                        vkGetDescriptorEXT( nativeDevice, &getInfo, currInfo.size, currInfo.pData );
                    }
                    else if( desc.usage & BGS_FLAG( ResourceViewUsageFlagBits::STORAGE_TEXTURE ) )
                    {
                        VulkanDescriptorInfo& currInfo = pResView->descriptorData;

                        VkDescriptorImageInfo imageInfo;
                        imageInfo.imageView   = nativeView;
                        imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

                        VkDescriptorGetInfoEXT getInfo;
                        getInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
                        getInfo.pNext              = nullptr;
                        getInfo.type               = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                        getInfo.data.pStorageImage = &imageInfo;

                        vkGetDescriptorEXT( nativeDevice, &getInfo, currInfo.size, currInfo.pData );
                    }
                }
                else
                {
                    // Retriving descriptor data for buffer based descriptor
                    if( desc.usage & BGS_FLAG( ResourceViewUsageFlagBits::CONSTANT_TEXEL_BUFFER ) )
                    {
                        const TexelBufferViewDesc& buffDesc = static_cast<const TexelBufferViewDesc&>( desc );
                        VulkanDescriptorInfo&      currInfo = pResView->descriptorData;

                        VkBufferDeviceAddressInfo addressInfo;
                        addressInfo.sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
                        addressInfo.pNext  = nullptr;
                        addressInfo.buffer = buffDesc.hResource.GetNativeHandle()->buffer;

                        VkDescriptorAddressInfoEXT buffInfo;
                        buffInfo.sType   = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT;
                        buffInfo.pNext   = nullptr;
                        buffInfo.address = vkGetBufferDeviceAddress( nativeDevice, &addressInfo ) + buffDesc.range.offset;
                        buffInfo.range   = buffDesc.range.size;
                        buffInfo.format  = MapBigosFormatToVulkanFormat( buffDesc.format );

                        VkDescriptorGetInfoEXT getInfo;
                        getInfo.sType                    = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
                        getInfo.pNext                    = nullptr;
                        getInfo.type                     = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
                        getInfo.data.pUniformTexelBuffer = &buffInfo;

                        vkGetDescriptorEXT( nativeDevice, &getInfo, currInfo.size, currInfo.pData );
                    }
                    else if( desc.usage & BGS_FLAG( ResourceViewUsageFlagBits::STORAGE_TEXEL_BUFFER ) )
                    {
                        const TexelBufferViewDesc& buffDesc = static_cast<const TexelBufferViewDesc&>( desc );
                        VulkanDescriptorInfo&      currInfo = pResView->descriptorData;

                        VkBufferDeviceAddressInfo addressInfo;
                        addressInfo.sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
                        addressInfo.pNext  = nullptr;
                        addressInfo.buffer = buffDesc.hResource.GetNativeHandle()->buffer;

                        VkDescriptorAddressInfoEXT buffInfo;
                        buffInfo.sType   = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT;
                        buffInfo.pNext   = nullptr;
                        buffInfo.address = vkGetBufferDeviceAddress( nativeDevice, &addressInfo ) + buffDesc.range.offset;
                        buffInfo.range   = buffDesc.range.size;
                        buffInfo.format  = MapBigosFormatToVulkanFormat( buffDesc.format );

                        VkDescriptorGetInfoEXT getInfo;
                        getInfo.sType                    = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
                        getInfo.pNext                    = nullptr;
                        getInfo.type                     = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
                        getInfo.data.pStorageTexelBuffer = &buffInfo;

                        vkGetDescriptorEXT( nativeDevice, &getInfo, currInfo.size, currInfo.pData );
                    }
                    else if( desc.usage & BGS_FLAG( ResourceUsageFlagBits::CONSTANT_BUFFER ) )
                    {
                        const BufferViewDesc& buffDesc = static_cast<const BufferViewDesc&>( desc );
                        VulkanDescriptorInfo& currInfo = pResView->descriptorData;

                        VkBufferDeviceAddressInfo addressInfo;
                        addressInfo.sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
                        addressInfo.pNext  = nullptr;
                        addressInfo.buffer = buffDesc.hResource.GetNativeHandle()->buffer;

                        VkDescriptorAddressInfoEXT buffInfo;
                        buffInfo.sType   = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT;
                        buffInfo.pNext   = nullptr;
                        buffInfo.address = vkGetBufferDeviceAddress( nativeDevice, &addressInfo ) + buffDesc.range.offset;
                        buffInfo.range   = buffDesc.range.size;

                        VkDescriptorGetInfoEXT getInfo;
                        getInfo.sType               = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
                        getInfo.pNext               = nullptr;
                        getInfo.type                = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                        getInfo.data.pUniformBuffer = &buffInfo;

                        vkGetDescriptorEXT( nativeDevice, &getInfo, currInfo.size, currInfo.pData );
                    }
                    else if( desc.usage & BGS_FLAG( ResourceUsageFlagBits::READ_ONLY_STORAGE_BUFFER ) )
                    {
                        const BufferViewDesc& buffDesc = static_cast<const BufferViewDesc&>( desc );
                        VulkanDescriptorInfo& currInfo = pResView->descriptorData;

                        VkBufferDeviceAddressInfo addressInfo;
                        addressInfo.sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
                        addressInfo.pNext  = nullptr;
                        addressInfo.buffer = buffDesc.hResource.GetNativeHandle()->buffer;

                        VkDescriptorAddressInfoEXT buffInfo;
                        buffInfo.sType   = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT;
                        buffInfo.pNext   = nullptr;
                        buffInfo.address = vkGetBufferDeviceAddress( nativeDevice, &addressInfo ) + buffDesc.range.offset;
                        buffInfo.range   = buffDesc.range.size;

                        VkDescriptorGetInfoEXT getInfo;
                        getInfo.sType               = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
                        getInfo.pNext               = nullptr;
                        getInfo.type                = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        getInfo.data.pStorageBuffer = &buffInfo;

                        vkGetDescriptorEXT( nativeDevice, &getInfo, currInfo.size, currInfo.pData );
                    }
                    else if( desc.usage & BGS_FLAG( ResourceUsageFlagBits::READ_WRITE_STORAGE_BUFFER ) )
                    {
                        const BufferViewDesc& buffDesc = static_cast<const BufferViewDesc&>( desc );
                        VulkanDescriptorInfo& currInfo = pResView->descriptorData;

                        VkBufferDeviceAddressInfo addressInfo;
                        addressInfo.sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
                        addressInfo.pNext  = nullptr;
                        addressInfo.buffer = buffDesc.hResource.GetNativeHandle()->buffer;

                        VkDescriptorAddressInfoEXT buffInfo;
                        buffInfo.sType   = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT;
                        buffInfo.pNext   = nullptr;
                        buffInfo.address = vkGetBufferDeviceAddress( nativeDevice, &addressInfo ) + buffDesc.range.offset;
                        buffInfo.range   = buffDesc.range.size;

                        VkDescriptorGetInfoEXT getInfo;
                        getInfo.sType               = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
                        getInfo.pNext               = nullptr;
                        getInfo.type                = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        getInfo.data.pStorageBuffer = &buffInfo;

                        vkGetDescriptorEXT( nativeDevice, &getInfo, currInfo.size, currInfo.pData );
                    }
                }

                *pHandle = ResourceViewHandle( pResView );

                return Results::OK;
            }

            void VulkanDevice::DestroyResourceView( ResourceViewHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Resource view (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != ResourceViewHandle(), "Resource view (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != ResourceViewHandle() ) )
                {
                    VulkanResourceView* pNativeView = pHandle->GetNativeHandle();
                    if( pNativeView->type == VulkanResourceTypes::IMAGE )
                    {
                        VkDevice    nativeDevice = m_handle.GetNativeHandle();
                        VkImageView nativeView   = pNativeView->imageView;

                        vkDestroyImageView( nativeDevice, nativeView, nullptr );
                    }

                    Core::Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), &pNativeView );

                    *pHandle = ResourceViewHandle();
                }
            }

            RESULT VulkanDevice::CreateSampler( const SamplerDesc& desc, SamplerHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Sampler (pHandle) must be a valid address." );
                if( ( pHandle == nullptr ) ) // TODO: Validation
                {
                    return Results::FAIL;
                }

                VkDevice  nativeDevice  = m_handle.GetNativeHandle();
                VkSampler nativeSampler = VK_NULL_HANDLE;

                VkSamplerCustomBorderColorCreateInfoEXT borderColorInfo;
                if( desc.type == SamplerTypes::NORMAL )
                {
                    borderColorInfo.sType                          = VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT;
                    borderColorInfo.pNext                          = nullptr;
                    borderColorInfo.format                         = VK_FORMAT_UNDEFINED;
                    borderColorInfo.customBorderColor.float32[ 0 ] = desc.customBorderColor.r;
                    borderColorInfo.customBorderColor.float32[ 1 ] = desc.customBorderColor.g;
                    borderColorInfo.customBorderColor.float32[ 2 ] = desc.customBorderColor.b;
                    borderColorInfo.customBorderColor.float32[ 3 ] = desc.customBorderColor.a;
                }

                VkSamplerReductionModeCreateInfo reductionModeInfo;
                reductionModeInfo.sType         = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO;
                reductionModeInfo.pNext         = desc.type == SamplerTypes::NORMAL ? &borderColorInfo : nullptr;
                reductionModeInfo.reductionMode = MapBigosSamplerReductionModeToVulkanSamplerReductionMode( desc.reductionMode );

                VkSamplerCreateInfo samplerInfo;
                samplerInfo.sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                samplerInfo.pNext            = &reductionModeInfo;
                samplerInfo.flags            = 0;
                samplerInfo.minFilter        = MapBigosFilterTypeToVulkanFilterType( desc.minFilter );
                samplerInfo.magFilter        = MapBigosFilterTypeToVulkanFilterType( desc.magFilter );
                samplerInfo.mipmapMode       = MapBigosFilterTypeToVulkanMipMapMode( desc.mipMapFilter );
                samplerInfo.addressModeU     = MapBigosTextureAddressModeToVultakSamplerAddressMode( desc.addressU );
                samplerInfo.addressModeV     = MapBigosTextureAddressModeToVultakSamplerAddressMode( desc.addressV );
                samplerInfo.addressModeW     = MapBigosTextureAddressModeToVultakSamplerAddressMode( desc.addressW );
                samplerInfo.anisotropyEnable = desc.anisotropyEnable;
                samplerInfo.maxAnisotropy    = desc.maxAnisotropy;
                samplerInfo.compareEnable    = desc.compareEnable;
                samplerInfo.compareOp        = MapBigosCompareOperationTypeToVulkanCompareOp( desc.compareOperation );
                samplerInfo.minLod           = desc.minLod;
                samplerInfo.maxLod           = desc.maxLod;
                samplerInfo.mipLodBias       = desc.mipLodBias;
                samplerInfo.borderColor      = desc.type == SamplerTypes::IMMUTABLE ? MapBigosBorderColorToVulkanBorderColor( desc.enumBorderColor )
                                                                                    : VK_BORDER_COLOR_FLOAT_CUSTOM_EXT;
                samplerInfo.unnormalizedCoordinates = VK_FALSE;

                if( vkCreateSampler( nativeDevice, &samplerInfo, nullptr, &nativeSampler ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }

                *pHandle = SamplerHandle( nativeSampler );

                return Results::OK;
            }

            void VulkanDevice::DestroySampler( SamplerHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Sampler (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != SamplerHandle(), "Sampler (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != SamplerHandle() ) )
                {
                    VkDevice  nativeDevice  = m_handle.GetNativeHandle();
                    VkSampler nativeSampler = pHandle->GetNativeHandle();

                    vkDestroySampler( nativeDevice, nativeSampler, nullptr );

                    *pHandle = SamplerHandle();
                }
            }

            RESULT VulkanDevice::CreateBindingHeapLayout( const BindingHeapLayoutDesc& desc, BindingHeapLayoutHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Binding heap layout (pHandle) must be a valid address." );
                BGS_ASSERT( desc.bindingRangeCount <= Config::Driver::Pipeline::MAX_BINDING_RANGE_COUNT,
                            "Binding count (desc.bindingCount) must be less than %d", Config::Driver::Pipeline::MAX_BINDING_RANGE_COUNT );
                if( ( pHandle == nullptr ) || ( desc.bindingRangeCount > Config::Driver::Pipeline::MAX_BINDING_RANGE_COUNT ) )
                {
                    return Results::FAIL;
                }

                uint32_t                     immutableSamplerCnt = 0;
                VkDevice                     nativeDevice        = m_handle.GetNativeHandle();
                VkDescriptorSetLayout        nativeLayout        = VK_NULL_HANDLE;
                VkDescriptorSetLayoutBinding layoutBindings[ Config::Driver::Pipeline::MAX_BINDING_RANGE_COUNT ];
                VkSampler                    immutableSamplerRanges[ Config::Driver::Pipeline::MAX_BINDING_RANGE_COUNT ]
                                                [ Config::Driver::Pipeline::MAX_IMMUTABLE_SAMPLER_COUNT ];
                for( index_t ndx = 0; static_cast<uint32_t>( ndx ) < desc.bindingRangeCount; ++ndx )
                {
                    const BindingRangeDesc&       currDesc    = desc.pBindingRanges[ ndx ];
                    VkDescriptorSetLayoutBinding& currBinding = layoutBindings[ ndx ];

                    currBinding.descriptorCount = currDesc.bindingCount;
                    currBinding.binding         = currDesc.baseBindingSlot;
                    currBinding.descriptorType  = MapBigosBindingTypeToVulkanDescriptorType( currDesc.type );
                    if( ( currDesc.type == BindingTypes::SAMPLER ) && ( currDesc.immutableSampler.phSamplers != nullptr ) )
                    {
                        // Creating immutable samplers
                        for( index_t ndy = 0; static_cast<uint32_t>( ndy ) < currDesc.bindingCount; ++ndy )
                        {
                            immutableSamplerRanges[ ndx ][ ndy ] = currDesc.immutableSampler.phSamplers[ ndy ].GetNativeHandle();
                        }

                        currBinding.pImmutableSamplers = immutableSamplerRanges[ ndx ];
                        currBinding.stageFlags         = MapBigosShaderVisibilityToVulkanShaderStageFlags( currDesc.immutableSampler.visibility );
                        immutableSamplerCnt += currDesc.bindingCount;
                        if( immutableSamplerCnt > Config::Driver::Pipeline::MAX_IMMUTABLE_SAMPLER_COUNT )
                        {
                            return Results::FAIL;
                        }
                    }
                    else
                    {
                        currBinding.pImmutableSamplers = nullptr;
                        currBinding.stageFlags         = MapBigosShaderVisibilityToVulkanShaderStageFlags( desc.visibility );
                    }
                }

                VkDescriptorSetLayoutCreateInfo layoutInfo;
                layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfo.flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
                layoutInfo.pNext        = nullptr;
                layoutInfo.pBindings    = layoutBindings;
                layoutInfo.bindingCount = desc.bindingRangeCount;

                if( vkCreateDescriptorSetLayout( nativeDevice, &layoutInfo, nullptr, &nativeLayout ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }

                *pHandle = BindingHeapLayoutHandle( nativeLayout );

                return Results::OK;
            }

            void VulkanDevice::DestroyBindingHeapLayout( BindingHeapLayoutHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Binding heap layout (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != BindingHeapLayoutHandle(), "Binding heap layout (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != BindingHeapLayoutHandle() ) )
                {
                    VkDevice              nativeDevice = m_handle.GetNativeHandle();
                    VkDescriptorSetLayout nativeLayout = pHandle->GetNativeHandle();

                    vkDestroyDescriptorSetLayout( nativeDevice, nativeLayout, nullptr );

                    *pHandle = BindingHeapLayoutHandle();
                }
            }

            RESULT VulkanDevice::CreateBindingHeap( const BindingHeapDesc& desc, BindingHeapHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Binding heap (pHandle) must be a valid address." );
                BGS_ASSERT( desc.bindingCount > 0, "Binding count (desc.bindingCount) must be more than 0." );
                if( ( pHandle == nullptr ) || ( desc.bindingCount < 0 ) )
                {
                    return Results::FAIL;
                }
                VulkanBindingHeap* pHeap = nullptr;
                if( BGS_FAILED( Core::Memory::AllocateObject( m_pParent->GetParent()->GetDefaultAllocator(), &pHeap ) ) )
                {
                    return Results::NO_MEMORY;
                }
                BGS_ASSERT( m_bindingSize );

                VkDevice       nativeDevice = m_handle.GetNativeHandle();
                VkBuffer       nativeHeap   = VK_NULL_HANDLE;
                VkDeviceMemory nativeMemory = VK_NULL_HANDLE;

                static const uint32_t           qFamNdx[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
                static const VkBufferUsageFlags baseUsage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

                VkBufferCreateInfo buffInfo;
                buffInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                buffInfo.pNext                 = nullptr;
                buffInfo.flags                 = 0;
                buffInfo.size                  = desc.bindingCount * m_bindingSize;
                buffInfo.usage                 = baseUsage | MapBigosBindingHeapTypeToVulkanBufferUsageFlags( desc.type );
                buffInfo.sharingMode           = VK_SHARING_MODE_CONCURRENT;
                buffInfo.queueFamilyIndexCount = static_cast<uint32_t>( m_queueProperties.size() );
                buffInfo.pQueueFamilyIndices   = qFamNdx;

                if( vkCreateBuffer( nativeDevice, &buffInfo, nullptr, &nativeHeap ) != VK_SUCCESS )
                {
                    Core::Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), &pHeap );
                    return Results::FAIL;
                }

                uint32_t memNdx = FindVulkanMemTypeNdx( &m_heapProperties.memoryProperties, MAX_UINT32, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );
                BGS_ASSERT( memNdx != MAX_UINT32 );
                if( memNdx == MAX_UINT32 )
                {
                    vkDestroyBuffer( nativeDevice, nativeHeap, nullptr );
                    Core::Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), &pHeap );
                    return Results::NO_MEMORY;
                }

                VkMemoryRequirements memRequirements;
                vkGetBufferMemoryRequirements( nativeDevice, nativeHeap, &memRequirements );

                VkMemoryAllocateFlagsInfo allocFlags;
                allocFlags.sType      = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
                allocFlags.pNext      = nullptr;
                allocFlags.deviceMask = 0;
                allocFlags.flags      = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

                VkMemoryAllocateInfo allocInfo;
                allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.pNext           = &allocFlags;
                allocInfo.memoryTypeIndex = memNdx;
                allocInfo.allocationSize  = memRequirements.size;

                if( vkAllocateMemory( nativeDevice, &allocInfo, nullptr, &nativeMemory ) != VK_SUCCESS )
                {
                    Core::Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), &pHeap );
                    vkDestroyBuffer( nativeDevice, nativeHeap, nullptr );
                    return Results::NO_MEMORY;
                }

                if( vkBindBufferMemory( nativeDevice, nativeHeap, nativeMemory, 0 ) != VK_SUCCESS )
                {
                    Core::Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), &pHeap );
                    vkFreeMemory( nativeDevice, nativeMemory, nullptr );
                    vkDestroyBuffer( nativeDevice, nativeHeap, nullptr );
                    return Results::FAIL;
                }

                VkBufferDeviceAddressInfo addressInfo;
                addressInfo.sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
                addressInfo.pNext  = nullptr;
                addressInfo.buffer = nativeHeap;

                pHeap->buffer  = nativeHeap;
                pHeap->memory  = nativeMemory;
                pHeap->address = vkGetBufferDeviceAddress( nativeDevice, &addressInfo );
                pHeap->flags   = buffInfo.usage;

                *pHandle = BindingHeapHandle( pHeap );

                return Results::OK;
            }

            void VulkanDevice::DestroyBindingHeap( BindingHeapHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Binding heap (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != BindingHeapHandle(), "Binding heap (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != BindingHeapHandle() ) )
                {
                    VulkanBindingHeap* pHeap        = pHandle->GetNativeHandle();
                    VkDevice           nativeDevice = m_handle.GetNativeHandle();

                    vkDestroyBuffer( nativeDevice, pHeap->buffer, nullptr );
                    vkFreeMemory( nativeDevice, pHeap->memory, nullptr );

                    Core::Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), &pHeap );

                    *pHandle = BindingHeapHandle();
                }
            }

            void VulkanDevice::CopyBinding( const CopyBindingDesc& desc ) { desc; }

            RESULT VulkanDevice::CreateQueryPool( const QueryPoolDesc& desc, QueryPoolHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Query pool (pHandle) must be a valid address." );
                BGS_ASSERT( desc.queryCount > 0, "Query count (desc.queryCount) must be more than 0." );
                if( ( pHandle == nullptr ) || ( desc.queryCount < 0 ) )
                {
                    return Results::FAIL;
                }

                VkDevice    nativeDevice = m_handle.GetNativeHandle();
                VkQueryPool nativePool   = VK_NULL_HANDLE;

                VkQueryPoolCreateInfo poolInfo;
                poolInfo.sType      = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
                poolInfo.pNext      = nullptr;
                poolInfo.flags      = 0;
                poolInfo.queryCount = desc.queryCount;
                poolInfo.queryType  = MapBigosQueryTypeToVulkanQueryType( desc.type );
                // Mimicing d3d12 behaviour
                poolInfo.pipelineStatistics =
                    VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT | VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
                    VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT | VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT |
                    VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT | VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT |
                    VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT | VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT |
                    VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT |
                    VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT |
                    VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;

                if( vkCreateQueryPool( nativeDevice, &poolInfo, nullptr, &nativePool ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }

                *pHandle = QueryPoolHandle( nativePool );

                return Results::OK;
            }

            void VulkanDevice::DestroyQueryPool( QueryPoolHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Query pool (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != QueryPoolHandle(), "Query pool (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != QueryPoolHandle() ) )
                {
                    VkDevice    nativeDevice = m_handle.GetNativeHandle();
                    VkQueryPool nativePool   = pHandle->GetNativeHandle();

                    vkDestroyQueryPool( nativeDevice, nativePool, nullptr );

                    *pHandle = QueryPoolHandle();
                }
            }

            RESULT VulkanDevice::Create( const DeviceDesc& desc, VulkanFactory* pFactory )
            {
                BGS_ASSERT( pFactory != nullptr, "Factory (pFactory) must be a valid pointer." );
                BGS_ASSERT( desc.pAdapter != nullptr, "Adapter (pAdapter) must be a valid pointer." );
                if( ( pFactory == nullptr ) || ( desc.pAdapter == nullptr ) )
                {
                    return Results::FAIL;
                }

                m_desc    = desc;
                m_pParent = pFactory;

                if( BGS_FAILED( CreateVkDevice() ) )
                {
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void VulkanDevice::Destroy()
            {
                BGS_ASSERT( m_handle != DeviceHandle() );

                if( m_handle != DeviceHandle() )
                {
                    VkDevice nativeDevice = m_handle.GetNativeHandle();
                    vkDestroyDevice( nativeDevice, nullptr );
                    // m_deviceAPI.vkDestroyDevice( nativeDevice, nullptr );
                }

                for( index_t ndx = 0; ndx < m_queueProperties.size(); ++ndx )
                {
                    m_queueProperties[ ndx ].queueParams.clear();
                }
                m_queueProperties.clear();

                m_pParent = nullptr;
                m_handle  = DeviceHandle();
            }

            RESULT VulkanDevice::CreateVkDevice()
            {
                VkPhysicalDevice       nativeAdapter = m_desc.pAdapter->GetHandle().GetNativeHandle();
                VkDevice               nativeDevice  = VK_NULL_HANDLE;
                VkDeviceFeaturesHelper deviceFeaturesHelper( m_desc );

                EnumerateNativeQueues();
                HeapArray<VkDeviceQueueCreateInfo> queueCreateInfos( m_queueProperties.size() );
                static float                       queuePrios[ 4 ][ 64 ];

                for( index_t ndx = 0; ndx < m_queueProperties.size(); ++ndx )
                {
                    for( index_t ndy = 0; ndy < m_queueProperties[ ndx ].queueParams.size(); ++ndy )
                    {
                        queuePrios[ ndx ][ ndy ] = MapBigosQueuePriorityToVulkanQueuePriority( m_queueProperties[ ndx ].queueParams[ ndy ].priority );
                    }

                    queueCreateInfos[ ndx ].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    queueCreateInfos[ ndx ].pNext            = nullptr;
                    queueCreateInfos[ ndx ].flags            = 0;
                    queueCreateInfos[ ndx ].queueFamilyIndex = static_cast<uint32_t>( ndx );
                    queueCreateInfos[ ndx ].queueCount       = static_cast<uint32_t>( m_queueProperties[ ndx ].queueParams.size() );
                    queueCreateInfos[ ndx ].pQueuePriorities = queuePrios[ ndx ];
                }

                const char* const* ppQueriedExt    = deviceFeaturesHelper.GetExtNames();
                const uint32_t     queriedExtCount = deviceFeaturesHelper.GetExtCount();

                if( BGS_FAILED( CheckVkExtensionSupport( queriedExtCount, ppQueriedExt ) ) )
                {
                    return Results::NOT_FOUND;
                }

                VkDeviceCreateInfo devInfo;
                devInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
                devInfo.pNext                   = deviceFeaturesHelper.GetExtChain();
                devInfo.flags                   = 0;
                devInfo.enabledExtensionCount   = queriedExtCount;
                devInfo.ppEnabledExtensionNames = ppQueriedExt;
                devInfo.pEnabledFeatures        = deviceFeaturesHelper.GetDeviceFeatures();
                devInfo.pQueueCreateInfos       = queueCreateInfos.data();
                devInfo.queueCreateInfoCount    = static_cast<uint32_t>( queueCreateInfos.size() );
                devInfo.ppEnabledLayerNames     = nullptr;
                devInfo.enabledLayerCount       = 0;

                if( vkCreateDevice( nativeAdapter, &devInfo, nullptr, &nativeDevice ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }
                // TODO: Change to volkLOadDeviceTable() not working right now (24.07.2023)
                // volkLoadDeviceTable( &m_deviceAPI, nativeDevice );
                volkLoadDevice( nativeDevice );

                // Geting needed internals
                m_heapProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
                m_heapProperties.pNext = nullptr;
                vkGetPhysicalDeviceMemoryProperties2( nativeAdapter, &m_heapProperties );

                QueryVkBindingsSize();

                m_handle = DeviceHandle( nativeDevice );

                return Results::OK;
            }

            RESULT VulkanDevice::CreateVkGraphicsPipeline( const GraphicsPipelineDesc& gpDesc, VkPipeline* pNativePipeline )
            {
                BGS_ASSERT( gpDesc.hPipelineLayout != PipelineLayoutHandle(), "Pipeline layout (gpDesc.hPipelineLayout) must be valid handle." );
                if( ( gpDesc.hPipelineLayout == PipelineLayoutHandle() ) )
                {
                    return Results::FAIL;
                }

                // Shader stages
                VkPipelineShaderStageCreateInfo shaderStages[ 5 ];
                uint32_t                        stageCnt = 0;
                if( gpDesc.vertexShader.hShader != ShaderHandle() )
                {
                    shaderStages[ stageCnt++ ] = CreateShaderStage( gpDesc.vertexShader, VK_SHADER_STAGE_VERTEX_BIT );
                }
                if( gpDesc.pixelShader.hShader != ShaderHandle() )
                {
                    shaderStages[ stageCnt++ ] = CreateShaderStage( gpDesc.pixelShader, VK_SHADER_STAGE_FRAGMENT_BIT );
                }
                if( gpDesc.domainShader.hShader != ShaderHandle() )
                {
                    shaderStages[ stageCnt++ ] = CreateShaderStage( gpDesc.domainShader, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT );
                }
                if( gpDesc.hullShader.hShader != ShaderHandle() )
                {
                    shaderStages[ stageCnt++ ] = CreateShaderStage( gpDesc.hullShader, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT );
                }
                if( gpDesc.geometryShader.hShader != ShaderHandle() )
                {
                    shaderStages[ stageCnt++ ] = CreateShaderStage( gpDesc.geometryShader, VK_SHADER_STAGE_GEOMETRY_BIT );
                }

                // Vertex input state desc
                BGS_ASSERT( gpDesc.inputState.inputBindingCount <= Config::Driver::Pipeline::MAX_INPUT_BINDING_COUNT );
                if( gpDesc.inputState.inputBindingCount > Config::Driver::Pipeline::MAX_INPUT_BINDING_COUNT )

                {
                    return Results::FAIL;
                }
                VkVertexInputBindingDescription bindings[ Config::Driver::Pipeline::MAX_INPUT_BINDING_COUNT ];
                for( index_t ndx = 0; ndx < static_cast<index_t>( gpDesc.inputState.inputBindingCount ); ++ndx )
                {
                    const InputBindingDesc&          currDesc = gpDesc.inputState.pInputBindings[ ndx ];
                    VkVertexInputBindingDescription& currBind = bindings[ ndx ];
                    currBind.binding                          = currDesc.binding;
                    currBind.stride                           = MAX_UINT32; // Will be ignored because of dynamic state flag
                    currBind.inputRate                        = MapBigosInputStepRateToVulkanVertexInputStepRate( currDesc.inputRate );
                }

                BGS_ASSERT( gpDesc.inputState.inputElementCount <= Config::Driver::Pipeline::MAX_INPUT_ELEMENT_COUNT );
                if( gpDesc.inputState.inputElementCount > Config::Driver::Pipeline::MAX_INPUT_ELEMENT_COUNT )
                {
                    return Results::FAIL;
                }
                VkVertexInputAttributeDescription attribs[ Config::Driver::Pipeline::MAX_INPUT_ELEMENT_COUNT ];
                for( index_t ndx = 0; ndx < static_cast<index_t>( gpDesc.inputState.inputElementCount ); ++ndx )
                {
                    const InputElementDesc&            currDesc = gpDesc.inputState.pInputElements[ ndx ];
                    VkVertexInputAttributeDescription& currAttr = attribs[ ndx ];
                    currAttr.binding                            = currDesc.binding;
                    currAttr.format                             = MapBigosFormatToVulkanFormat( currDesc.format );
                    currAttr.location                           = currDesc.location;
                    currAttr.offset                             = currDesc.offset;
                }

                VkPipelineVertexInputStateCreateInfo viInfo;
                viInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                viInfo.pNext                           = nullptr;
                viInfo.flags                           = 0;
                viInfo.vertexBindingDescriptionCount   = gpDesc.inputState.inputBindingCount;
                viInfo.pVertexBindingDescriptions      = bindings;
                viInfo.vertexAttributeDescriptionCount = gpDesc.inputState.inputElementCount;
                viInfo.pVertexAttributeDescriptions    = attribs;

                // Input assembler
                VkPipelineInputAssemblyStateCreateInfo inputAssembly;
                inputAssembly.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                inputAssembly.pNext    = nullptr;
                inputAssembly.flags    = 0;
                inputAssembly.topology = MapBigosPrimitiveTopologyToVulkanPrimitiveTopology(
                    gpDesc.inputAssemblerState.topology ); // Will be ignored because of dynamic state flag
                inputAssembly.primitiveRestartEnable = MapBigosIndexRestartValueToVulkanBool( gpDesc.inputAssemblerState.indexRestartValue );

                // TODO: Handle tesselation state

                // Rasterizer state
                VkPipelineRasterizationStateCreateInfo rasterInfo;
                rasterInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                rasterInfo.pNext                   = nullptr;
                rasterInfo.flags                   = 0;
                rasterInfo.rasterizerDiscardEnable = VK_TRUE; // Required for viewport dynamic state with prerasterization shader active
                rasterInfo.depthClampEnable        = gpDesc.rasterizeState.depthClipEnable;
                rasterInfo.polygonMode             = MapBigosPolygonFillModeToVulkanPolygonMode( gpDesc.rasterizeState.fillMode );
                rasterInfo.cullMode                = MapBigosCullModeToVulkanCullModeFlags( gpDesc.rasterizeState.cullMode );
                rasterInfo.frontFace               = MapBigosFrontFaceModeToVulkanFrontFace( gpDesc.rasterizeState.frontFaceMode );
                rasterInfo.depthBiasEnable         = VK_TRUE;
                rasterInfo.depthBiasConstantFactor = gpDesc.rasterizeState.depthBiasConstantFactor;
                rasterInfo.depthBiasSlopeFactor    = gpDesc.rasterizeState.depthBiasSlopeFactor;
                rasterInfo.depthBiasClamp          = gpDesc.rasterizeState.depthBiasClamp;
                rasterInfo.lineWidth               = 1.0f; // To mimic D3D12 behaviour

                // Multisample state
                VkPipelineMultisampleStateCreateInfo multisampleInfo;
                multisampleInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                multisampleInfo.pNext                 = nullptr;
                multisampleInfo.flags                 = 0;
                multisampleInfo.rasterizationSamples  = MapBigosSampleCountToVulkanSampleCountFlags( gpDesc.multisampleState.sampleCount );
                multisampleInfo.sampleShadingEnable   = VK_FALSE; // Potentially need to handle all above
                multisampleInfo.minSampleShading      = 0.0f;
                multisampleInfo.pSampleMask           = nullptr;
                multisampleInfo.alphaToOneEnable      = VK_FALSE;
                multisampleInfo.alphaToCoverageEnable = VK_FALSE;

                // Depth stencil state
                VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
                depthStencilInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                depthStencilInfo.pNext             = nullptr;
                depthStencilInfo.flags             = 0;
                depthStencilInfo.depthTestEnable   = gpDesc.depthStencilState.depthTestEnable;
                depthStencilInfo.depthWriteEnable  = gpDesc.depthStencilState.depthWriteEnable;
                depthStencilInfo.depthCompareOp    = MapBigosCompareOperationTypeToVulkanCompareOp( gpDesc.depthStencilState.depthCompare );
                depthStencilInfo.stencilTestEnable = gpDesc.depthStencilState.stencilTestEnable;
                depthStencilInfo.front.failOp      = MapBigosStencilOperationTypeToVulkanStencilOp( gpDesc.depthStencilState.frontFace.failOp );
                depthStencilInfo.front.depthFailOp = MapBigosStencilOperationTypeToVulkanStencilOp( gpDesc.depthStencilState.frontFace.depthFailOp );
                depthStencilInfo.front.passOp      = MapBigosStencilOperationTypeToVulkanStencilOp( gpDesc.depthStencilState.frontFace.passOp );
                depthStencilInfo.front.compareOp   = MapBigosCompareOperationTypeToVulkanCompareOp( gpDesc.depthStencilState.frontFace.compareOp );
                depthStencilInfo.front.compareMask = gpDesc.depthStencilState.stencilReadMask;
                depthStencilInfo.front.writeMask   = gpDesc.depthStencilState.stencilWriteMask;
                // Stencil reference is ignored because of dynamic states
                depthStencilInfo.back.failOp      = MapBigosStencilOperationTypeToVulkanStencilOp( gpDesc.depthStencilState.backFace.failOp );
                depthStencilInfo.back.depthFailOp = MapBigosStencilOperationTypeToVulkanStencilOp( gpDesc.depthStencilState.backFace.depthFailOp );
                depthStencilInfo.back.passOp      = MapBigosStencilOperationTypeToVulkanStencilOp( gpDesc.depthStencilState.backFace.passOp );
                depthStencilInfo.back.compareOp   = MapBigosCompareOperationTypeToVulkanCompareOp( gpDesc.depthStencilState.backFace.compareOp );
                depthStencilInfo.back.compareMask = gpDesc.depthStencilState.stencilReadMask;
                depthStencilInfo.back.writeMask   = gpDesc.depthStencilState.stencilWriteMask;
                // Stencil reference is ignored because of dynamic states
                depthStencilInfo.maxDepthBounds = 0.0f; // Ignored because of dynamic states
                depthStencilInfo.minDepthBounds = 0.0f; // Ignored because of dynamic states

                // Blend state
                BGS_ASSERT( gpDesc.blendState.renderTargetBlendDescCount <= Config::Driver::Pipeline::MAX_BLEND_STATE_COUNT );
                if( gpDesc.blendState.renderTargetBlendDescCount > Config::Driver::Pipeline::MAX_BLEND_STATE_COUNT )
                {
                    return Results::FAIL;
                }

                VkPipelineColorBlendAttachmentState nativeRTBlends[ Config::Driver::Pipeline::MAX_BLEND_STATE_COUNT ];
                for( index_t ndx = 0; ndx < static_cast<index_t>( gpDesc.blendState.renderTargetBlendDescCount ); ++ndx )
                {
                    const RenderTargetBlendDesc&         currDesc   = gpDesc.blendState.pRenderTargetBlendDescs[ ndx ];
                    VkPipelineColorBlendAttachmentState& currAttach = nativeRTBlends[ ndx ];

                    currAttach.blendEnable         = currDesc.blendEnable;
                    currAttach.srcColorBlendFactor = MapBigosBlendFactorToVulkanBlendFactor( currDesc.srcColorBlendFactor );
                    currAttach.dstColorBlendFactor = MapBigosBlendFactorToVulkanBlendFactor( currDesc.dstColorBlendFactor );
                    currAttach.colorBlendOp        = MapBigosBlendOperationTypeToVulkanBlendOp( currDesc.colorBlendOp );
                    currAttach.srcAlphaBlendFactor = MapBigosBlendFactorToVulkanBlendFactor( currDesc.srcAlphaBlendFactor );
                    currAttach.dstAlphaBlendFactor = MapBigosBlendFactorToVulkanBlendFactor( currDesc.dstAlphaBlendFactor );
                    currAttach.alphaBlendOp        = MapBigosBlendOperationTypeToVulkanBlendOp( currDesc.alphaBlendOp );
                    currAttach.colorWriteMask      = static_cast<uint32_t>( currDesc.writeFlag );
                }

                VkPipelineColorBlendStateCreateInfo blendState;
                blendState.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                blendState.pNext           = nullptr;
                blendState.flags           = 0;
                blendState.logicOpEnable   = gpDesc.blendState.enableLogicOperations;
                blendState.logicOp         = MapBigosLogicOperationTypeToVulkanLogicOp( gpDesc.blendState.logicOperation );
                blendState.attachmentCount = gpDesc.blendState.renderTargetBlendDescCount;
                blendState.pAttachments    = nativeRTBlends;
                // Blend constants are ignored because of dynamic states

                // Dynamic states - set to mimic D3D12 behaviour
                VkDynamicState dynamicStates[] = {
                    VK_DYNAMIC_STATE_VIEWPORT,     VK_DYNAMIC_STATE_SCISSOR,           VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE,
                    VK_DYNAMIC_STATE_DEPTH_BOUNDS, VK_DYNAMIC_STATE_STENCIL_REFERENCE, VK_DYNAMIC_STATE_BLEND_CONSTANTS,
                };

                VkPipelineDynamicStateCreateInfo dynamicState;
                dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                dynamicState.pNext             = nullptr;
                dynamicState.flags             = 0;
                dynamicState.dynamicStateCount = __crt_countof( dynamicStates );
                dynamicState.pDynamicStates    = dynamicStates;

                // Render target formats
                BGS_ASSERT( gpDesc.renderTargetCount <= Config::Driver::Pipeline::MAX_RENDER_TARGET_COUNT );
                if( gpDesc.renderTargetCount > Config::Driver::Pipeline::MAX_RENDER_TARGET_COUNT )
                {
                    return Results::FAIL;
                }

                VkFormat rtFormats[ Config::Driver::Pipeline::MAX_RENDER_TARGET_COUNT ];
                for( index_t ndx = 0; ndx < static_cast<index_t>( gpDesc.renderTargetCount ); ++ndx )
                {
                    rtFormats[ ndx ] = MapBigosFormatToVulkanFormat( gpDesc.pRenderTargetFormats[ ndx ] );
                }

                VkPipelineRenderingCreateInfo dynamicInfo;
                dynamicInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
                dynamicInfo.pNext                   = nullptr;
                dynamicInfo.viewMask                = 0;
                dynamicInfo.colorAttachmentCount    = gpDesc.renderTargetCount;
                dynamicInfo.pColorAttachmentFormats = rtFormats;
                dynamicInfo.depthAttachmentFormat   = MapBigosFormatToVulkanFormat( gpDesc.depthStencilFormat );
                dynamicInfo.stencilAttachmentFormat = MapBigosFormatToVulkanFormat( gpDesc.depthStencilFormat );

                VkGraphicsPipelineCreateInfo pipelineInfo;
                pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                pipelineInfo.pNext               = &dynamicInfo;
                pipelineInfo.flags               = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
                pipelineInfo.pStages             = shaderStages;
                pipelineInfo.stageCount          = stageCnt;
                pipelineInfo.pVertexInputState   = &viInfo;
                pipelineInfo.pInputAssemblyState = &inputAssembly;
                pipelineInfo.pTessellationState  = nullptr;
                pipelineInfo.pViewportState      = nullptr;
                pipelineInfo.pRasterizationState = &rasterInfo;
                pipelineInfo.pMultisampleState   = &multisampleInfo;
                pipelineInfo.pDepthStencilState  = &depthStencilInfo;
                pipelineInfo.pColorBlendState    = &blendState;
                pipelineInfo.pDynamicState       = &dynamicState;
                pipelineInfo.layout              = gpDesc.hPipelineLayout.GetNativeHandle();
                pipelineInfo.renderPass          = VK_NULL_HANDLE; // We do support only dynamic rendering for now
                pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
                pipelineInfo.basePipelineIndex   = 0;
                pipelineInfo.subpass             = 0;

                VkDevice   nativeDevice   = m_handle.GetNativeHandle();
                VkPipeline nativePipeline = VK_NULL_HANDLE;

                // TODO: Handle cached pso
                if( vkCreateGraphicsPipelines( nativeDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &nativePipeline ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }

                *pNativePipeline = nativePipeline;

                return Results::OK;
            }

            RESULT VulkanDevice::CreateVkBuffer( const ResourceDesc desc, VkBuffer* pBuff )
            {
                VkDevice nativeDevice = m_handle.GetNativeHandle();

                static const uint32_t qFamNdx[ 4 ] = { 0, 1, 2, 3 };

                VkBufferCreateInfo buffInfo;
                buffInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                buffInfo.pNext                 = nullptr;
                buffInfo.flags                 = 0; // TODO: Handle
                buffInfo.size                  = desc.size.width;
                buffInfo.usage                 = MapBigosResourceUsageToVulkanBufferUsageFlags( desc.resourceUsage );
                buffInfo.sharingMode           = MapBigosResourceSharingModeToVulkanSharingMode( desc.sharingMode );
                buffInfo.queueFamilyIndexCount = static_cast<uint32_t>( m_queueProperties.size() );
                buffInfo.pQueueFamilyIndices   = qFamNdx;

                if( vkCreateBuffer( nativeDevice, &buffInfo, nullptr, pBuff ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }

                return Results::OK;
            }

            RESULT VulkanDevice::CreateVkImage( const ResourceDesc desc, VkImage* pImg )
            {
                VkDevice nativeDevice = m_handle.GetNativeHandle();

                static const uint32_t qFamNdx[ 4 ] = { 0, 1, 2, 3 };

                VkImageCreateInfo imgInfo;
                imgInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                imgInfo.pNext                 = nullptr;
                imgInfo.flags                 = 0; // TODO: Handle
                imgInfo.imageType             = MapBigosResourceTypeToVulkanImageType( desc.resourceType );
                imgInfo.format                = MapBigosFormatToVulkanFormat( desc.format );
                imgInfo.extent.width          = desc.size.width;
                imgInfo.extent.height         = desc.size.height;
                imgInfo.extent.depth          = desc.size.depth;
                imgInfo.mipLevels             = desc.mipLevelCount;
                imgInfo.arrayLayers           = desc.arrayLayerCount;
                imgInfo.samples               = MapBigosSampleCountToVulkanSampleCountFlags( desc.sampleCount );
                imgInfo.tiling                = MapBigosResourceLayoutToVulkanImageTiling( desc.resourceLayout );
                imgInfo.usage                 = MapBigosResourceUsageFlagsToVulkanImageUsageFlags( desc.resourceUsage );
                imgInfo.queueFamilyIndexCount = static_cast<uint32_t>( m_queueProperties.size() );
                imgInfo.pQueueFamilyIndices   = qFamNdx;

                if( vkCreateImage( nativeDevice, &imgInfo, nullptr, pImg ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void VulkanDevice::EnumerateNativeQueues()
            {
                VkPhysicalDevice nativeAdapter = m_desc.pAdapter->GetHandle().GetNativeHandle();

                uint32_t familyCount = 0;
                vkGetPhysicalDeviceQueueFamilyProperties( nativeAdapter, &familyCount, nullptr );
                VkQueueFamilyProperties nativeProps[ 4 ];
                vkGetPhysicalDeviceQueueFamilyProperties( nativeAdapter, &familyCount, nativeProps );
                m_queueProperties.resize( familyCount );

                for( index_t ndx = 0; ndx < familyCount; ++ndx )
                {
                    const uint32_t qCnt = nativeProps[ ndx ].queueCount;
                    m_queueProperties[ ndx ].queueParams.reserve( qCnt );
                    m_queueProperties[ ndx ].familiyParams = nativeProps[ ndx ];

                    // Decide about queue count
                    if( qCnt == 1 )
                    {
                        VulkanQueueParams params;
                        params.familyIndex = static_cast<uint32_t>( ndx );
                        params.queueIndex  = 0;
                        params.priority    = QueuePriorityTypes::NORMAL;
                        params.free        = BGS_TRUE;

                        m_queueProperties[ ndx ].queueParams.push_back( params );
                    }
                    else if( qCnt == 2 )
                    {
                        VulkanQueueParams params;
                        params.familyIndex = static_cast<uint32_t>( ndx );
                        params.queueIndex  = 0;
                        params.priority    = QueuePriorityTypes::NORMAL;
                        params.free        = BGS_TRUE;
                        m_queueProperties[ ndx ].queueParams.push_back( params );

                        params.queueIndex = 1;
                        params.priority   = QueuePriorityTypes::HIGH;
                        m_queueProperties[ ndx ].queueParams.push_back( params );
                    }
                    else if( qCnt == 3 )
                    {
                        VulkanQueueParams params;
                        params.familyIndex = static_cast<uint32_t>( ndx );
                        params.queueIndex  = 0;
                        params.priority    = QueuePriorityTypes::NORMAL;
                        params.free        = BGS_TRUE;
                        m_queueProperties[ ndx ].queueParams.push_back( params );

                        params.queueIndex = 1;
                        params.priority   = QueuePriorityTypes::HIGH;
                        m_queueProperties[ ndx ].queueParams.push_back( params );

                        params.queueIndex = 2;
                        params.priority   = QueuePriorityTypes::REALTIME;
                        m_queueProperties[ ndx ].queueParams.push_back( params );
                    }
                    else // 0 or more than 3
                    {
                        VulkanQueueParams params;
                        params.familyIndex = static_cast<uint32_t>( ndx );
                        params.free        = BGS_TRUE;

                        for( index_t ndy = 0; ndy < qCnt / 2; ++ndy )
                        {
                            params.priority   = QueuePriorityTypes::NORMAL;
                            params.queueIndex = static_cast<uint32_t>( ndy );

                            m_queueProperties[ ndx ].queueParams.push_back( params );
                        }

                        for( index_t ndy = qCnt / 2; ndy < qCnt - 1; ++ndy )
                        {
                            params.priority   = QueuePriorityTypes::HIGH;
                            params.queueIndex = static_cast<uint32_t>( ndy );

                            m_queueProperties[ ndx ].queueParams.push_back( params );
                        }

                        params.priority   = QueuePriorityTypes::REALTIME;
                        params.queueIndex = static_cast<uint32_t>( qCnt - 1 );
                        m_queueProperties[ ndx ].queueParams.push_back( params );
                    }
                }
            }

            void VulkanDevice::QueryVkBindingsSize()
            {
                VkPhysicalDevice                              nativeAdapter = m_desc.pAdapter->GetHandle().GetNativeHandle();
                VkPhysicalDeviceDescriptorBufferPropertiesEXT descBufferProps;
                descBufferProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT;
                descBufferProps.pNext = nullptr;
                VkPhysicalDeviceProperties2 nativeProps2;
                nativeProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
                nativeProps2.pNext = &descBufferProps;
                vkGetPhysicalDeviceProperties2( nativeAdapter, &nativeProps2 );
                m_limits.samplerBindingSize                = descBufferProps.samplerDescriptorSize;
                m_limits.sampledTextureBindingSize         = descBufferProps.sampledImageDescriptorSize;
                m_limits.storageTextureBindingSize         = descBufferProps.storageImageDescriptorSize;
                m_limits.constantTexelBufferBindingSize    = descBufferProps.uniformTexelBufferDescriptorSize;
                m_limits.storageTexelBufferBindingSize     = descBufferProps.storageTexelBufferDescriptorSize;
                m_limits.constantBufferBindingSize         = descBufferProps.uniformBufferDescriptorSize;
                m_limits.readOnlyStorageBufferBindingSize  = descBufferProps.storageBufferDescriptorSize;
                m_limits.readWriteStorageBufferBindingSize = descBufferProps.storageBufferDescriptorSize;

                if( m_limits.samplerBindingSize > m_bindingSize )
                {
                    m_bindingSize = m_limits.samplerBindingSize;
                }
                if( m_limits.sampledTextureBindingSize > m_bindingSize )
                {
                    m_bindingSize = m_limits.sampledTextureBindingSize;
                }
                if( m_limits.storageTextureBindingSize > m_bindingSize )
                {
                    m_bindingSize = m_limits.storageTextureBindingSize;
                }
                if( m_limits.constantTexelBufferBindingSize > m_bindingSize )
                {
                    m_bindingSize = m_limits.constantTexelBufferBindingSize;
                }
                if( m_limits.storageTexelBufferBindingSize > m_bindingSize )
                {
                    m_bindingSize = m_limits.storageTexelBufferBindingSize;
                }
                if( m_limits.constantBufferBindingSize > m_bindingSize )
                {
                    m_bindingSize = m_limits.constantBufferBindingSize;
                }
                if( m_limits.readOnlyStorageBufferBindingSize > m_bindingSize )
                {
                    m_bindingSize = m_limits.readOnlyStorageBufferBindingSize;
                }
                if( m_limits.readWriteStorageBufferBindingSize > m_bindingSize )
                {
                    m_bindingSize = m_limits.readWriteStorageBufferBindingSize;
                }
            }

            RESULT VulkanDevice::CheckVkExtensionSupport( uint32_t extCount, const char* const* ppQueriedExts )
            {
                uint32_t         availExtCount = 0;
                VkPhysicalDevice nativeAdapter = m_desc.pAdapter->GetHandle().GetNativeHandle();

                if( vkEnumerateDeviceExtensionProperties( nativeAdapter, nullptr, &availExtCount, nullptr ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }

                HeapArray<VkExtensionProperties> extProps( availExtCount );
                if( vkEnumerateDeviceExtensionProperties( nativeAdapter, nullptr, &availExtCount, extProps.data() ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }

                HeapArray<const char*> availExts( availExtCount );
                for( index_t ndx = 0; ndx < availExtCount; ++ndx )
                {
                    availExts[ ndx ] = extProps[ ndx ].extensionName;
                }

                for( index_t ndx = 0; ndx < extCount; ++ndx )
                {
                    if( Core::Utils::String::FindString( availExts, ppQueriedExts[ ndx ] ) == INVALID_POSITION )
                    {
                        // TODO: Log not found, then return not found.
                        return Results::NOT_FOUND;
                    }
                }

                return Results::OK;
            }

            RESULT VulkanDevice::FindSuitableQueue( QUEUE_TYPE type, QUEUE_PRIORITY_TYPE prio, uint32_t* familyIndex, uint32_t* queueIndex )
            {
                // Due to hw specification best way is to start looking for proper queue in reverse family order.
                const index_t famCnt = m_queueProperties.size();
                for( index_t ndx = 0; ndx < famCnt; ++ndx )
                {
                    const index_t revFamNdx = famCnt - 1 - ndx;
                    for( index_t ndy = 0; ndy < m_queueProperties[ revFamNdx ].queueParams.size(); ++ndy )
                    {
                        VulkanQueueParams& qParams = m_queueProperties[ revFamNdx ].queueParams[ ndy ];
                        // TODO: Video here

                        // Looking for copy queue
                        if( ( type == QueueTypes::COPY ) && ( m_queueProperties[ revFamNdx ].familiyParams.queueFlags & VK_QUEUE_TRANSFER_BIT ) &&
                            ( prio == qParams.priority ) && ( qParams.free == BGS_TRUE ) )
                        {
                            qParams.free = BGS_FALSE;
                            *familyIndex = static_cast<uint32_t>( revFamNdx );
                            *queueIndex  = static_cast<uint32_t>( ndy );

                            return Results::OK;
                        }

                        // Looking for compute queue
                        if( ( type == QueueTypes::COMPUTE ) && ( m_queueProperties[ revFamNdx ].familiyParams.queueFlags & VK_QUEUE_COMPUTE_BIT ) &&
                            ( prio == qParams.priority ) && ( qParams.free == BGS_TRUE ) )
                        {
                            qParams.free = BGS_FALSE;
                            *familyIndex = static_cast<uint32_t>( revFamNdx );
                            *queueIndex  = static_cast<uint32_t>( ndy );

                            return Results::OK;
                        }

                        // Looking for graphics queue
                        if( ( type == QueueTypes::GRAPHICS ) && ( m_queueProperties[ revFamNdx ].familiyParams.queueFlags & VK_QUEUE_GRAPHICS_BIT ) &&
                            ( prio == qParams.priority ) && ( qParams.free == BGS_TRUE ) )
                        {
                            qParams.free = BGS_FALSE;
                            *familyIndex = static_cast<uint32_t>( revFamNdx );
                            *queueIndex  = static_cast<uint32_t>( ndy );

                            return Results::OK;
                        }
                    }
                }

                *familyIndex = MAX_UINT32;
                *queueIndex  = MAX_UINT32;

                return Results::NOT_FOUND;
            }

            void VulkanDevice::FreeNativeQueue( uint32_t familyIndex, uint32_t queueIndex )
            {
                BGS_ASSERT( familyIndex < m_queueProperties.size(), "Family index extends count of native queue families." )
                BGS_ASSERT( queueIndex < m_queueProperties[ familyIndex ].queueParams.size(),
                            "Queue index extends count of native queues in this family." )
                if( ( familyIndex >= m_queueProperties.size() ) || ( queueIndex >= m_queueProperties[ familyIndex ].queueParams.size() ) )
                {
                    return;
                }

                m_queueProperties[ familyIndex ].queueParams[ queueIndex ].free = true;
            }

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS