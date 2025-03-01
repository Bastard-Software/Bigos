#pragma once

#include "D3D12Types.h"

#include "Driver/Backend/API.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            class BGS_API D3D12Swapchain final : public ISwapchain
            {
                friend class D3D12Device;

            public:
                D3D12Swapchain();

                virtual RESULT Present( const SwapchainPresentDesc& desc ) override;
                virtual RESULT GetNextFrame( FrameInfo* pInfo ) override;

            protected:
                RESULT Create( const SwapchainDesc& desc, D3D12Device* pDevice );
                void   Destroy();

            private:
                RESULT CreateD3D12Swapchain();

                RESULT GetD3D12BackBuffers();

            private:
                D3D12Device* m_pParent;
                UINT         m_swapInterval;
            };
        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS