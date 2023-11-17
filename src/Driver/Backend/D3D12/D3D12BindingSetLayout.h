#pragma once

#include "D3D12Types.h"

namespace BIGOS::Driver::Backend
{

    struct D3D12BindingSetLayout
    {
        const BindingRangeDesc* pRanges;
        uint32_t                rangeCount;
        D3D12_SHADER_VISIBILITY visibility;
    };

} // namespace BIGOS::Driver::Backend