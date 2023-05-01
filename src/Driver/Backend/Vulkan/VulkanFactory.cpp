#define BGS_USE_VULKAN_HANDLES 1

#include "Core/CoreTypes.h"

#include "VulkanFactory.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {

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

                return Results::OK;
            }

            void VulkanFactory::Destroy()
            {
                m_pParent = nullptr;
            }

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS