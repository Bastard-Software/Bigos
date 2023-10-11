#pragma once

#include "D3D12Types.h"

#include "Driver/Backend/API.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {

            class BGS_API D3D12Queue final : public IQueue
            {
                friend class D3D12Device;

            public:
                virtual RESULT Submit( const QueueSubmitDesc& desc ) override;

            protected:
                RESULT Create( const QueueDesc& desc, D3D12Device* pDevice );
                void   Destroy();

            private:
                RESULT QueryQueueLimits();

            private:
                D3D12Device* m_pParent = nullptr;
            };

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS