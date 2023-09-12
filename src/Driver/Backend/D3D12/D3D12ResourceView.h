#pragma once

#include "D3D12Types.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {

            struct D3D12ResourceView
            {
                union
                {
                    BufferViewDesc  bufferView;
                    TextureViewDesc textureView;
                };
            };

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS