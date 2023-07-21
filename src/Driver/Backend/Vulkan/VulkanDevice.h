#pragma once

#include "VulkanTypes.h"

#include "Driver/Backend/API.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            class BGS_API VulkanDevice final : public IDevice
            {
                friend class D3D12Factory;

            public:
            protected:
                RESULT Create( const DeviceDesc& desc, VulkanFactory* pFactory );
                void   Destroy();

            private:
                RESULT CreateVkDevice();

            private:
                VolkDeviceTable* m_pDeviceAPI = nullptr;
                VulkanFactory*   m_pParent    = nullptr;
            };
        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS