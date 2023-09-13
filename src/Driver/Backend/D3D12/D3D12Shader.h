#pragma once

#include "Core/CoreTypes.h"

namespace BIGOS::Driver::Backend
{

    struct D3D12ShaderModule
    {
        byte_t*  pByteCode;
        uint32_t codeSize;
    };

} // namespace BIGOS::Driver::Backend