#pragma once

#include "D3D12Types.h"

namespace BIGOS::Driver::Backend
{
    struct D3D12Resource
    {
        ID3D12Resource*     pNativeResource;
        D3D12_RESOURCE_DESC desc;
    };
} // namespace BIGOS::Driver::Backend