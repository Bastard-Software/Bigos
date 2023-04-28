#pragma once

#include "Driver/Backend/APIHandles.h"

#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

#if BGS_USE_D3D12_HANDLES
#    define BGS_DECLARE_D3D12_HANDLE( name, type )                                                                     \
        template<>                                                                                                     \
        struct BIGOS::Core::Handle<BIGOS::Driver::Backend::Private::BGS_HANDLE_TAG_NAME( name )>                       \
        {                                                                                                              \
            using NativeHandleTypes = type;                                                                            \
        };

BGS_DECLARE_D3D12_HANDLE( Factory, IDXGIFactory4* );
BGS_DECLARE_D3D12_HANDLE( Adapter, IDXGIAdapter1* );

#endif

#include "Driver/Backend/APITypes.h"

namespace BIGOS::Driver::Backend
{
    class D3D12Adapter;
    class D3D12Factory;
} // namespace BIGOS::Driver::Backend