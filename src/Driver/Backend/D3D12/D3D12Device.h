#pragma once
#include "D3D12Types.h"

#include "Driver/Backend/API.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {

            class BGS_API D3D12Device final : public IDevice
            {
                friend class D3D12Factory;

            public:
            protected:
                RESULT Create( const DeviceDesc& desc, D3D12Factory* pParent );
                void   Destroy();

            private:
                RESULT CreateD3D12Device();

            private:
                D3D12Factory* m_pParent = nullptr;
            };

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS