#pragma once

#include "D3D12Types.h"

namespace BIGOS::Driver::Backend
{

    struct D3D12ResourceView
    {
        union
        {
            uint32_t rtvNdx;
            uint32_t dsvNdx;
            uint32_t cbvNdx;
            uint32_t srvNdx;
            uint32_t uavNdx;
        };
        D3D12_DESCRIPTOR_TYPE type;
    };

} // namespace BIGOS::Driver::Backend