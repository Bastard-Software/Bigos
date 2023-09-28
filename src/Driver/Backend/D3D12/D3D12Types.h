#pragma once

#include "Driver/Backend/APIHandles.h"

#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

namespace BIGOS::Driver::Backend
{
    struct D3D12Fence;
    struct D3D12ShaderModule;
    struct D3D12Pipeline;
    struct D3D12Resource;
    struct D3D12ResourceView;
    struct D3D12Sampler;
    struct D3D12BindingHeapLayout;
} // namespace BIGOS::Driver::Backend

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
BGS_DECLARE_D3D12_HANDLE( Swapchain, IDXGISwapChain3* );
BGS_DECLARE_D3D12_HANDLE( CommandPool, ID3D12CommandAllocator* );
BGS_DECLARE_D3D12_HANDLE( CommandBuffer, ID3D12GraphicsCommandList4* );
BGS_DECLARE_D3D12_HANDLE( Shader, BIGOS::Driver::Backend::D3D12ShaderModule* );
BGS_DECLARE_D3D12_HANDLE( PipelineLayout, ID3D12RootSignature* );
BGS_DECLARE_D3D12_HANDLE( Pipeline, BIGOS::Driver::Backend::D3D12Pipeline* );
BGS_DECLARE_D3D12_HANDLE( Fence, BIGOS::Driver::Backend::D3D12Fence* );
BGS_DECLARE_D3D12_HANDLE( Semaphore, ID3D12Fence* );
BGS_DECLARE_D3D12_HANDLE( Memory, ID3D12Heap* )
BGS_DECLARE_D3D12_HANDLE( Resource, BIGOS::Driver::Backend::D3D12Resource* )
BGS_DECLARE_D3D12_HANDLE( ResourceView, BIGOS::Driver::Backend::D3D12ResourceView* )
BGS_DECLARE_D3D12_HANDLE( Sampler, BIGOS::Driver::Backend::D3D12Sampler* )
BGS_DECLARE_D3D12_HANDLE( BindingHeapLayout, BIGOS::Driver::Backend::D3D12BindingHeapLayout* )
BGS_DECLARE_D3D12_HANDLE( BindingHeap, ID3D12DescriptorHeap* )
BGS_DECLARE_D3D12_HANDLE( QueryPool, ID3D12QueryHeap* )

#endif

#include "Driver/Backend/APITypes.h"

namespace BIGOS::Driver::Backend
{
    class D3D12Adapter;
    class D3D12Factory;
    class D3D12Device;

    enum class D3D12SemaphoreStates : uint64_t
    {
        NOT_SIGNALED,
        SIGNALED,
        _MAX_ENUM,
    };
    using D3D12_SEMAPHORE_STATE = D3D12SemaphoreStates;

    enum class D3D12DescriptorTypes : uint8_t
    {
        RTV,
        DSV,
        CBV,
        SRV,
        UAV,
        _MAX_ENUM,
    };
    using D3D12_DESCRIPTOR_TYPE = D3D12DescriptorTypes;

} // namespace BIGOS::Driver::Backend

#define RELEASE_COM_PTR( _x )                                                                                                                        \
    {                                                                                                                                                \
        if( ( _x ) != nullptr )                                                                                                                      \
        {                                                                                                                                            \
            ( _x )->Release();                                                                                                                       \
            ( _x ) = nullptr;                                                                                                                        \
        }                                                                                                                                            \
    }

#include "Driver/Backend/APICommon.h"