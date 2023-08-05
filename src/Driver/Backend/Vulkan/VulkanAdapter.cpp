#define BGS_USE_VULKAN_HANDLES 1

#include "VulkanAdapter.h"

#include "Core/Utils/String.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            static ADAPTER_TYPE MapVkPhysicalDeviceTypeToBIGOSAdapterType( VkPhysicalDeviceType type );
            static VersionDesc  GetVkPhysicalDeviceAPIVersion( uint32_t version );
            static VersionDesc  GetVkPhysicalDeviceDriverVersion( VkPhysicalDevice nativeAdapter );

            RESULT VulkanAdapter::Create( VkPhysicalDevice adapter, VulkanFactory* pFactory )
            {
                BGS_ASSERT( adapter != VK_NULL_HANDLE, "Adapter (adapter) must be a valid pointer." );
                BGS_ASSERT( pFactory != nullptr, "Factory (pFactory) must be a valid pointer." );

                m_handle  = AdapterHandle( adapter );
                m_pParent = pFactory;

                GetInternalInfo();

                return Results::OK;
            }

            void VulkanAdapter::Destroy()
            {
                BGS_ASSERT( m_handle != AdapterHandle() );
                m_handle  = AdapterHandle();
                m_pParent = nullptr;
            }

            RESULT VulkanAdapter::GetInternalInfo()
            {
                BGS_ASSERT( m_handle != AdapterHandle() );
                VkPhysicalDevice nativeAdapter = m_handle.GetNativeHandle();
                BGS_ASSERT( nativeAdapter != VK_NULL_HANDLE );

                VkPhysicalDeviceProperties adapterProps;
                vkGetPhysicalDeviceProperties( nativeAdapter, &adapterProps );

                const char* pAdapterName = adapterProps.deviceName;
                Core::Utils::String::Copy( m_info.description, Config::Driver::Adapter::MAX_DESCRIPTION_TEXT_LENGHT, pAdapterName );
                m_info.vendorID      = adapterProps.vendorID;
                m_info.deviceID      = adapterProps.deviceID;
                m_info.type          = MapVkPhysicalDeviceTypeToBIGOSAdapterType( adapterProps.deviceType );
                m_info.apiVersion    = GetVkPhysicalDeviceAPIVersion( adapterProps.apiVersion );
                m_info.driverVersion = GetVkPhysicalDeviceDriverVersion( nativeAdapter );

                return Results::OK;
            }

            ADAPTER_TYPE MapVkPhysicalDeviceTypeToBIGOSAdapterType( VkPhysicalDeviceType type )
            {
                static const ADAPTER_TYPE translateTable[ 5 ] = {
                    AdapterTypes::UNKNOWN,    // VK_PHYSICAL_DEVICE_TYPE_OTHER
                    AdapterTypes::INTEGRATED, // VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
                    AdapterTypes::DISCRETE,   // VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
                    AdapterTypes::SOFTWARE,   // VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU
                    AdapterTypes::SOFTWARE,   // VK_PHYSICAL_DEVICE_TYPE_CPU
                };

                return translateTable[ type ];
            }

            VersionDesc GetVkPhysicalDeviceAPIVersion( uint32_t version )
            {
                VersionDesc apiVersion;
                apiVersion.major = VK_API_VERSION_MAJOR( version );
                apiVersion.minor = VK_API_VERSION_MINOR( version );
                apiVersion.patch = VK_API_VERSION_PATCH( version );

                return apiVersion;
            }

            VersionDesc GetVkPhysicalDeviceDriverVersion( VkPhysicalDevice nativeAdapter )
            {
                BGS_ASSERT( nativeAdapter != VK_NULL_HANDLE );

                VersionDesc driverVersion;

                VkPhysicalDeviceIDProperties adapterIDProperties;
                adapterIDProperties       = {};
                adapterIDProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;

                VkPhysicalDeviceProperties2 adapterProperties;
                adapterProperties       = {};
                adapterProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
                adapterProperties.pNext = &adapterIDProperties;

                vkGetPhysicalDeviceProperties2( nativeAdapter, &adapterProperties );

                Core::Utils::String::Parse( reinterpret_cast<const char*>( adapterIDProperties.driverUUID ), sizeof( adapterIDProperties.driverUUID ),
                                            "%hu.%hu.%hu.%hu", &driverVersion.major, &driverVersion.minor, &driverVersion.patch,
                                            &driverVersion.build );

                return driverVersion;
            }

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS