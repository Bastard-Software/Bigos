#pragma once

#include "D3D12Types.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            struct D3D12Fence
            {
                ID3D12Fence* pFence;
                HANDLE       hEvent;
            };
        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS