#pragma once

#include "D3D12Types.h"

namespace BIGOS::Driver::Backend
{

    struct D3D12ResourceView
    {
        union
        {
            BufferViewDesc  bufferView;
            TextureViewDesc textureView;
        };
    };

} // namespace BIGOS::Driver::Backend