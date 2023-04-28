#pragma once
#include "D3D12Types.h"

#include "Driver/Backend/API.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            class BGS_API D3D12Adapter final : public IAdapter
            {
            public:
                friend class D3D12Factory;

            protected:
                RESULT Create( IDXGIAdapter1* pAdapter, D3D12Factory* pFactory );
                void   Destroy();

            private:
                D3D12Factory* m_pFactory = nullptr;
            };
        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS