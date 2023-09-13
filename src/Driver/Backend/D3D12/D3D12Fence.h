#pragma once

#include "D3D12Types.h"

namespace BIGOS::Driver::Backend
{

    struct D3D12Fence
    {
        ID3D12Fence* pFence;
        HANDLE       hEvent;
    };

} // namespace BIGOS::Driver::Backend