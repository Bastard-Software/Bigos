#pragma once
#include "VulkanTypes.h"

#include "Driver/Backend/API.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {
            class DriverSystem;
        }

        namespace Backend
        {
            class BGS_API VulkanFactory final : public IFactory
            {
                friend class BIGOS::Driver::Frontend::DriverSystem;

            public:
                RESULT CreateDevice( const DeviceDesc& desc, IDevice** ppDevice ) override;
                void   DestroyDevice( IDevice** ppDevice ) override;

            protected:
                RESULT Create( const FactoryDesc& desc, BIGOS::Driver::Frontend::DriverSystem* pParent );
                void   Destroy();

            private:
                RESULT CheckValidationLayersSupport();
                RESULT CheckVkExtSupport( const uint32_t count, const char* const* ppNames );
                RESULT CreateVkInstance();

                RESULT EnumAdapters();

            private:
                BIGOS::Driver::Frontend::DriverSystem* m_pParent;

#if( BGS_RENDER_DEBUG )
                VkDebugUtilsMessengerEXT m_nativeDebug;
                const char*              m_debugLayers[ 1 ];
#endif // ( BGS_RENDER_DEBUG )
            };
        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS