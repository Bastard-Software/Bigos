#pragma once
#include "VulkanTypes.h"

#include "Driver/Backend/API.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {
            class RenderSystem;
        }

        namespace Backend
        {
            class BGS_API VulkanFactory final : public IFactory
            {
                friend class BIGOS::Driver::Frontend::RenderSystem;

            public:
                RESULT CreateDevice( const DeviceDesc& desc, IDevice** ppDevice ) override;
                void   DestroyDevice( IDevice** ppDevice ) override;

                BIGOS::Driver::Frontend::RenderSystem* GetParent() { return m_pParent; }

            protected:
                RESULT Create( const FactoryDesc& desc, BIGOS::Driver::Frontend::RenderSystem* pParent );
                void   Destroy();

            private:
                RESULT CheckValidationLayersSupport();
                RESULT CheckVkExtSupport( uint32_t extCount, const char* const* ppExts );
                RESULT CreateVkInstance();

                RESULT EnumAdapters();

            private:
                BIGOS::Driver::Frontend::RenderSystem* m_pParent = nullptr;

#if( BGS_RENDER_DEBUG )
                VkDebugUtilsMessengerEXT m_nativeDebug;
                const char*              m_debugLayers[ 1 ];
#endif // ( BGS_RENDER_DEBUG )
            };
        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS