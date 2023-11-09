#pragma once

#include "D3D12Types.h"

namespace BIGOS::Driver::Backend
{

    struct D3D12BindingHeapLayout
    {
        const D3D12_STATIC_SAMPLER_DESC* pImmutableSamplers;
        D3D12_ROOT_DESCRIPTOR_TABLE1     shaderResourceTable;
        D3D12_ROOT_DESCRIPTOR_TABLE1     samplerTable;
        const BINDING_TYPE*              pBindingLayout;
        uint32_t                         bindingCount;
        uint32_t                         immutableSamplerCount;
        D3D12_SHADER_VISIBILITY          visibility;
    };

} // namespace BIGOS::Driver::Backend