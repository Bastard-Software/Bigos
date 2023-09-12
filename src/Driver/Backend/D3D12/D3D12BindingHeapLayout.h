#pragma once

#include "D3D12Types.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {

            struct D3D12ImmutableSampler
            {
                D3D12_SAMPLER_DESC*     pSampler;
                D3D12_SHADER_VISIBILITY visibility;
                uint32_t                shaderRegister;
            };

            struct D3D12BindingHeapLayout
            {
                const D3D12ImmutableSampler* pImmutableSamplers;
                D3D12_ROOT_DESCRIPTOR_TABLE1 bindingTable;
                uint32_t                     immutableSamplerCount;
                D3D12_SHADER_VISIBILITY      visibility;
            };

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS