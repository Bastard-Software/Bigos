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
                virtual RESULT CreateQueue( const QueueDesc& desc, IQueue** ppQueue ) override;
                virtual void   DestroyQueue( IQueue** ppQueue ) override;

                virtual RESULT CreateCommandPool( const CommandPoolDesc& desc, CommandPoolHandle* pHandle ) override;
                virtual void   DestroyCommandPool( CommandPoolHandle* pHandle ) override;
                virtual RESULT ResetCommandPool( CommandPoolHandle handle ) override;

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