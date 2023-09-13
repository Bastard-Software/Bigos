#pragma once

#include "D3D12Types.h"

namespace BIGOS::Driver::Backend
{

    struct D3D12Sampler
    {
        D3D12_SAMPLER_DESC        sampler;
        D3D12_STATIC_BORDER_COLOR staticColor;
    };

} // namespace BIGOS::Driver::Backend