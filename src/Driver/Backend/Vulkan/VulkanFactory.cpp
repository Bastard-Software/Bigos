#define BGS_USE_VULKAN_HANDLES 1

#include "VulkanFactory.h"

#include "Core/CoreTypes.h"

#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Memory.h"
#include "Core/Utils/String.h"
#include "Driver/Frontend/RenderSystem.h"
#include "VulkanAdapter.h"
#include "VulkanDevice.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {

#if( BGS_RENDER_DEBUG )
            VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT,
                                                          const VkDebugUtilsMessengerCallbackDataEXT*, void* );
#endif // ( BGS_RENDER_DEBUG )

            RESULT VulkanFactory::CreateDevice( const DeviceDesc& desc, IDevice** ppDevice )
            {
                BGS_ASSERT( ( ppDevice != nullptr ) && ( *ppDevice == nullptr ) );

                VulkanDevice* pDevice = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( m_pParent->GetDefaultAllocator(), &pDevice ) ) )
                {
                    return Results::NO_MEMORY;
                }
                BGS_ASSERT( pDevice != nullptr );

                if( BGS_FAILED( pDevice->Create( desc, this ) ) )
                {
                    Memory::FreeObject( m_pParent->GetDefaultAllocator(), &pDevice );
                    return Results::FAIL;
                }

                ( *ppDevice ) = pDevice;

                return Results::OK;
            }

            void VulkanFactory::DestroyDevice( IDevice** ppDevice )
            {
                BGS_ASSERT( ( ppDevice != nullptr ) && ( *ppDevice != nullptr ) );
                if( *ppDevice != nullptr )
                {
                    static_cast<VulkanDevice*>( *ppDevice )->Destroy();
                }

                Core::Memory::FreeObject( m_pParent->GetDefaultAllocator(), ppDevice );
            }

            RESULT VulkanFactory::Create( const FactoryDesc& desc, BIGOS::Driver::Frontend::RenderSystem* pParent )
            {
                BGS_ASSERT( pParent != nullptr );

                m_desc    = desc;
                m_pParent = pParent;

                if( volkInitialize() != VK_SUCCESS )
                {
                    return Results::FAIL;
                }

                if( BGS_FAILED( CreateVkInstance() ) )
                {
                    return Results::FAIL;
                }

                if( BGS_FAILED( EnumAdapters() ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void VulkanFactory::Destroy()
            {
                BGS_ASSERT( m_handle != FactoryHandle() );

                if( m_handle != FactoryHandle() )
                {
                    VkInstance nativeInstance = m_handle.GetNativeHandle();

#if( BGS_RENDER_DEBUG )
                    if( m_nativeDebug != VK_NULL_HANDLE )
                    {
                        vkDestroyDebugUtilsMessengerEXT( nativeInstance, m_nativeDebug, nullptr );
                    }
#endif // (BGS_RENDER_DEBUG)
                    vkDestroyInstance( nativeInstance, nullptr );
                }

                for( index_t ndx = 0; ndx < m_adapters.size(); ++ndx )
                {
                    static_cast<VulkanAdapter*>( m_adapters[ ndx ] )->Destroy();
                    Core::Memory::FreeObject( m_pParent->GetDefaultAllocator(), &m_adapters[ ndx ] );
                }
                m_adapters.clear();

                m_pParent = nullptr;
                m_handle  = FactoryHandle();
            }

            RESULT VulkanFactory::CheckValidationLayersSupport()
            {
                uint32_t layerCount = 0;
                vkEnumerateInstanceLayerProperties( &layerCount, nullptr );

                HeapArray<VkLayerProperties> layerProps( layerCount );
                if( vkEnumerateInstanceLayerProperties( &layerCount, layerProps.data() ) != VK_SUCCESS )
                {
                    BGS_ASSERT( 0 );
                    return Results::FAIL;
                }

                HeapArray<const char*> availLayers( layerCount );
                for( index_t ndx = 0; ndx < availLayers.size(); ++ndx )
                {
                    availLayers[ ndx ] = layerProps[ ndx ].layerName;
                }

                if( Core::Utils::String::FindString( availLayers, "VK_LAYER_KHRONOS_validation" ) != INVALID_POSITION )
                {
                    m_debugLayers[ 0 ] = "VK_LAYER_KHRONOS_validation";
                    return Results::OK;
                }

                return Results::NOT_FOUND;
            }

            RESULT VulkanFactory::CheckVkExtSupport( uint32_t extCount, const char* const* ppExts )
            {
                uint32_t availExtCount = 0;
                vkEnumerateInstanceExtensionProperties( nullptr, &availExtCount, nullptr );

                HeapArray<VkExtensionProperties> extProps( availExtCount );
                if( vkEnumerateInstanceExtensionProperties( nullptr, &availExtCount, extProps.data() ) != VK_SUCCESS )
                {
                    BGS_ASSERT( 0 );
                    return Results::FAIL;
                }

                HeapArray<const char*> availExts( availExtCount );
                for( index_t ndx = 0; ndx < availExts.size(); ++ndx )
                {
                    availExts[ ndx ] = extProps[ ndx ].extensionName;
                }

                bool_t failed = BGS_FALSE;

                for( index_t ndx = 0; ndx < extCount; ++ndx )
                {
                    if( Core::Utils::String::FindString( availExts, ppExts[ ndx ] ) == INVALID_POSITION )
                    {
                        failed = BGS_TRUE;
                    }
                }

                if( failed == BGS_TRUE )
                {
                    return Results::NOT_FOUND;
                }

                return Results::OK;
            }

            RESULT VulkanFactory::CreateVkInstance()
            {
                VkInstance nativeInstance = VK_NULL_HANDLE;

                // Required exts
                HeapArray<const char*> instExt = {
                    VK_KHR_SURFACE_EXTENSION_NAME,
#if( BGS_WINDOWS )
                    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif( BGS_LINUX )
#    error
#endif // (BGS_WINDOWS)
                };

                // In application info we only specify latest API Version.
                VkApplicationInfo appInfo;
                appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
                appInfo.pNext              = nullptr;
                appInfo.engineVersion      = 0;
                appInfo.pEngineName        = nullptr;
                appInfo.applicationVersion = 0;
                appInfo.pApplicationName   = nullptr;
                appInfo.apiVersion         = VK_API_VERSION_1_3;

                VkInstanceCreateInfo instInfo;
                instInfo.sType               = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
                instInfo.pNext               = nullptr;
                instInfo.pApplicationInfo    = &appInfo;
                instInfo.flags               = 0;
                instInfo.enabledLayerCount   = 0;
                instInfo.ppEnabledLayerNames = nullptr;

#if( BGS_RENDER_DEBUG )
                VkValidationFeaturesEXT            valFeatures;
                VkValidationFeatureEnableEXT       enableFeatures[ 2 ];
                uint32_t                           enableCount = 0;
                VkDebugUtilsMessengerCreateInfoEXT debugInfo;
                debugInfo.pNext           = nullptr;
                debugInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                debugInfo.flags           = 0;
                debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                debugInfo.pfnUserCallback = DebugCallback;

                if( m_desc.flags > 0 )
                {
                    RESULT      layersSupport = CheckValidationLayersSupport();
                    const char* layerExt[]    = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
                    RESULT      dbgExtSupport = CheckVkExtSupport( 1, layerExt );
                    if( ( BGS_FAILED( layersSupport ) ) && ( BGS_FAILED( dbgExtSupport ) ) &&
                        ( m_desc.flags & static_cast<uint32_t>( FactoryFlagBits::ENABLE_DEBUG_LAYERS ) ) )
                    {
                        return Results::FAIL;
                    }
                    if( BGS_SUCCESS( layersSupport ) )
                    {
                        instExt.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
                        const char* layerGPUExt[]    = { VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME };
                        RESULT      dbgGPUExtSupport = CheckVkExtSupport( 1, layerGPUExt );
                        if( m_desc.flags & static_cast<uint32_t>( FactoryFlagBits::ENABLE_GPU_BASED_VALIDATION ) )
                        {
                            if( BGS_FAILED( dbgGPUExtSupport ) )
                            {
                                return Results::FAIL;
                            }
                            enableFeatures[ enableCount++ ] = VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT;
                        }
                        if( m_desc.flags & static_cast<uint32_t>( FactoryFlagBits::ENABLE_QUEUE_SYNCHRONIZATION_VALIDATION ) )
                        {
                            if( BGS_FAILED( dbgGPUExtSupport ) )
                            {
                                return Results::FAIL;
                            }
                            enableFeatures[ enableCount++ ] = VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT;
                        }
                        if( enableCount > 0 )
                        {
                            instExt.push_back( VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME );
                        }

                        valFeatures.sType                          = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
                        valFeatures.pNext                          = &debugInfo;
                        valFeatures.pEnabledValidationFeatures     = enableFeatures;
                        valFeatures.enabledValidationFeatureCount  = enableCount;
                        valFeatures.pDisabledValidationFeatures    = nullptr;
                        valFeatures.disabledValidationFeatureCount = 0;

                        instInfo.ppEnabledLayerNames = m_debugLayers;
                        instInfo.enabledLayerCount   = 1;
                        instInfo.pNext               = &valFeatures;
                    }
                }
#endif // ( BGS_RENDER_DEBUG )

                const uint32_t extCount = static_cast<uint32_t>( instExt.size() );
                if( BGS_FAILED( CheckVkExtSupport( extCount, &instExt[ 0 ] ) ) )
                {
                    return Results::FAIL;
                }
                instInfo.ppEnabledExtensionNames = instExt.data();
                instInfo.enabledExtensionCount   = extCount;

                if( vkCreateInstance( &instInfo, nullptr, &nativeInstance ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }
                // volkLoadInstance( nativeInstance );
                volkLoadInstanceOnly( nativeInstance );

#if( BGS_RENDER_DEBUG )
                if( m_desc.flags > 0 )
                {
                    if( vkCreateDebugUtilsMessengerEXT( nativeInstance, &debugInfo, nullptr, &m_nativeDebug ) != VK_SUCCESS )
                    {
                        Destroy();
                        return Results::FAIL;
                    }
                }
#endif // ( BGS_RENDER_DEBUG )

                m_handle = FactoryHandle( nativeInstance );

                return Results::OK;
            }

            RESULT VulkanFactory::EnumAdapters()
            {
                BGS_ASSERT( m_handle != FactoryHandle() );
                VkInstance nativeInstance = m_handle.GetNativeHandle();
                BGS_ASSERT( nativeInstance != VK_NULL_HANDLE );

                uint32_t adaptersCount = 0;
                vkEnumeratePhysicalDevices( nativeInstance, &adaptersCount, nullptr );

                HeapArray<VkPhysicalDevice> nativeAdapters( adaptersCount );
                if( vkEnumeratePhysicalDevices( nativeInstance, &adaptersCount, nativeAdapters.data() ) != VK_SUCCESS )
                {
                    BGS_ASSERT( 0 );
                    return Results::FAIL;
                }

                for( index_t ndx = 0; ndx < nativeAdapters.size(); ++ndx )
                {
                    VulkanAdapter* pVulkanAdapter = nullptr;
                    if( BGS_FAILED( Core::Memory::AllocateObject( m_pParent->GetDefaultAllocator(), &pVulkanAdapter ) ) )
                    {
                        return Results::NO_MEMORY;
                    }
                    if( BGS_FAILED( pVulkanAdapter->Create( nativeAdapters[ ndx ], this ) ) )
                    {
                        Core::Memory::FreeObject( m_pParent->GetDefaultAllocator(), &pVulkanAdapter );
                        return Results::FAIL;
                    }

                    m_adapters.push_back( pVulkanAdapter );
                }

                return Results::OK;
            }

#if( BGS_RENDER_DEBUG )
            VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT,
                                                          const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* )
            {
                // TODO: Refactor after logger.
                if( messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT )
                {
                    printf( "%s", pCallbackData->pMessage );
                    printf( "\n" );
                }
                else if( messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT )
                {
                    printf( "%s", pCallbackData->pMessage );
                    printf( "\n" );
                }
                else if( messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT )
                {
                    printf( "%s", pCallbackData->pMessage );
                    printf( "\n" );
                }
                else if( messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT )
                {
                    printf( "%s", pCallbackData->pMessage );
                    printf( "\n" );
                }

                return VK_FALSE;
            }
#endif // ( BGS_RENDER_DEBUG )

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS