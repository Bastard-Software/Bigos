#define BGS_USE_VULKAN_HANDLES 1

#include "VulkanDevice.h"

#include "BigosFramework/Config.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Memory.h"
#include "Core/Utils/String.h"
#include "Driver/Frontend/RenderSystem.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommon.h"
#include "VulkanFactory.h"
#include "VulkanQueue.h"

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
                    m_dev12Features.sType             = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
                    m_dev12Features.pNext             = &m_dev13Features;
                    m_dev12Features.drawIndirectCount = VK_TRUE;
                    // m_dev12Features.samplerMirrorClampToEdge = VK_TRUE;
                    m_dev12Features.timelineSemaphore = VK_TRUE; // Crucial for synchronization

                    // Vulkan 1.3 features
                    m_dev13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
                    // m_dev13Features.pNext = next p chain
                    m_dev13Features.synchronization2 = VK_TRUE; // Crucial for synchronization

                    m_pNext = &m_dev11Features;

                    // Extensions
                    m_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
                    // TODO: Add logic
                }

                VkDeviceFeaturesHelper() = default;

                void*                     GetExtChain() const { return m_pNext; }
                VkPhysicalDeviceFeatures* GetDeviceFeatures() { return &m_devCoreFeatures; }
                const char* const*        GetExtNames() const { return m_extensions.data(); }
                uint32_t                  GetExtCount() const { return static_cast<uint32_t>( m_extensions.size() ); }

            private:
                DeviceDesc m_devDesc;

                VkPhysicalDeviceFeatures         m_devCoreFeatures;
                VkPhysicalDeviceVulkan11Features m_dev11Features;
                VkPhysicalDeviceVulkan12Features m_dev12Features;
                VkPhysicalDeviceVulkan13Features m_dev13Features;

                HeapArray<const char*> m_extensions;

                void* m_pNext;
            };

            RESULT VulkanDevice::CreateQueue( const QueueDesc& desc, IQueue** ppQueue )
            {
                BGS_ASSERT( ppQueue != nullptr, "Queue (ppQueue) must be a valid address." );
                BGS_ASSERT( *ppQueue == nullptr, "There is a pointer at the given address. Queue (*ppQueue) must be nullptr." );
                if( ( ppQueue == nullptr ) && ( *ppQueue != nullptr ) )
                {
                    return Results::FAIL;
                }

                VulkanQueue* pQueue = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( m_pParent->GetParentPtr()->GetDefaultAllocatorPtr(), &pQueue ) ) )
                {
                    return Results::NO_MEMORY;
                }
                BGS_ASSERT( pQueue != nullptr );

                if( BGS_FAILED( pQueue->Create( desc, this ) ) )
                {
                    Memory::FreeObject( m_pParent->GetParentPtr()->GetDefaultAllocatorPtr(), &pQueue );
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
                    Memory::FreeObject( m_pParent->GetParentPtr()->GetDefaultAllocatorPtr(), ppQueue );
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
                if( BGS_FAILED( Memory::AllocateObject( m_pParent->GetParentPtr()->GetDefaultAllocatorPtr(), &pCommandBuffer ) ) )
                {
                    return Results::NO_MEMORY;
                }
                BGS_ASSERT( pCommandBuffer != nullptr );

                if( BGS_FAILED( pCommandBuffer->Create( desc, this ) ) )
                {
                    Memory::FreeObject( m_pParent->GetParentPtr()->GetDefaultAllocatorPtr(), &pCommandBuffer );
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
                    Memory::FreeObject( m_pParent->GetParentPtr()->GetDefaultAllocatorPtr(), ppCommandBuffer );
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
                BGS_ASSERT( desc.pWaitValues != nullptr, "Uint array (desc.pWaitValues) must be a valid pointer." );
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
                BGS_ASSERT( pHandle != nullptr, "Semaphore handle (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != SemaphoreHandle(), "Semaphore handle (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != SemaphoreHandle() ) )
                {
                    VkDevice    nativeDevice    = m_handle.GetNativeHandle();
                    VkSemaphore nativeSemaphore = pHandle->GetNativeHandle();

                    vkDestroySemaphore( nativeDevice, nativeSemaphore, nullptr );

                    *pHandle = SemaphoreHandle( nativeSemaphore );
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

                m_handle = DeviceHandle( nativeDevice );

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