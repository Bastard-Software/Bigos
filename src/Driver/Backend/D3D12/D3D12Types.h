#pragma once

#include "Driver/Backend/APIHandles.h"

#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

namespace BIGOS::Driver::Backend
{
    struct D3D12Fence;
}

#if BGS_USE_D3D12_HANDLES
#    define BGS_DECLARE_D3D12_HANDLE( name, type )                                                                                                   \
        template<>                                                                                                                                   \
        struct BIGOS::Core::HandleTraits<BIGOS::Driver::Backend::Private::BGS_HANDLE_TAG_NAME( name )>                                               \
        {                                                                                                                                            \
            using NativeHandleTypes = type;                                                                                                          \
        };

BGS_DECLARE_D3D12_HANDLE( Factory, IDXGIFactory4* );
BGS_DECLARE_D3D12_HANDLE( Adapter, IDXGIAdapter1* );
BGS_DECLARE_D3D12_HANDLE( Device, ID3D12Device* );
BGS_DECLARE_D3D12_HANDLE( Queue, ID3D12CommandQueue* );
BGS_DECLARE_D3D12_HANDLE( CommandPool, ID3D12CommandAllocator* );
BGS_DECLARE_D3D12_HANDLE( CommandBuffer, ID3D12GraphicsCommandList4* );
BGS_DECLARE_D3D12_HANDLE( Fence, BIGOS::Driver::Backend::D3D12Fence* );
BGS_DECLARE_D3D12_HANDLE( Semaphore, ID3D12Fence* );

#endif

#include "Driver/Backend/APITypes.h"

namespace BIGOS::Driver::Backend
{
    class D3D12Adapter;
    class D3D12Factory;
    class D3D12Device;
} // namespace BIGOS::Driver::Backend

#define RELEASE_COM_PTR( _x )                                                                                                                        \
    {                                                                                                                                                \
        if( ( _x ) != nullptr )                                                                                                                      \
        {                                                                                                                                            \
            ( _x )->Release();                                                                                                                       \
            ( _x ) = nullptr;                                                                                                                        \
        }                                                                                                                                            \
    }