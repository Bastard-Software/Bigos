#define BGS_USE_VULKAN_HANDLES 1

#include "VulkanFactory.h"

#include "Core/CoreTypes.h"

#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Memory.h"
#include "Driver/Frontend/DriverSystem.h"
#include "VulkanAdapter.h"

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
                desc;

                return Results::OK;
            }

            void VulkanFactory::DestroyDevice( IDevice** ppDevice )
            {
                BGS_ASSERT( ( ppDevice != nullptr ) && ( *ppDevice != nullptr ) );
            }

            RESULT VulkanFactory::Create( const FactoryDesc& desc, BIGOS::Driver::Frontend::DriverSystem* pParent )
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

                for( index_t i = 0; i < m_adapters.size(); ++i )
                {
                    static_cast<VulkanAdapter*>( m_adapters[ i ] )->Destroy();
                    Core::Memory::FreeObject( m_pParent->GetDefaultAllocatorPtr(), &m_adapters[ i ] );
                }
                m_adapters.clear();

                BGS_ASSERT( m_handle != FactoryHandle() );
                VkInstance nativeInstance = m_handle.GetNativeHandle();

                BGS_ASSERT( nativeInstance != VK_NULL_HANDLE );
                if( nativeInstance != VK_NULL_HANDLE )
                {
#if( BGS_RENDER_DEBUG )
                    vkDestroyDebugUtilsMessengerEXT( nativeInstance, m_nativeDebug, nullptr );
#endif // (BGS_RENDER_DEBUG)
                    vkDestroyInstance( nativeInstance, nullptr );
                }

                m_handle  = FactoryHandle();
                m_pParent = nullptr;
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
                for( index_t i = 0; i < availLayers.size(); ++i )
                {
                    availLayers[ i ] = layerProps[ i ].layerName;
                }

                for( index_t i = 0; i < availLayers.size(); ++i )
                {
                    if( !strcmp( availLayers[ i ], "VK_LAYER_KHRONOS_validation" ) )
                    {
                        m_debugLayers[ 0 ] = "VK_LAYER_KHRONOS_validation";
                        return Results::OK;
                    }
                }

                return Results::NOT_FOUND;
            }

            RESULT VulkanFactory::CheckVkExtSupport( const uint32_t count, const char* const* ppNames )
            {
                uint32_t extCount = 0;
                vkEnumerateInstanceExtensionProperties( nullptr, &extCount, nullptr );

                HeapArray<VkExtensionProperties> extProps( extCount );
                if( vkEnumerateInstanceExtensionProperties( nullptr, &extCount, extProps.data() ) != VK_SUCCESS )
                {
                    BGS_ASSERT( 0 );
                    return Results::FAIL;
                }

                HeapArray<const char*> availExts( extCount );
                for( index_t i = 0; i < availExts.size(); ++i )
                {
                    availExts[ i ] = extProps[ i ].extensionName;
                }

                for( index_t i = 0; i < count; ++i )
                {
                    bool found = false;
                    for( index_t j = 0; j < extCount; ++j )
                    {
                        if( strcmp( ppNames[ i ], availExts[ j ] ) )
                        {
                            found = true;
                        }
                    }
                    if( !found )
                    {
                        return Results::NOT_FOUND;
                    }
                }

                return Results::OK;
            }

            RESULT VulkanFactory::CreateVkInstance()
            {
                // Required exts
                const char* instExts[] = {
                    VK_KHR_SURFACE_EXTENSION_NAME,
#if( BGS_WINDOWS )
                    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif( BGS_LINUX )
#    error
#endif // (BGS_WINDOWS)

#if( BGS_RENDER_DEBUG )
                    VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME,
                    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif // ( BGS_RENDER_DEBUG )
                };
                const uint32_t count = _countof( instExts );

                // We ignore application info, it is useless for now.
                VkInstanceCreateInfo instInfo;
                instInfo.sType               = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
                instInfo.pNext               = nullptr;
                instInfo.pApplicationInfo    = nullptr;
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
                    RESULT layersSupport = CheckValidationLayersSupport();
                    if( ( BGS_FAILED( layersSupport ) ) && ( m_desc.flags & static_cast<uint32_t>( FactoryFlagBits::ENABLE_DEBUG_LAYERS ) ) )
                    {
                        return Results::FAIL;
                    }
                    if( BGS_SUCCESS( layersSupport ) )
                    {
                        if( m_desc.flags & static_cast<uint32_t>( FactoryFlagBits::ENABLE_GPU_BASED_VALIDATION ) )
                        {
                            enableFeatures[ enableCount++ ] = VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT;
                        }
                        if( m_desc.flags & static_cast<uint32_t>( FactoryFlagBits::ENABLE_QUEUE_SYNCHRONIZATION_VALIDATION ) )
                        {
                            enableFeatures[ enableCount++ ] = VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT;
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

                if( BGS_FAILED( CheckVkExtSupport( count, instExts ) ) )
                {
                    return Results::FAIL;
                }
                instInfo.ppEnabledExtensionNames = instExts;
                instInfo.enabledExtensionCount   = count;

                VkInstance nativeInstance = VK_NULL_HANDLE;
                if( vkCreateInstance( &instInfo, nullptr, &nativeInstance ) != VK_SUCCESS )
                {
                    return Results::FAIL;
                }
                volkLoadInstance( nativeInstance );

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

                for( index_t i = 0; i < nativeAdapters.size(); ++i )
                {
                    VulkanAdapter* pVulkanAdapter = nullptr;
                    if( BGS_FAILED( Core::Memory::AllocateObject( m_pParent->GetDefaultAllocatorPtr(), &pVulkanAdapter ) ) )
                    {
                        return Results::NO_MEMORY;
                    }
                    if( BGS_FAILED( pVulkanAdapter->Create( nativeAdapters[ i ], this ) ) )
                    {
                        Core::Memory::FreeObject( m_pParent->GetDefaultAllocatorPtr(), &pVulkanAdapter );
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