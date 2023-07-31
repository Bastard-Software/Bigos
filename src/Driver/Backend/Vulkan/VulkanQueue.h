#pragma once

#include "VulkanTypes.h"

#include "Driver/Backend/API.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {

            class BGS_API VulkanQueue final : public IQueue
            {
                friend class VulkanDevice;

            public:
                virtual RESULT Submit( const QueueSubmitDesc& desc ) override;
                virtual RESULT Wait( const QueueWaitDesc& desc ) override;

            protected:
                RESULT Create( const QueueDesc& desc, VulkanDevice* pDevice );
                void   Destroy();

            private:
                VulkanDevice* m_pParent = nullptr;

                uint32_t m_nativeQueueFamilyIndex = MAX_UINT32; // Equivalent of INVALID_POSITION for 32 bit ints.
                uint32_t m_nativeQueueIndex       = MAX_UINT32; // Equivalent of INVALID_POSITION for 32 bit ints.
            };

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS