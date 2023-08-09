#pragma once

#include "D3D12Types.h"

namespace BIGOS::Driver::Backend
{
    struct D3D12Pipeline
    {
        ID3D12PipelineState* pPipeline;
        ID3D12RootSignature* pRootSignature;
    };
} // namespace BIGOS::Driver::Backend