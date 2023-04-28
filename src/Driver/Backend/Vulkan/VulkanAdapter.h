#pragma once
#include "VulkanTypes.h"

#include "Driver/Backend/API.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            class BGS_API VulkanAdapter final : public IAdapter
            {
            public:
                friend class VulkanFactory;

            protected:
                RESULT Create( VkPhysicalDevice adapter, VulkanFactory* pFactory );
                void   Destroy();

            private:
                VulkanFactory* m_pFactory = nullptr;
            };

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS