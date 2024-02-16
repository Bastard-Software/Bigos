#pragma once

#include "D3D12Types.h"

namespace BIGOS::Driver::Backend
{

    struct D3D12PipelineLayout
    {
        ID3D12RootSignature* pNativeRootSignature;
        uint32_t             pushConstantTable[ 8 ]; // Count of D3D12_SHADER_VISIBILITY
    };

} // namespace BIGOS::Driver::Backend