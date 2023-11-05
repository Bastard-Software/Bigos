#define BGS_USE_D3D12_HANDLES 1

#include "D3D12Device.h"

#include "BigosFramework/Config.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Memory.h"
#include "Core/Utils/String.h"
#include "D3D12BindingHeapLayout.h"
#include "D3D12CommandBuffer.h"
#include "D3D12Common.h"
#include "D3D12Factory.h"
#include "D3D12Fence.h"
#include "D3D12Pipeline.h"
#include "D3D12Queue.h"
#include "D3D12Resource.h"
#include "D3D12ResourceView.h"
#include "D3D12Sampler.h"
#include "D3D12Shader.h"
#include "D3D12Swapchain.h"
#include "Driver/Frontend/RenderSystem.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            RESULT D3D12Device::CreateQueue( const QueueDesc& desc, IQueue** ppQueue )
            {
                BGS_ASSERT( ppQueue != nullptr, "Queue (ppQueue) must be a valid address." );
                BGS_ASSERT( *ppQueue == nullptr, "There is a pointer at the given address. Queue (*ppQueue) must be nullptr." );
                if( ( ppQueue == nullptr ) || ( *ppQueue != nullptr ) )
                {
                    return Results::FAIL;
                }

                D3D12Queue* pQueue = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( m_pParent->GetParent()->GetDefaultAllocator(), &pQueue ) ) )
                {
                    return Results::NO_MEMORY;
                }
                BGS_ASSERT( pQueue != nullptr );

                if( BGS_FAILED( pQueue->Create( desc, this ) ) )
                {
                    Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), &pQueue );
                    return Results::FAIL;
                }

                ( *ppQueue ) = pQueue;

                return Results::OK;
            }

            void D3D12Device::DestroyQueue( IQueue** ppQueue )
            {
                BGS_ASSERT( ppQueue != nullptr, "Queue (ppQueue) must be a valid address." );
                BGS_ASSERT( *ppQueue != nullptr, "Queue (*ppQueue) must be a valid pointer." );
                if( ( ppQueue != nullptr ) && ( *ppQueue != nullptr ) )
                {
                    static_cast<D3D12Queue*>( *ppQueue )->Destroy();
                    Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), ppQueue );
                }
            }

            RESULT D3D12Device::CreateSwapchain( const SwapchainDesc& desc, ISwapchain** ppSwapchain )
            {
                BGS_ASSERT( ppSwapchain != nullptr, "Swapchain (ppQueue) must be a valid address." );
                BGS_ASSERT( *ppSwapchain == nullptr, "There is a pointer at the given address. Swapchain (*ppSwapchain) must be nullptr." );
                BGS_ASSERT( desc.pWindow != nullptr, "Window (desc.pWindow) must be a valid pointer." );
                BGS_ASSERT( desc.pQueue != nullptr, "Queue (desc.pQueue) must be a valid pointer." );
                BGS_ASSERT( desc.pQueue->GetDesc().type == QueueTypes::GRAPHICS, "Queue (desc.pQueue) must be a graphics queue." );
                // TODO: Better validation
                if( ( ppSwapchain == nullptr ) || ( *ppSwapchain != nullptr ) || ( desc.pWindow == nullptr ) || ( desc.pQueue == nullptr ) ||
                    ( desc.pQueue->GetDesc().type != QueueTypes::GRAPHICS ) )
                {
                    return Results::FAIL;
                }

                D3D12Swapchain* pSwapchain = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( m_pParent->GetParent()->GetDefaultAllocator(), &pSwapchain ) ) )
                {
                    return Results::NO_MEMORY;
                }
                BGS_ASSERT( pSwapchain != nullptr );

                if( BGS_FAILED( pSwapchain->Create( desc, this ) ) )
                {
                    Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), &pSwapchain );
                    return Results::FAIL;
                }

                *ppSwapchain = pSwapchain;

                return Results::OK;
            }

            void D3D12Device::DestroySwapchain( ISwapchain** ppSwapchain )
            {
                BGS_ASSERT( ppSwapchain != nullptr, "Swapchain (ppSwapchain) must be a valid address." );
                BGS_ASSERT( *ppSwapchain != nullptr, "Swapchain (*ppSwapchain) must be a valid pointer." );
                if( ( ppSwapchain != nullptr ) && ( *ppSwapchain != nullptr ) )
                {
                    static_cast<D3D12Swapchain*>( *ppSwapchain )->Destroy();
                    Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), ppSwapchain );
                }
            }

            RESULT D3D12Device::CreateCommandPool( const CommandPoolDesc& desc, CommandPoolHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Command pool (pHandle) must be a valid address." );
                BGS_ASSERT( desc.pQueue != nullptr, "Queue (desc.pQueue) must be a valid pointer." );
                if( desc.pQueue == nullptr )
                {
                    return Results::FAIL;
                }

                ID3D12Device*           pNativeDevice      = m_handle.GetNativeHandle();
                ID3D12CommandAllocator* pNativeCommandPool = nullptr;

                if( FAILED( pNativeDevice->CreateCommandAllocator( MapBigosQueueTypeToD3D12CommandListType( desc.pQueue->GetDesc().type ),
                                                                   IID_PPV_ARGS( &pNativeCommandPool ) ) ) )
                {
                    return Results::FAIL;
                }

                *pHandle = CommandPoolHandle( pNativeCommandPool );

                return Results::OK;
            }

            void D3D12Device::DestroyCommandPool( CommandPoolHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Command pool (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != CommandPoolHandle(), "Command pool (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != CommandPoolHandle() ) )
                {
                    ID3D12CommandAllocator* pNativeCommandPool = pHandle->GetNativeHandle();

                    RELEASE_COM_PTR( pNativeCommandPool );

                    *pHandle = CommandPoolHandle();
                }
            }

            RESULT D3D12Device::ResetCommandPool( CommandPoolHandle handle )
            {
                BGS_ASSERT( handle != CommandPoolHandle(), "Command pool (handle) must be a valid handle." );
                if( handle == CommandPoolHandle() )
                {
                    return Results::FAIL;
                }

                ID3D12CommandAllocator* pNativeCommandPool = handle.GetNativeHandle();
                if( FAILED( pNativeCommandPool->Reset() ) )
                {
                    return Results::FAIL;
                }

                return Results::OK;
            }

            RESULT D3D12Device::CreateCommandBuffer( const CommandBufferDesc& desc, ICommandBuffer** ppCommandBuffer )
            {
                BGS_ASSERT( ppCommandBuffer != nullptr, "Command buffer (ppCommandBuffer) must be a valid address." );
                BGS_ASSERT( *ppCommandBuffer == nullptr,
                            "There is a pointer at the given address. Command buffer (*ppCommandBuffer) must be nullptr." );
                if( ( ppCommandBuffer == nullptr ) || ( *ppCommandBuffer != nullptr ) )
                {
                    return Results::FAIL;
                }

                D3D12CommandBuffer* pCommandBuffer = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( m_pParent->GetParent()->GetDefaultAllocator(), &pCommandBuffer ) ) )
                {
                    return Results::NO_MEMORY;
                }
                BGS_ASSERT( pCommandBuffer != nullptr );

                if( BGS_FAILED( pCommandBuffer->Create( desc, this ) ) )
                {
                    Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), &pCommandBuffer );
                    return Results::FAIL;
                }

                *ppCommandBuffer = pCommandBuffer;

                return Results::OK;
            }

            void D3D12Device::DestroyCommandBuffer( ICommandBuffer** ppCommandBuffer )
            {
                BGS_ASSERT( ppCommandBuffer != nullptr, "Command buffer (ppCommandBuffer) must be a valid address." );
                BGS_ASSERT( *ppCommandBuffer != nullptr, "Command buffer (*ppCommandBuffer) must be a valid pointer." );
                if( ( ppCommandBuffer != nullptr ) && ( *ppCommandBuffer != nullptr ) )
                {
                    static_cast<D3D12CommandBuffer*>( *ppCommandBuffer )->Destroy();
                    Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), ppCommandBuffer );
                }
            }

            RESULT D3D12Device::CreateShader( const ShaderDesc& desc, ShaderHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Shader (pHandle) must be a valid address." );
                BGS_ASSERT( desc.pByteCode != nullptr, "Source code (desc.pByteCode) must be a valid pointer." );
                if( desc.pByteCode == nullptr )
                {
                    return Results::FAIL;
                }

                D3D12ShaderModule* pNativeShader = nullptr;
                const uint32_t     allocSize     = sizeof( D3D12ShaderModule ) + desc.codeSize;
                byte_t*            pMem          = nullptr;
                if( BGS_FAILED( Core::Memory::AllocateBytes( m_pParent->GetParent()->GetDefaultAllocator(), &pMem, allocSize ) ) )
                {
                    return Results::NO_MEMORY;
                }

                pNativeShader            = reinterpret_cast<D3D12ShaderModule*>( pMem );
                pNativeShader->pByteCode = pMem + sizeof( D3D12ShaderModule );
                pNativeShader->codeSize  = desc.codeSize;

                Core::Memory::Copy( desc.pByteCode, desc.codeSize, pNativeShader->pByteCode, pNativeShader->codeSize );

                *pHandle = ShaderHandle( pNativeShader );

                return Results::OK;
            }

            void D3D12Device::DestroyShader( ShaderHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Shader (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != ShaderHandle(), "Shader (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != ShaderHandle() ) )
                {
                    D3D12ShaderModule* pNativeShader = pHandle->GetNativeHandle();

                    Core::Memory::Free( m_pParent->GetParent()->GetDefaultAllocator(), &pNativeShader );

                    *pHandle = ShaderHandle();
                }
            }

            RESULT D3D12Device::CreatePipelineLayout( const PipelineLayoutDesc& desc, PipelineLayoutHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Pipeline layout (pHandle) must be a valid address." );
                BGS_ASSERT( desc.constantRangeCount <= BGS_ENUM_COUNT( ShaderVisibilities ),
                            "Push constant range count (desc.constantRangeCount) must be less than %d.", BGS_ENUM_COUNT( ShaderVisibilities ) );
                BGS_ASSERT( desc.hBindigHeapLayout != BindingHeapLayoutHandle(),
                            "Binding heap layout (desc.hBindigHeapLayout) must be valid handle." )
                if( ( desc.constantRangeCount > BGS_ENUM_COUNT( ShaderVisibilities ) ) || ( desc.hBindigHeapLayout == BindingHeapLayoutHandle() ) )
                {
                    return Results::FAIL;
                }

                // TODO: D3D12 feature support helper same as in vulkan
                D3D12_ROOT_PARAMETER1 rootParameters[ BGS_ENUM_COUNT( ShaderVisibilities ) + 1 + 1 ]; // each for one of shader visibility, one for
                                                                                                      // sampler table and one for shader resources
                const D3D12_STATIC_SAMPLER_DESC* pImmutableSamplers  = nullptr;
                uint32_t                         immutableSamplerCnt = 0;
                uint32_t                         rootParamCnt        = 0;
                bool_t                           createEmpty         = BGS_TRUE;

                const D3D12BindingHeapLayout& resourceLayout      = *desc.hBindigHeapLayout.GetNativeHandle();
                D3D12_ROOT_PARAMETER1&        samplerTable        = rootParameters[ rootParamCnt++ ];
                D3D12_ROOT_PARAMETER1&        shaderResourceTable = rootParameters[ rootParamCnt++ ];

                samplerTable.ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                samplerTable.DescriptorTable  = resourceLayout.samplerTable;
                samplerTable.ShaderVisibility = resourceLayout.visibility;
                createEmpty                   = resourceLayout.samplerTable.NumDescriptorRanges != 0 ? BGS_FALSE : BGS_TRUE;

                shaderResourceTable.ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                shaderResourceTable.DescriptorTable  = resourceLayout.shaderResourceTable;
                shaderResourceTable.ShaderVisibility = resourceLayout.visibility;
                createEmpty                          = resourceLayout.shaderResourceTable.NumDescriptorRanges != 0 ? BGS_FALSE : BGS_TRUE;

                pImmutableSamplers  = resourceLayout.pImmutableSamplers;
                immutableSamplerCnt = resourceLayout.immutableSamplerCount;
                createEmpty         = resourceLayout.immutableSamplerCount != 0 ? BGS_FALSE : BGS_TRUE;

                if( ( desc.pConstantRanges != nullptr ) && ( desc.constantRangeCount != 0 ) )
                {
                    for( index_t ndx = 0; static_cast<uint32_t>( ndx ) < desc.constantRangeCount; ++ndx )
                    {
                        const PushConstantRangeDesc& currRange = desc.pConstantRanges[ ndx ];
                        D3D12_ROOT_PARAMETER1&       param     = rootParameters[ rootParamCnt++ ];

                        param.ParameterType            = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
                        param.Constants.Num32BitValues = currRange.constantCount;
                        param.Constants.ShaderRegister = currRange.shaderRegister;
                        param.Constants.RegisterSpace  = 0; // TODO: Handle
                        param.ShaderVisibility         = MapBigosShaderVisibilityToD3D12ShaderVisibility( currRange.visibility );
                    }

                    createEmpty = BGS_FALSE;
                }

                D3D12_ROOT_SIGNATURE_DESC1 nativeDesc;
                nativeDesc.Flags             = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // Mimicking vulkan behaviour
                nativeDesc.NumParameters     = createEmpty ? 0 : rootParamCnt;
                nativeDesc.pParameters       = createEmpty ? nullptr : rootParameters;
                nativeDesc.NumStaticSamplers = createEmpty ? 0 : immutableSamplerCnt;
                nativeDesc.pStaticSamplers   = createEmpty ? nullptr : pImmutableSamplers;

                ID3D12Device*        pNativeDevice        = m_handle.GetNativeHandle();
                ID3D12RootSignature* pNativeRootSignature = nullptr;
                ID3DBlob*            pSignature           = nullptr;
                ID3DBlob*            pError               = nullptr;

                D3D12_VERSIONED_ROOT_SIGNATURE_DESC serializeDesc;
                serializeDesc.Version  = D3D_ROOT_SIGNATURE_VERSION_1_1;
                serializeDesc.Desc_1_1 = nativeDesc;

                if( FAILED( D3D12SerializeVersionedRootSignature( &serializeDesc, &pSignature, &pError ) ) )
                {
                    char  buffer[ 2048 ];
                    char* pBuffer = buffer;
                    Core::Utils::String::Copy( pBuffer, static_cast<size_t>( pError->GetBufferSize() ),
                                               static_cast<const char*>( pError->GetBufferPointer() ) );
                    printf( "%s", buffer );
                    return Results::FAIL;
                }
                if( FAILED( pNativeDevice->CreateRootSignature( 0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(),
                                                                IID_PPV_ARGS( &pNativeRootSignature ) ) ) )
                {
                    RELEASE_COM_PTR( pSignature );
                    RELEASE_COM_PTR( pError );
                    return Results::FAIL;
                }
                RELEASE_COM_PTR( pSignature );
                RELEASE_COM_PTR( pError );

                *pHandle = PipelineLayoutHandle( pNativeRootSignature );

                return Results::OK;
            }

            void D3D12Device::DestroyPipelineLayout( PipelineLayoutHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Pipeline layout (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != PipelineLayoutHandle(), "Pipeline layout (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != PipelineLayoutHandle() ) )
                {
                    ID3D12RootSignature* pNativeRootSignature = pHandle->GetNativeHandle();

                    RELEASE_COM_PTR( pNativeRootSignature );

                    *pHandle = PipelineLayoutHandle();
                }
            }

            RESULT D3D12Device::CreatePipeline( const PipelineDesc& desc, PipelineHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Pipeline (pHandle) must be a valid address." );
                BGS_ASSERT( desc.hPipelineLayout != PipelineLayoutHandle(), "Pipeline layout (desc.hPipelineLayout) must be valid handle." );
                if( ( pHandle == nullptr ) && ( desc.hPipelineLayout == PipelineLayoutHandle() ) )
                {
                    return Results::FAIL;
                }

                D3D12Pipeline* pNativePipeline = nullptr;

                switch( desc.type )
                {
                    case PipelineTypes::GRAPHICS:
                    {
                        const GraphicsPipelineDesc& gpDesc = static_cast<const GraphicsPipelineDesc&>( desc );

                        if( BGS_FAILED( CreateD3D12GraphicsPipeline( gpDesc, &pNativePipeline ) ) )
                        {
                            return Results::FAIL;
                        }

                        break;
                    }
                    case PipelineTypes::COMPUTE:
                    {
                        const ComputePipelineDesc& cpDesc = static_cast<const ComputePipelineDesc&>( desc );

                        if( BGS_FAILED( CreateD3D12ComputePipeline( cpDesc, &pNativePipeline ) ) )
                        {
                            return Results::FAIL;
                        }

                        break;
                    }
                    case PipelineTypes::RAY_TRACING:
                    {
                        // TODO: Implement
                        return Results::FAIL;
                    }
                }

                *pHandle = PipelineHandle( pNativePipeline );
                return Results::OK;
            }

            void D3D12Device::DestroyPipeline( PipelineHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Pipeline (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != PipelineHandle(), "Pipeline (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != PipelineHandle() ) )
                {
                    D3D12Pipeline* pNativePipeline = pHandle->GetNativeHandle();

                    RELEASE_COM_PTR( pNativePipeline->pPipeline );
                    Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), &pNativePipeline );

                    *pHandle = PipelineHandle();
                }
            }

            RESULT D3D12Device::CreateFence( const FenceDesc& desc, FenceHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Fence (pHandle) must be a valid address." );

                ID3D12Device* pNativeDevice = m_handle.GetNativeHandle();
                D3D12Fence*   pNativeFence  = nullptr;

                if( BGS_FAILED( Memory::AllocateObject( m_pParent->GetParent()->GetDefaultAllocator(), &pNativeFence ) ) )
                {
                    return Results::NO_MEMORY;
                }

                /*
                Params:
                  - LPSECURITY_ATTRIBUTES - ptr to struct containing security description, if null there is default
                    initialization which meets our needs
                  - bool - indicate if event needs manual reset. In our case we do not need manual reset (autoreset
                    after trigger is fine)
                  - bool - initial state, false is not signaled
                  - LPCSTR - event name
                */
                pNativeFence->hEvent = CreateEvent( nullptr, FALSE, FALSE, nullptr );

                if( FAILED( pNativeDevice->CreateFence( desc.initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &pNativeFence->pFence ) ) ) )
                {
                    CloseHandle( pNativeFence->hEvent );
                    Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), &pNativeFence );
                    return Results::FAIL;
                }

                *pHandle = FenceHandle( pNativeFence );

                return Results::OK;
            }

            void D3D12Device::DestroyFence( FenceHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Fence (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != FenceHandle(), "Fence (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != FenceHandle() ) )
                {
                    D3D12Fence* pNativeFence = pHandle->GetNativeHandle();

                    CloseHandle( pNativeFence->hEvent );
                    RELEASE_COM_PTR( pNativeFence->pFence );
                    Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), &pNativeFence );

                    *pHandle = FenceHandle();
                }
            }

            RESULT D3D12Device::WaitForFences( const WaitForFencesDesc& desc, uint64_t timeout )
            {
                BGS_ASSERT( desc.pFences != nullptr, "Fence (desc.pFences) must be a valid pointer." );
                BGS_ASSERT( desc.pWaitValues != nullptr, "Uint array (desc.pWaitValues) must be a valid address." );
                BGS_ASSERT( desc.fenceCount < Config::Driver::Synchronization::MAX_FENCES_TO_WAIT_COUNT,
                            "Fence count (desc.fenceCount) must be less than %d", Config::Driver::Synchronization::MAX_FENCES_TO_WAIT_COUNT );
                if( ( desc.pFences == nullptr ) || ( desc.pWaitValues == nullptr ) ||
                    ( desc.fenceCount >= Config::Driver::Synchronization::MAX_FENCES_TO_WAIT_COUNT ) )
                {
                    return Results::FAIL;
                }

                D3D12Fence* pNativeFence = nullptr;
                uint32_t    eventCnt     = 0;
                HANDLE      events[ Config::Driver::Synchronization::MAX_FENCES_TO_WAIT_COUNT ];
                for( index_t ndx = 0; ndx < static_cast<index_t>( desc.fenceCount ); ++ndx )
                {
                    if( desc.pFences[ ndx ] == FenceHandle() )
                    {
                        return Results::FAIL;
                    }
                    // Setting event only for fences which has lower internal value than wait value
                    pNativeFence = desc.pFences[ ndx ].GetNativeHandle();
                    if( pNativeFence->pFence->GetCompletedValue() < desc.pWaitValues[ ndx ] )
                    {
                        pNativeFence->pFence->SetEventOnCompletion( desc.pWaitValues[ ndx ], pNativeFence->hEvent );
                        events[ eventCnt++ ] = pNativeFence->hEvent;
                    }
                }
                // Waiting for necessary events
                if( WaitForMultipleObjects( eventCnt, events, desc.waitAll, static_cast<DWORD>( timeout ) ) == WAIT_TIMEOUT )
                {
                    return Results::TIMEOUT;
                }

                return Results::OK;
            }

            RESULT D3D12Device::SignalFence( uint64_t value, FenceHandle handle )
            {
                BGS_ASSERT( handle != FenceHandle(), "Fence (handle) must be a valid handle." );
                if( handle == FenceHandle() )
                {
                    return Results::FAIL;
                }

                D3D12Fence* pNativeFence = handle.GetNativeHandle();
                if( FAILED( pNativeFence->pFence->Signal( value ) ) )
                {
                    return Results::FAIL;
                }

                return Results::OK;
            }

            RESULT D3D12Device::GetFenceValue( FenceHandle handle, uint64_t* pValue )
            {
                BGS_ASSERT( handle != FenceHandle(), "Fence (handle) must be a valid handle." );
                if( handle == FenceHandle() )
                {
                    return Results::FAIL;
                }

                D3D12Fence* pNativeFence = handle.GetNativeHandle();
                *pValue                  = pNativeFence->pFence->GetCompletedValue();

                return Results::OK;
            }

            RESULT D3D12Device::CreateSemaphore( const SemaphoreDesc& desc, SemaphoreHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Semaphore (pHandle) must be a valid address." );
                desc;

                ID3D12Device* pNativeDevice    = m_handle.GetNativeHandle();
                ID3D12Fence*  pNativeSemaphore = nullptr;

                if( FAILED( pNativeDevice->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &pNativeSemaphore ) ) ) )
                {
                    return Results::FAIL;
                }

                *pHandle = SemaphoreHandle( pNativeSemaphore );

                return Results::OK;
            }

            void D3D12Device::DestroySemaphore( SemaphoreHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Semaphore (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != SemaphoreHandle(), "Semaphore (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != SemaphoreHandle() ) )
                {
                    ID3D12Fence* pNativeSemaphore = pHandle->GetNativeHandle();

                    RELEASE_COM_PTR( pNativeSemaphore );

                    *pHandle = SemaphoreHandle();
                }
            }

            RESULT D3D12Device::AllocateMemory( const AllocateMemoryDesc& desc, MemoryHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Memory (pHandle) must be a valid address." );

                D3D12_HEAP_DESC nativeHeapDesc;
                CreateD3D12MemoryHeapProperties( desc, &nativeHeapDesc.Properties );
                nativeHeapDesc.Flags       = MapBigosMemoryHeapUsageToD3D12HeapFlags( desc.heapUsage );
                nativeHeapDesc.SizeInBytes = desc.size;
                nativeHeapDesc.Alignment   = desc.alignment;

                ID3D12Device* pNativeDevice = m_handle.GetNativeHandle();
                ID3D12Heap*   pNativeHeap   = nullptr;

                if( FAILED( pNativeDevice->CreateHeap( &nativeHeapDesc, IID_PPV_ARGS( &pNativeHeap ) ) ) )
                {
                    return Results::FAIL;
                }

                *pHandle = MemoryHandle( pNativeHeap );

                return Results::OK;
            }

            void D3D12Device::FreeMemory( MemoryHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Memory (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != MemoryHandle(), "Memory (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != MemoryHandle() ) )
                {
                    ID3D12Heap* pNativeHeap = pHandle->GetNativeHandle();

                    RELEASE_COM_PTR( pNativeHeap );

                    *pHandle = MemoryHandle();
                }
            }

            RESULT D3D12Device::CreateResource( const ResourceDesc& desc, ResourceHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Resource (pHandle) must be a valid address." );
                // TODO: Think about validation, because things ale getting complex

                // In D3D12 we only fill resource desc in this function. Resource will be created while binding to memory (mimicing vulkan
                // behaviour)
                D3D12Resource* pResource = nullptr;
                if( BGS_FAILED( Core::Memory::AllocateObject( m_pParent->GetParent()->GetDefaultAllocator(), &pResource ) ) )
                {
                    return Results::NO_MEMORY;
                }

                D3D12_RESOURCE_DESC& nativeDesc = pResource->desc;
                nativeDesc.Dimension            = MapBigosResourceTypeToD3D12ResourceDimension( desc.resourceType );
                nativeDesc.Alignment            = 0; // TODO: Rethik
                nativeDesc.Width                = desc.size.width;
                nativeDesc.Height               = desc.size.height;
                nativeDesc.DepthOrArraySize     = desc.resourceType == ResourceTypes::TEXTURE_3D ? static_cast<uint16_t>( desc.size.depth )
                                                                                                 : static_cast<uint16_t>( desc.arrayLayerCount );
                nativeDesc.MipLevels            = static_cast<uint16_t>( desc.mipLevelCount );
                nativeDesc.Format               = MapBigosFormatToD3D12Format( desc.format );
                nativeDesc.SampleDesc.Count     = MapBigosSampleCountToD3D12Uint( desc.sampleCount );
                nativeDesc.SampleDesc.Quality   = 0; // TODO: Handle (same for pipeline state)
                // In most case it is the best solution. Textures can be created with D3D12_TEXTURE_LAYOUT_ROW_MAJOR
                // but only if memory heap can be shared across adapters.
                nativeDesc.Layout = desc.resourceType == ResourceTypes::BUFFER ? D3D12_TEXTURE_LAYOUT_ROW_MAJOR : D3D12_TEXTURE_LAYOUT_UNKNOWN;
                nativeDesc.Flags  = MapBigosResourceUsageFlagsToD3D12ResourceFlags( desc.resourceUsage );
                if( desc.sharingMode == ResourceSharingModes::SIMULTANEOUS_ACCESS )
                {
                    nativeDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
                }

                pResource->pNativeResource = nullptr;

                *pHandle = ResourceHandle( pResource );

                return Results::OK;
            }

            void D3D12Device::DestroyResource( ResourceHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Resource (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != ResourceHandle(), "Resource (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != ResourceHandle() ) )
                {
                    D3D12Resource* pNativeResource = pHandle->GetNativeHandle();

                    RELEASE_COM_PTR( pNativeResource->pNativeResource );

                    Core::Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), &pNativeResource );

                    *pHandle = ResourceHandle();
                }
            }

            RESULT D3D12Device::BindResourceMemory( const BindResourceMemoryDesc& desc )
            {
                BGS_ASSERT( desc.hResource != ResourceHandle(), "Resource (desc.hResource) must be a valid handle." )
                BGS_ASSERT( desc.hResource.GetNativeHandle() != nullptr, "Resource (desc.hResource) must hold valid internal resource." );
                BGS_ASSERT( desc.hMemory != MemoryHandle(), "Memory (desc.hMemory) must be a valid handle." )
                BGS_ASSERT( desc.hMemory.GetNativeHandle() != nullptr, "Memory (desc.hMemory) must hold valid internal memory." );
                if( ( desc.hResource == ResourceHandle() ) && ( desc.hResource.GetNativeHandle() == nullptr ) && ( desc.hMemory == MemoryHandle() ) &&
                    ( desc.hMemory.GetNativeHandle() == nullptr ) )
                {
                    return Results::FAIL;
                }

                // Creating D3D12 resource here.
                ID3D12Device* pNativeDevice = m_handle.GetNativeHandle();
                if( FAILED( pNativeDevice->CreatePlacedResource( desc.hMemory.GetNativeHandle(), desc.memoryOffset,
                                                                 &desc.hResource.GetNativeHandle()->desc, D3D12_RESOURCE_STATE_COMMON, nullptr,
                                                                 IID_PPV_ARGS( &desc.hResource.GetNativeHandle()->pNativeResource ) ) ) )
                {
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void D3D12Device::GetResourceAllocationInfo( ResourceHandle handle, ResourceAllocationInfo* pInfo )
            {
                BGS_ASSERT( handle != ResourceHandle(), "Resource (handle) must be a valid handle." );
                BGS_ASSERT( handle.GetNativeHandle() != nullptr, "Resource (handle) must hold valid internal resource." );
                BGS_ASSERT( pInfo != nullptr, "Resource allocation info (pInfo) must be a valid address." );
                if( ( handle != ResourceHandle() ) && ( handle.GetNativeHandle() != nullptr ) && ( pInfo != nullptr ) )
                {
                    ID3D12Device*                         pNativeDevice = m_handle.GetNativeHandle();
                    const D3D12_RESOURCE_ALLOCATION_INFO& allocInfo =
                        pNativeDevice->GetResourceAllocationInfo( 0, 1, &handle.GetNativeHandle()->desc );

                    pInfo->alignment = allocInfo.Alignment;
                    pInfo->size      = allocInfo.SizeInBytes;
                }
            }

            RESULT D3D12Device::MapResource( const MapResourceDesc& desc, void** ppResource )
            {
                BGS_ASSERT( desc.hResource != ResourceHandle(), "Resource (hResource) must be a valid handle." );
                BGS_ASSERT( desc.hResource.GetNativeHandle() != nullptr, "Resource (hResource) must hold valid internal resource." );
                BGS_ASSERT( ppResource != nullptr, "Memory (ppResource) must be a valid address." );
                if( ( desc.hResource == ResourceHandle() ) && ( ppResource == nullptr ) )
                {
                    return Results::FAIL;
                }

                ID3D12Resource*            pNativeResource = desc.hResource.GetNativeHandle()->pNativeResource;
                const D3D12_RESOURCE_DESC& resDesc         = desc.hResource.GetNativeHandle()->desc;
                uint32_t                   subResNdx       = 0;
                D3D12_RANGE*               pResRange       = nullptr;
                D3D12_RANGE                resRange;
                if( resDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER )
                {
                    // From D3D12 docs: MipSlice + ArraySlice * MipLevels + PlaneSlice * MipLevels * ArraySize;
                    subResNdx = desc.textureRange.mipLevel + desc.textureRange.arrayLayer * desc.textureRange.mipLevelCount +
                                MapBigosTextureComponentFlagsToD3D12PlaneSlice( desc.textureRange.components ) * desc.textureRange.mipLevelCount *
                                    desc.textureRange.arrayLayerCount;
                }
                else // Buffers
                {
                    resRange.Begin = desc.bufferRange.offset;
                    resRange.End   = desc.bufferRange.offset + desc.bufferRange.size;
                    pResRange      = &resRange;
                }

                if( FAILED( pNativeResource->Map( subResNdx, pResRange, ppResource ) ) )
                {
                    return Results::FAIL;
                }

                return Results::OK;
            }

            RESULT D3D12Device::UnmapResource( const MapResourceDesc& desc )
            {
                BGS_ASSERT( desc.hResource != ResourceHandle(), "Resource (hResource) must be a valid handle." );
                BGS_ASSERT( desc.hResource.GetNativeHandle() != nullptr, "Resource (hResource) must hold valid internal resource." );
                if( desc.hResource == ResourceHandle() )
                {
                    return Results::FAIL;
                }

                ID3D12Resource*            pNativeResource = desc.hResource.GetNativeHandle()->pNativeResource;
                const D3D12_RESOURCE_DESC& resDesc         = desc.hResource.GetNativeHandle()->desc;
                uint32_t                   subResNdx       = 0;
                D3D12_RANGE*               pResRange       = nullptr;
                D3D12_RANGE                resRange;
                if( resDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER )
                {
                    // From D3D12 docs: MipSlice + ArraySlice * MipLevels + PlaneSlice * MipLevels * ArraySize;
                    subResNdx = desc.textureRange.mipLevel + desc.textureRange.arrayLayer * desc.textureRange.mipLevelCount +
                                MapBigosTextureComponentFlagsToD3D12PlaneSlice( desc.textureRange.components ) * desc.textureRange.mipLevelCount *
                                    desc.textureRange.arrayLayerCount;
                }
                else
                {
                    resRange.Begin = desc.bufferRange.offset;
                    resRange.End   = desc.bufferRange.offset + desc.bufferRange.size;
                    pResRange      = &resRange;
                }

                pNativeResource->Unmap( subResNdx, pResRange );

                return Results::OK;
            }

            RESULT D3D12Device::CreateResourceView( const ResourceViewDesc& desc, ResourceViewHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Resource view (pHandle) must be a valid address." );
                if( ( pHandle == nullptr ) ) // TODO: Validation
                {
                    return Results::FAIL;
                }

                D3D12ResourceView* pResView = nullptr;
                if( BGS_FAILED( Core::Memory::AllocateObject( m_pParent->GetParent()->GetDefaultAllocator(), &pResView ) ) )
                {
                    return Results::NO_MEMORY;
                }

                if( desc.usage & BGS_FLAG( ResourceViewUsageFlagBits::COLOR_RENDER_TARGET ) )
                {
                    const TextureViewDesc& texDesc = static_cast<const TextureViewDesc&>( desc );
                    pResView->rtvNdx               = CreateD3D12RTV( texDesc );
                    pResView->type                 = D3D12DescriptorTypes::RTV;
                }
                else if( desc.usage & BGS_FLAG( ResourceViewUsageFlagBits::DEPTH_STENCIL_TARGET ) )
                {
                    const TextureViewDesc& texDesc = static_cast<const TextureViewDesc&>( desc );
                    pResView->dsvNdx               = CreateD3D12DSV( texDesc );
                    pResView->type                 = D3D12DescriptorTypes::DSV;
                }
                else if( desc.usage & BGS_FLAG( ResourceViewUsageFlagBits::CONSTANT_BUFFER ) )
                {
                    const BufferViewDesc& buffDesc = static_cast<const BufferViewDesc&>( desc );
                    pResView->cbvNdx               = CreateD3D12CBV( buffDesc );
                    pResView->type                 = D3D12DescriptorTypes::CBV;
                }
                else if( desc.usage & BGS_FLAG( ResourceViewUsageFlagBits::SAMPLED_TEXTURE ) )
                {
                    const TextureViewDesc& texDesc = static_cast<const TextureViewDesc&>( desc );
                    pResView->srvNdx               = CreateD3D12SRV( texDesc );
                    pResView->type                 = D3D12DescriptorTypes::SRV;
                }
                else if( desc.usage & BGS_FLAG( ResourceViewUsageFlagBits::STORAGE_TEXTURE ) )
                {
                    const TextureViewDesc& texDesc = static_cast<const TextureViewDesc&>( desc );
                    pResView->uavNdx               = CreateD3D12UAV( texDesc );
                    pResView->type                 = D3D12DescriptorTypes::UAV;
                }
                else if( desc.usage & BGS_FLAG( ResourceViewUsageFlagBits::CONSTANT_TEXEL_BUFFER ) )
                {
                    const TexelBufferViewDesc& buffDesc = static_cast<const TexelBufferViewDesc&>( desc );
                    pResView->srvNdx                    = CreateD3D12SRV( buffDesc );
                    pResView->type                      = D3D12DescriptorTypes::SRV;
                }
                else if( desc.usage & BGS_FLAG( ResourceViewUsageFlagBits::STORAGE_TEXEL_BUFFER ) )
                {
                    const TexelBufferViewDesc& buffDesc = static_cast<const TexelBufferViewDesc&>( desc );
                    pResView->uavNdx                    = CreateD3D12UAV( buffDesc );
                    pResView->type                      = D3D12DescriptorTypes::UAV;
                }
                else if( desc.usage & BGS_FLAG( ResourceViewUsageFlagBits::READ_ONLY_STORAGE_BUFFER ) )
                {
                    const BufferViewDesc& buffDesc = static_cast<const BufferViewDesc&>( desc );
                    pResView->srvNdx               = CreateD3D12SRV( buffDesc );
                    pResView->type                 = D3D12DescriptorTypes::SRV;
                }
                else if( desc.usage & BGS_FLAG( ResourceViewUsageFlagBits::READ_WRITE_STORAGE_BUFFER ) )
                {
                    const BufferViewDesc& buffDesc = static_cast<const BufferViewDesc&>( desc );
                    pResView->uavNdx               = CreateD3D12UAV( buffDesc );
                    pResView->type                 = D3D12DescriptorTypes::UAV;
                }

                *pHandle = ResourceViewHandle( pResView );

                return Results::OK;
            }

            void D3D12Device::DestroyResourceView( ResourceViewHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Resource view (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != ResourceViewHandle(), "Resource view (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != ResourceViewHandle() ) )
                {
                    D3D12ResourceView* pNativeView = pHandle->GetNativeHandle();
                    if( pNativeView->type == D3D12DescriptorTypes::CBV )
                    {
                        m_descHeaps[ static_cast<uint32_t>( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ) ].allocator.Free( pNativeView->cbvNdx );
                    }
                    else if( pNativeView->type == D3D12DescriptorTypes::SRV )
                    {
                        m_descHeaps[ static_cast<uint32_t>( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ) ].allocator.Free( pNativeView->srvNdx );
                    }
                    else if( pNativeView->type == D3D12DescriptorTypes::UAV )
                    {
                        m_descHeaps[ static_cast<uint32_t>( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ) ].allocator.Free( pNativeView->uavNdx );
                    }
                    else if( pNativeView->type == D3D12DescriptorTypes::RTV )
                    {
                        m_descHeaps[ static_cast<uint32_t>( D3D12_DESCRIPTOR_HEAP_TYPE_RTV ) ].allocator.Free( pNativeView->rtvNdx );
                    }
                    else if( pNativeView->type == D3D12DescriptorTypes::DSV )
                    {
                        m_descHeaps[ static_cast<uint32_t>( D3D12_DESCRIPTOR_HEAP_TYPE_DSV ) ].allocator.Free( pNativeView->dsvNdx );
                    }

                    Core::Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), &pNativeView );

                    *pHandle = ResourceViewHandle();
                }
            }

            RESULT D3D12Device::CreateSampler( const SamplerDesc& desc, SamplerHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Sampler (pHandle) must be a valid address." );
                if( ( pHandle == nullptr ) ) // TODO: Validation
                {
                    return Results::FAIL;
                }

                D3D12Sampler* pSampler = nullptr;
                if( BGS_FAILED( Core::Memory::AllocateObject( m_pParent->GetParent()->GetDefaultAllocator(), &pSampler ) ) )
                {
                    return Results::NO_MEMORY;
                }

                pSampler->sampler.Filter         = MapBigosFiltesToD3D12Filter( desc.minFilter, desc.magFilter, desc.mipMapFilter, desc.reductionMode,
                                                                                desc.anisotropyEnable, desc.compareEnable );
                pSampler->sampler.AddressU       = MapBigosTextureAddressModeToD3D12TextureAddressMode( desc.addressU );
                pSampler->sampler.AddressV       = MapBigosTextureAddressModeToD3D12TextureAddressMode( desc.addressV );
                pSampler->sampler.AddressW       = MapBigosTextureAddressModeToD3D12TextureAddressMode( desc.addressW );
                pSampler->sampler.MaxAnisotropy  = desc.anisotropyEnable == BGS_FALSE ? 1 : static_cast<UINT>( desc.maxAnisotropy );
                pSampler->sampler.MipLODBias     = desc.mipLodBias;
                pSampler->sampler.MinLOD         = desc.minLod;
                pSampler->sampler.MaxLOD         = desc.maxLod;
                pSampler->sampler.ComparisonFunc = MapBigosCompareOperationTypeToD3D12ComparsionFunc( desc.compareOperation );
                if( desc.type == SamplerTypes::NORMAL )
                {
                    pSampler->sampler.BorderColor[ 0 ] = desc.customBorderColor.r;
                    pSampler->sampler.BorderColor[ 1 ] = desc.customBorderColor.g;
                    pSampler->sampler.BorderColor[ 2 ] = desc.customBorderColor.b;
                    pSampler->sampler.BorderColor[ 3 ] = desc.customBorderColor.a;
                }
                else // desc.type == SamplerTypes::IMMUTABLE
                {
                    pSampler->staticColor = MapBigosBorderColorToD3D12StaticBorderColor( desc.enumBorderColor );
                }

                *pHandle = SamplerHandle( pSampler );

                return Results::OK;
            }

            void D3D12Device::DestroySampler( SamplerHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Sampler (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != SamplerHandle(), "Sampler (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != SamplerHandle() ) )
                {
                    D3D12Sampler* pNativeSampler = pHandle->GetNativeHandle();
                    Core::Memory::FreeObject( m_pParent->GetParent()->GetDefaultAllocator(), &pNativeSampler );

                    *pHandle = SamplerHandle();
                }
            }

            RESULT D3D12Device::CreateBindingHeapLayout( const BindingHeapLayoutDesc& desc, BindingHeapLayoutHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Binding heap layout (pHandle) must be a valid address." );
                BGS_ASSERT( desc.bindingRangeCount <= Config::Driver::Pipeline::MAX_BINDING_RANGE_COUNT,
                            "Binding count (desc.bindingCount) must be less than %d", Config::Driver::Pipeline::MAX_BINDING_RANGE_COUNT );
                if( ( pHandle == nullptr ) || ( desc.bindingRangeCount > Config::Driver::Pipeline::MAX_BINDING_RANGE_COUNT ) )
                {
                    return Results::FAIL;
                }

                D3D12BindingHeapLayout* pLayout                  = nullptr;
                byte_t*                 pMem                     = nullptr;
                uint32_t                immutableSamplerCnt      = 0;
                uint32_t                immutableSamplerRangeCnt = 0;
                uint32_t                shaderResourceRangeCnt   = 0;
                uint32_t                samplerRangeCnt          = 0;

                // Calculating size of needed memory allocation
                uint32_t allocSize = 0;
                // Scanning for binding types
                for( index_t ndx = 0; static_cast<uint32_t>( ndx ) < desc.bindingRangeCount; ++ndx )
                {
                    const BindingRangeDesc& currDesc = desc.pBindingRanges[ ndx ];
                    if( ( currDesc.type == BindingTypes::SAMPLER ) && ( currDesc.immutableSampler.phSamplers != nullptr ) )
                    {
                        allocSize += currDesc.bindingCount * sizeof( D3D12_STATIC_SAMPLER_DESC );
                        immutableSamplerCnt += currDesc.bindingCount;
                        ++immutableSamplerRangeCnt;
                    }
                    else if( ( currDesc.type == BindingTypes::SAMPLER ) && ( currDesc.immutableSampler.phSamplers == nullptr ) )
                    {
                        samplerRangeCnt++;
                    }
                    else // other shader resources
                    {
                        shaderResourceRangeCnt++;
                    }
                }
                // Crash if we have to much immutable samplers
                if( immutableSamplerCnt > Config::Driver::Pipeline::MAX_IMMUTABLE_SAMPLER_COUNT )
                {
                    return Results::FAIL;
                }
                // Memory for samplers
                allocSize += samplerRangeCnt * sizeof( D3D12_DESCRIPTOR_RANGE1 );
                // Memory for shader resources
                allocSize += shaderResourceRangeCnt * sizeof( D3D12_DESCRIPTOR_RANGE1 );
                // Memory for binding set layout emulation
                allocSize += sizeof( D3D12BindingHeapLayout );

                if( BGS_FAILED( Core::Memory::AllocateBytes( m_pParent->GetParent()->GetDefaultAllocator(), &pMem, allocSize ) ) )
                {
                    return Results::NO_MEMORY;
                }
                // We want pLayout at the start of the mem block to free it easily
                pLayout = reinterpret_cast<D3D12BindingHeapLayout*>( pMem );
                pMem += sizeof( D3D12BindingHeapLayout );

                // Filling in immutable samplers
                D3D12_STATIC_SAMPLER_DESC* pImmutableSamplers = reinterpret_cast<D3D12_STATIC_SAMPLER_DESC*>( pMem );
                pMem += immutableSamplerCnt * sizeof( D3D12_STATIC_SAMPLER_DESC );
                uint32_t immutableSamplerNdx = 0;
                for( index_t ndx = 0; static_cast<uint32_t>( ndx ) < desc.bindingRangeCount; ++ndx )
                {
                    const BindingRangeDesc& currDesc = desc.pBindingRanges[ ndx ];
                    if( ( currDesc.type == BindingTypes::SAMPLER ) && ( currDesc.immutableSampler.phSamplers != nullptr ) )
                    {
                        for( index_t ndy = 0; static_cast<uint32_t>( ndy ) < currDesc.bindingCount; ++ndy )
                        {
                            D3D12_STATIC_SAMPLER_DESC& immutableSampler = pImmutableSamplers[ immutableSamplerNdx++ ];
                            const D3D12Sampler&        currSampler      = *currDesc.immutableSampler.phSamplers[ ndy ].GetNativeHandle();

                            immutableSampler.Filter         = currSampler.sampler.Filter;
                            immutableSampler.AddressU       = currSampler.sampler.AddressU;
                            immutableSampler.AddressW       = currSampler.sampler.AddressW;
                            immutableSampler.AddressV       = currSampler.sampler.AddressV;
                            immutableSampler.MipLODBias     = currSampler.sampler.MipLODBias;
                            immutableSampler.MaxAnisotropy  = currSampler.sampler.MaxAnisotropy;
                            immutableSampler.ComparisonFunc = currSampler.sampler.ComparisonFunc;
                            immutableSampler.BorderColor    = currSampler.staticColor;
                            immutableSampler.MinLOD         = currSampler.sampler.MinLOD;
                            immutableSampler.MaxLOD         = currSampler.sampler.MaxLOD;
                            immutableSampler.ShaderRegister = currDesc.baseShaderRegister + static_cast<uint32_t>( ndy );
                            immutableSampler.RegisterSpace  = 0;
                            immutableSampler.ShaderVisibility =
                                MapBigosShaderVisibilityToD3D12ShaderVisibility( currDesc.immutableSampler.visibility );
                        }
                    }
                }

                // Filling in samplers and shader resources
                D3D12_DESCRIPTOR_RANGE1* pSamplerRanges = reinterpret_cast<D3D12_DESCRIPTOR_RANGE1*>( pMem );
                pMem += samplerRangeCnt * sizeof( D3D12_DESCRIPTOR_RANGE1 );
                D3D12_DESCRIPTOR_RANGE1* pShaderResourceRanges = reinterpret_cast<D3D12_DESCRIPTOR_RANGE1*>( pMem );
                pMem += shaderResourceRangeCnt * sizeof( D3D12_DESCRIPTOR_RANGE1 );
                uint32_t samplerNdx   = 0;
                uint32_t shaderResNdx = 0;
                for( index_t ndx = 0; static_cast<uint32_t>( ndx ) < desc.bindingRangeCount; ++ndx )
                {
                    const BindingRangeDesc& currDesc = desc.pBindingRanges[ ndx ];
                    if( ( currDesc.type == BindingTypes::SAMPLER ) && ( currDesc.immutableSampler.phSamplers == nullptr ) )
                    {
                        D3D12_DESCRIPTOR_RANGE1& range          = pSamplerRanges[ samplerNdx++ ];
                        range.BaseShaderRegister                = currDesc.baseShaderRegister;
                        range.RegisterSpace                     = 0; // TODO: Handle
                        range.NumDescriptors                    = currDesc.bindingCount;
                        range.RangeType                         = MapBigosBindingTypeToD3D12DescriptorRangeType( currDesc.type );
                        range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
                        range.Flags                             = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
                    }
                    else if( currDesc.type != BindingTypes::SAMPLER )
                    {
                        D3D12_DESCRIPTOR_RANGE1& range          = pShaderResourceRanges[ shaderResNdx++ ];
                        range.BaseShaderRegister                = currDesc.baseShaderRegister;
                        range.RegisterSpace                     = 0; // TODO: Handle
                        range.NumDescriptors                    = currDesc.bindingCount;
                        range.RangeType                         = MapBigosBindingTypeToD3D12DescriptorRangeType( currDesc.type );
                        range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
                        range.Flags                             = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
                    }
                }

                // Filling in binding heap layout emulation
                pLayout->samplerTable.pDescriptorRanges          = pSamplerRanges;
                pLayout->samplerTable.NumDescriptorRanges        = samplerRangeCnt;
                pLayout->shaderResourceTable.pDescriptorRanges   = pShaderResourceRanges;
                pLayout->shaderResourceTable.NumDescriptorRanges = shaderResourceRangeCnt;
                pLayout->pImmutableSamplers                      = pImmutableSamplers;
                pLayout->immutableSamplerCount                   = immutableSamplerCnt;
                pLayout->visibility                              = MapBigosShaderVisibilityToD3D12ShaderVisibility( desc.visibility );

                *pHandle = BindingHeapLayoutHandle( pLayout );

                return Results::OK;
            }

            void D3D12Device::DestroyBindingHeapLayout( BindingHeapLayoutHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Binding heap layout (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != BindingHeapLayoutHandle(), "Binding heap layout (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != BindingHeapLayoutHandle() ) )
                {
                    D3D12BindingHeapLayout* pNativeLayout = pHandle->GetNativeHandle();
                    Core::Memory::Free( m_pParent->GetParent()->GetDefaultAllocator(), &pNativeLayout );

                    *pHandle = BindingHeapLayoutHandle();
                }
            }

            RESULT D3D12Device::CreateBindingHeap( const BindingHeapDesc& desc, BindingHeapHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Binding heap (pHandle) must be a valid address." );
                BGS_ASSERT( desc.bindingCount > 0, "Binding count (desc.bindingCount) must be more than 0." );
                if( ( pHandle == nullptr ) || ( desc.bindingCount <= 0 ) )
                {
                    return Results::FAIL;
                }

                ID3D12Device*         pNativeDevice = m_handle.GetNativeHandle();
                ID3D12DescriptorHeap* pNativeHeap   = nullptr;

                D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
                heapDesc.Type           = MapBigosBindingHeapTypeToD3D12DescriptorHeapType( desc.type );
                heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
                heapDesc.NumDescriptors = desc.bindingCount;
                heapDesc.NodeMask       = 0;

                if( FAILED( pNativeDevice->CreateDescriptorHeap( &heapDesc, IID_PPV_ARGS( &pNativeHeap ) ) ) )
                {
                    return Results::FAIL;
                }

                *pHandle = BindingHeapHandle( pNativeHeap );

                return Results::OK;
            }

            void D3D12Device::DestroyBindingHeap( BindingHeapHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Binding heap (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != BindingHeapHandle(), "Binding heap (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != BindingHeapHandle() ) )
                {
                    ID3D12DescriptorHeap* pNativeHeap = pHandle->GetNativeHandle();

                    RELEASE_COM_PTR( pNativeHeap );

                    *pHandle = BindingHeapHandle();
                }
            }

            void D3D12Device::CopyBinding( const CopyBindingDesc& desc )
            {
                desc;
            }

            RESULT D3D12Device::CreateQueryPool( const QueryPoolDesc& desc, QueryPoolHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Query pool (pHandle) must be a valid address." );
                BGS_ASSERT( desc.queryCount > 0, "Query count (desc.queryCount) must be more than 0." );
                if( ( pHandle == nullptr ) || ( desc.queryCount < 0 ) )
                {
                    return Results::FAIL;
                }

                ID3D12Device*    pNativeDevice = m_handle.GetNativeHandle();
                ID3D12QueryHeap* pNativeHeap   = nullptr;

                D3D12_QUERY_HEAP_DESC heapDesc;
                heapDesc.NodeMask = 0;
                heapDesc.Type     = MapBigosQueryTypeToD3D12QueryHeapType( desc.type );
                heapDesc.Count    = desc.queryCount;

                if( FAILED( pNativeDevice->CreateQueryHeap( &heapDesc, IID_PPV_ARGS( &pNativeHeap ) ) ) )
                {
                    return Results::FAIL;
                }

                *pHandle = QueryPoolHandle( pNativeHeap );

                return Results::OK;
            }

            void D3D12Device::DestroyQueryPool( QueryPoolHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Query pool (pHandle) must be a valid address." );
                BGS_ASSERT( *pHandle != QueryPoolHandle(), "Query pool (pHandle) must point to valid handle." );
                if( ( pHandle != nullptr ) && ( *pHandle != QueryPoolHandle() ) )
                {
                    ID3D12QueryHeap* pNativeHeap = pHandle->GetNativeHandle();

                    RELEASE_COM_PTR( pNativeHeap );

                    *pHandle = QueryPoolHandle();
                }
            }

            RESULT D3D12Device::Create( const DeviceDesc& desc, D3D12Factory* pFactory )
            {
                BGS_ASSERT( pFactory != nullptr, "Factory (pFactory) must be a valid pointer." );
                BGS_ASSERT( desc.pAdapter != nullptr, "Adapter (pAdapter) must be a valid pointer." );
                if( ( pFactory == nullptr ) || ( desc.pAdapter == nullptr ) )
                {
                    return Results::FAIL;
                }

                m_desc    = desc;
                m_pParent = pFactory;

                if( BGS_FAILED( CreateD3D12Device() ) )
                {
                    return Results::FAIL;
                }

                if( BGS_FAILED( CreateD3D12GlobalDescriptorHeaps() ) )
                {
                    ID3D12Device* pNativeDevice = m_handle.GetNativeHandle();
                    RELEASE_COM_PTR( pNativeDevice );
                    return Results::FAIL;
                }

                QueryD3D12BindingsSize();

                return Results::OK;
            }

            void D3D12Device::Destroy()
            {
                BGS_ASSERT( m_handle != DeviceHandle() );

                if( m_handle != DeviceHandle() )
                {
                    ID3D12Device* pNativeDevice = m_handle.GetNativeHandle();
                    RELEASE_COM_PTR( pNativeDevice );

                    for( index_t ndx = 0; ndx < 4; ++ndx )
                    {
                        RELEASE_COM_PTR( m_descHeaps[ ndx ].pHeap );
                        m_descHeaps[ ndx ].allocator.Destroy();
                    }
                }

                m_handle  = DeviceHandle();
                m_pParent = nullptr;
            }

            RESULT D3D12Device::CreateD3D12Device()
            {
                IDXGIAdapter1* pNativeAdapter = m_desc.pAdapter->GetHandle().GetNativeHandle();
                ID3D12Device*  pNativeDevice  = nullptr;

                if( FAILED( D3D12CreateDevice( pNativeAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &pNativeDevice ) ) ) )
                {
                    return Results::FAIL;
                }

                m_heapProperties[ BGS_ENUM_INDEX( MemoryHeapTypes::DEFAULT ) ] = pNativeDevice->GetCustomHeapProperties( 0, D3D12_HEAP_TYPE_DEFAULT );
                m_heapProperties[ BGS_ENUM_INDEX( MemoryHeapTypes::READBACK ) ] =
                    pNativeDevice->GetCustomHeapProperties( 0, D3D12_HEAP_TYPE_READBACK );
                m_heapProperties[ BGS_ENUM_INDEX( MemoryHeapTypes::UPLOAD ) ] = pNativeDevice->GetCustomHeapProperties( 0, D3D12_HEAP_TYPE_UPLOAD );
                m_heapProperties[ BGS_ENUM_INDEX( MemoryHeapTypes::CUSTOM ) ] = {}; // TODO: Handle

                m_handle = DeviceHandle( pNativeDevice );

                return Results::OK;
            }

            RESULT D3D12Device::CreateD3D12GraphicsPipeline( const GraphicsPipelineDesc& gpDesc, D3D12Pipeline** ppPipeline )
            {
                D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
                // Shader stages
                if( gpDesc.vertexShader.hShader != ShaderHandle() )
                {
                    D3D12ShaderModule* pShader = gpDesc.vertexShader.hShader.GetNativeHandle();
                    psoDesc.VS.pShaderBytecode = pShader->pByteCode;
                    psoDesc.VS.BytecodeLength  = pShader->codeSize;
                }
                else
                {
                    psoDesc.VS.pShaderBytecode = nullptr;
                    psoDesc.VS.BytecodeLength  = 0;
                }
                if( gpDesc.pixelShader.hShader != ShaderHandle() )
                {
                    D3D12ShaderModule* pShader = gpDesc.pixelShader.hShader.GetNativeHandle();
                    psoDesc.PS.pShaderBytecode = pShader->pByteCode;
                    psoDesc.PS.BytecodeLength  = pShader->codeSize;
                }
                else
                {
                    psoDesc.PS.pShaderBytecode = nullptr;
                    psoDesc.PS.BytecodeLength  = 0;
                }
                if( gpDesc.domainShader.hShader != ShaderHandle() )
                {
                    D3D12ShaderModule* pShader = gpDesc.domainShader.hShader.GetNativeHandle();
                    psoDesc.DS.pShaderBytecode = pShader->pByteCode;
                    psoDesc.DS.BytecodeLength  = pShader->codeSize;
                }
                else
                {
                    psoDesc.DS.pShaderBytecode = nullptr;
                    psoDesc.DS.BytecodeLength  = 0;
                }
                if( gpDesc.hullShader.hShader != ShaderHandle() )
                {
                    D3D12ShaderModule* pShader = gpDesc.hullShader.hShader.GetNativeHandle();
                    psoDesc.HS.pShaderBytecode = pShader->pByteCode;
                    psoDesc.HS.BytecodeLength  = pShader->codeSize;
                }
                else
                {
                    psoDesc.HS.pShaderBytecode = nullptr;
                    psoDesc.HS.BytecodeLength  = 0;
                }
                if( gpDesc.geometryShader.hShader != ShaderHandle() )
                {
                    D3D12ShaderModule* pShader = gpDesc.geometryShader.hShader.GetNativeHandle();
                    psoDesc.GS.pShaderBytecode = pShader->pByteCode;
                    psoDesc.GS.BytecodeLength  = pShader->codeSize;
                }
                else
                {
                    psoDesc.GS.pShaderBytecode = nullptr;
                    psoDesc.GS.BytecodeLength  = 0;
                }

                // Input state
                BGS_ASSERT( gpDesc.inputState.inputElementCount <= Config::Driver::Pipeline::MAX_INPUT_ELEMENT_COUNT );
                BGS_ASSERT( gpDesc.inputState.inputBindingCount <= Config::Driver::Pipeline::MAX_INPUT_BINDING_COUNT );
                if( ( gpDesc.inputState.inputElementCount > Config::Driver::Pipeline::MAX_INPUT_ELEMENT_COUNT ) &&
                    ( gpDesc.inputState.inputBindingCount > Config::Driver::Pipeline::MAX_INPUT_BINDING_COUNT ) )
                {
                    return Results::FAIL;
                }
                D3D12_INPUT_ELEMENT_DESC elemDescs[ Config::Driver::Pipeline::MAX_INPUT_ELEMENT_COUNT ];
                for( index_t ndx = 0; ndx < static_cast<index_t>( gpDesc.inputState.inputElementCount ); ++ndx )
                {
                    const InputElementDesc& currDesc = gpDesc.inputState.pInputElements[ ndx ];

                    // In vulkan input rate is specified by binding not by element. We need to mimic this behavior here.
                    D3D12_INPUT_CLASSIFICATION inputClass{};
                    for( index_t ndy = 0; ndy < static_cast<index_t>( gpDesc.inputState.inputBindingCount ); ++ndy )
                    {
                        const InputBindingDesc& currBinding = gpDesc.inputState.pInputBindings[ ndy ];
                        if( currDesc.binding == currBinding.binding )
                        {
                            inputClass = MapBigosInputStepRateToD3D12InputClassification( currBinding.inputRate );
                            break;
                        }
                    }

                    D3D12_INPUT_ELEMENT_DESC& currElem = elemDescs[ ndx ];
                    currElem.SemanticName              = currDesc.pSemanticName;
                    currElem.SemanticIndex             = 0; // TODO: Handle
                    currElem.InputSlot                 = currDesc.binding;
                    currElem.Format                    = MapBigosFormatToD3D12Format( currDesc.format );
                    currElem.AlignedByteOffset         = currDesc.offset;
                    currElem.InputSlotClass            = inputClass;
                    currElem.InstanceDataStepRate      = inputClass == D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA ? 0 : 1;
                    // TODO: Handle
                }

                D3D12_INPUT_LAYOUT_DESC inputLayout;
                inputLayout.NumElements        = gpDesc.inputState.inputElementCount;
                inputLayout.pInputElementDescs = elemDescs;

                // Nothing simmilar in vulkan, probably hardly connected to windows
                D3D12_STREAM_OUTPUT_DESC so{};

                // Rasterizer state
                D3D12_RASTERIZER_DESC rasterState;
                rasterState.FillMode              = MapBigosPolygonFillModeToD3D12FillModes( gpDesc.rasterizeState.fillMode );
                rasterState.CullMode              = MapBigosCullModeToD3D12CullMode( gpDesc.rasterizeState.cullMode );
                rasterState.FrontCounterClockwise = MapBigosFrontFaceModeToD3D12Bool( gpDesc.rasterizeState.frontFaceMode );
                rasterState.DepthClipEnable       = gpDesc.rasterizeState.depthClipEnable;
                rasterState.DepthBias             = static_cast<INT>( gpDesc.rasterizeState.depthBiasConstantFactor );
                rasterState.SlopeScaledDepthBias  = gpDesc.rasterizeState.depthBiasSlopeFactor;
                rasterState.DepthBiasClamp        = gpDesc.rasterizeState.depthBiasClamp;
                rasterState.AntialiasedLineEnable = false; // Potentially need to handle all above
                rasterState.MultisampleEnable     = false;
                rasterState.ForcedSampleCount     = 0;
                rasterState.ConservativeRaster    = D3D12_CONSERVATIVE_RASTERIZATION_MODE ::D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

                // Multisample state
                DXGI_SAMPLE_DESC multisampleState;
                multisampleState.Count   = MapBigosSampleCountToD3D12Uint( gpDesc.multisampleState.sampleCount );
                multisampleState.Quality = 0; // TODO: For now, should swith to sampleCount - 1 ?

                // Depth stencil state
                D3D12_DEPTH_STENCIL_DESC depthStencilState;
                depthStencilState.DepthEnable      = gpDesc.depthStencilState.depthTestEnable;
                depthStencilState.DepthWriteMask   = MapBigosBoolToD3D12DepthWriteMask( gpDesc.depthStencilState.depthWriteEnable );
                depthStencilState.DepthFunc        = MapBigosCompareOperationTypeToD3D12ComparsionFunc( gpDesc.depthStencilState.depthCompare );
                depthStencilState.StencilEnable    = gpDesc.depthStencilState.stencilTestEnable;
                depthStencilState.StencilWriteMask = gpDesc.depthStencilState.stencilWriteMask;
                depthStencilState.StencilReadMask  = gpDesc.depthStencilState.stencilReadMask;

                depthStencilState.BackFace.StencilFailOp = MapBigosStencilOperationTypeToD3D12StencilOp( gpDesc.depthStencilState.backFace.failOp );
                depthStencilState.BackFace.StencilPassOp = MapBigosStencilOperationTypeToD3D12StencilOp( gpDesc.depthStencilState.backFace.passOp );
                depthStencilState.BackFace.StencilDepthFailOp =
                    MapBigosStencilOperationTypeToD3D12StencilOp( gpDesc.depthStencilState.backFace.depthFailOp );
                depthStencilState.BackFace.StencilFunc =
                    MapBigosCompareOperationTypeToD3D12ComparsionFunc( gpDesc.depthStencilState.backFace.compareOp );

                depthStencilState.FrontFace.StencilFailOp = MapBigosStencilOperationTypeToD3D12StencilOp( gpDesc.depthStencilState.frontFace.failOp );
                depthStencilState.FrontFace.StencilPassOp = MapBigosStencilOperationTypeToD3D12StencilOp( gpDesc.depthStencilState.frontFace.passOp );
                depthStencilState.FrontFace.StencilDepthFailOp =
                    MapBigosStencilOperationTypeToD3D12StencilOp( gpDesc.depthStencilState.frontFace.depthFailOp );
                depthStencilState.FrontFace.StencilFunc =
                    MapBigosCompareOperationTypeToD3D12ComparsionFunc( gpDesc.depthStencilState.frontFace.compareOp );

                // Blend state
                BGS_ASSERT( gpDesc.blendState.renderTargetBlendDescCount <= Config::Driver::Pipeline::MAX_BLEND_STATE_COUNT );
                if( gpDesc.blendState.renderTargetBlendDescCount > Config::Driver::Pipeline::MAX_BLEND_STATE_COUNT )
                {
                    return Results::FAIL;
                }

                D3D12_BLEND_DESC blendState;
                for( index_t ndx = 0; ndx < static_cast<index_t>( gpDesc.blendState.renderTargetBlendDescCount ); ++ndx )
                {
                    const RenderTargetBlendDesc&    currDesc   = gpDesc.blendState.pRenderTargetBlendDescs[ ndx ];
                    D3D12_RENDER_TARGET_BLEND_DESC& currTarget = blendState.RenderTarget[ ndx ];

                    currTarget.SrcBlend              = MapBigosBlendFactorToD3D12Blend( currDesc.srcColorBlendFactor );
                    currTarget.DestBlend             = MapBigosBlendFactorToD3D12Blend( currDesc.dstColorBlendFactor );
                    currTarget.BlendOp               = MapBigosBlendOperationTypeToD3D12BlendOp( currDesc.colorBlendOp );
                    currTarget.SrcBlendAlpha         = MapBigosBlendFactorToD3D12Blend( currDesc.srcAlphaBlendFactor );
                    currTarget.DestBlendAlpha        = MapBigosBlendFactorToD3D12Blend( currDesc.dstAlphaBlendFactor );
                    currTarget.BlendOpAlpha          = MapBigosBlendOperationTypeToD3D12BlendOp( currDesc.alphaBlendOp );
                    currTarget.RenderTargetWriteMask = static_cast<UINT8>( currDesc.writeFlag );
                    currTarget.BlendEnable           = currDesc.blendEnable;
                    currTarget.LogicOpEnable         = gpDesc.blendState.enableLogicOperations;
                    currTarget.LogicOp               = MapBigosLogicOperationTypeToD3D12LogicOp( gpDesc.blendState.logicOperation );
                }
                blendState.AlphaToCoverageEnable  = FALSE; // To mimic vulkan behaviour
                blendState.IndependentBlendEnable = FALSE; // To mimic vulkan behaviour

                // Render target formats
                BGS_ASSERT( gpDesc.renderTargetCount <= Config::Driver::Pipeline::MAX_RENDER_TARGET_COUNT );
                if( gpDesc.renderTargetCount > Config::Driver::Pipeline::MAX_RENDER_TARGET_COUNT )
                {
                    return Results::FAIL;
                }

                psoDesc.NumRenderTargets = gpDesc.renderTargetCount;
                psoDesc.DSVFormat        = MapBigosFormatToD3D12Format( gpDesc.depthStencilFormat );
                for( index_t ndx = 0; ndx < static_cast<index_t>( gpDesc.renderTargetCount ); ++ndx )
                {
                    psoDesc.RTVFormats[ ndx ] = MapBigosFormatToD3D12Format( gpDesc.pRenderTargetFormats[ ndx ] );
                }
                for( index_t ndx = gpDesc.renderTargetCount; ndx < static_cast<index_t>( Config::Driver::Pipeline::MAX_RENDER_TARGET_COUNT ); ++ndx )
                {
                    psoDesc.RTVFormats[ ndx ] = DXGI_FORMAT_UNKNOWN;
                }

                // Cahed pipeline - TODO: Handle
                D3D12_CACHED_PIPELINE_STATE cachedPipeline{};

                // Pipeline desc
                psoDesc.pRootSignature        = gpDesc.hPipelineLayout.GetNativeHandle();
                psoDesc.InputLayout           = inputLayout;
                psoDesc.PrimitiveTopologyType = MapBigosPrimitiveTopologyToD3D12PrimitiveTopologyType( gpDesc.inputAssemblerState.topology );
                psoDesc.IBStripCutValue   = MapBigosIndexRestartValueToD3D12IndexBufferStripCutValue( gpDesc.inputAssemblerState.indexRestartValue );
                psoDesc.StreamOutput      = so;
                psoDesc.RasterizerState   = rasterState;
                psoDesc.SampleDesc        = multisampleState;
                psoDesc.DepthStencilState = depthStencilState;
                psoDesc.BlendState        = blendState;
                psoDesc.SampleMask        = UINT_MAX;
                psoDesc.CachedPSO         = cachedPipeline;
                psoDesc.NodeMask          = 0;
                psoDesc.Flags             = D3D12_PIPELINE_STATE_FLAG_NONE;

                ID3D12Device*        pNativeDevice   = m_handle.GetNativeHandle();
                ID3D12PipelineState* pNativePipeline = nullptr;
                if( FAILED( pNativeDevice->CreateGraphicsPipelineState( &psoDesc, IID_PPV_ARGS( &pNativePipeline ) ) ) )
                {
                    return Results::FAIL;
                }

                D3D12Pipeline* pPipeline = nullptr;
                if( BGS_FAILED( Core::Memory::AllocateObject( m_pParent->GetParent()->GetDefaultAllocator(), &pPipeline ) ) )
                {
                    RELEASE_COM_PTR( pNativePipeline );
                    return Results::NO_MEMORY;
                }

                pPipeline->pPipeline      = pNativePipeline;
                pPipeline->pRootSignature = gpDesc.hPipelineLayout.GetNativeHandle();

                *ppPipeline = pPipeline;

                return Results::OK;
            }

            RESULT D3D12Device::CreateD3D12ComputePipeline( const ComputePipelineDesc& cpDesc, D3D12Pipeline** ppPipeline )
            {
                // Cahed pipeline - TODO: Handle
                D3D12_CACHED_PIPELINE_STATE cachedPipeline{};

                D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
                if( cpDesc.computeShader.hShader != ShaderHandle() )
                {
                    D3D12ShaderModule* pShader = cpDesc.computeShader.hShader.GetNativeHandle();
                    psoDesc.CS.pShaderBytecode = pShader->pByteCode;
                    psoDesc.CS.BytecodeLength  = pShader->codeSize;
                }
                else
                {
                    psoDesc.CS.pShaderBytecode = nullptr;
                    psoDesc.CS.BytecodeLength  = 0;
                }

                // Pipeline desc
                psoDesc.pRootSignature = cpDesc.hPipelineLayout.GetNativeHandle();
                psoDesc.CachedPSO      = cachedPipeline;
                psoDesc.Flags          = D3D12_PIPELINE_STATE_FLAG_NONE;
                psoDesc.NodeMask       = 0;

                ID3D12Device*        pNativeDevice   = m_handle.GetNativeHandle();
                ID3D12PipelineState* pNativePipeline = nullptr;
                if( FAILED( pNativeDevice->CreateComputePipelineState( &psoDesc, IID_PPV_ARGS( &pNativePipeline ) ) ) )
                {
                    return Results::FAIL;
                }

                D3D12Pipeline* pPipeline = nullptr;
                if( BGS_FAILED( Core::Memory::AllocateObject( m_pParent->GetParent()->GetDefaultAllocator(), &pPipeline ) ) )
                {
                    RELEASE_COM_PTR( pNativePipeline );
                    return Results::NO_MEMORY;
                }

                pPipeline->pPipeline      = pNativePipeline;
                pPipeline->pRootSignature = cpDesc.hPipelineLayout.GetNativeHandle();

                *ppPipeline = pPipeline;

                return Results::OK;
            }

            void D3D12Device::CreateD3D12MemoryHeapProperties( const AllocateMemoryDesc& desc, D3D12_HEAP_PROPERTIES* pProps )
            {
                if( desc.heapType == MemoryHeapTypes::CUSTOM )
                {
                    pProps->Type = D3D12_HEAP_TYPE_CUSTOM;
                    if( desc.access & static_cast<uint32_t>( MemoryAccessFlagBits::GPU_DEVICE_ACCESS ) )
                    {
                        // Device only access
                        pProps->CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
                    }
                    else
                    {
                        pProps->CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
                        pProps->MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
                        pProps->VisibleNodeMask      = 0;
                        pProps->CreationNodeMask     = 0;

                        if( desc.access & static_cast<uint32_t>( MemoryAccessFlagBits::CPU_READ ) ||
                            desc.access & static_cast<uint32_t>( MemoryAccessFlagBits::GPU_WRITES_TO_SYSTEM ) )
                        {
                            pProps->CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
                        }
                        if( desc.access & static_cast<uint32_t>( MemoryAccessFlagBits::CPU_WRITES_TO_DEVICE ) )
                        {
                            pProps->CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
                        }
                        if( desc.access & static_cast<uint32_t>( MemoryAccessFlagBits::CPU_ACCESS ) &&
                            desc.access & static_cast<uint32_t>( MemoryAccessFlagBits::GPU_DEVICE_ACCESS ) )
                        {
                            pProps->MemoryPoolPreference = D3D12_MEMORY_POOL_L1;
                        }
                        if( desc.access & static_cast<uint32_t>( MemoryAccessFlagBits::CPU_WRITES_TO_DEVICE ) &&
                            desc.access & static_cast<uint32_t>( MemoryAccessFlagBits::GPU_DEVICE_ACCESS ) )
                        {
                            pProps->MemoryPoolPreference = D3D12_MEMORY_POOL_L1;
                        }
                    }
                }
                else
                {
                    *pProps = m_heapProperties[ BGS_ENUM_INDEX( desc.heapType ) ];
                }
            }

            RESULT D3D12Device::CreateD3D12GlobalDescriptorHeaps()
            {

                RESULT        res           = Results::OK;
                ID3D12Device* pNativeDevice = m_handle.GetNativeHandle();

                Core::Memory::Set( m_descHeaps, 0, sizeof( m_descHeaps ) );

                D3D12_DESCRIPTOR_HEAP_DESC desc;
                desc.NodeMask       = 0;
                desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
                desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                desc.NumDescriptors = Config::Driver::Binding::MAX_SHADER_RESOURCE_COUNT;
                m_descHeaps[ static_cast<uint32_t>( desc.Type ) ].allocator.Create( desc.NumDescriptors );
                if( FAILED( pNativeDevice->CreateDescriptorHeap( &desc, IID_PPV_ARGS( &m_descHeaps[ static_cast<uint32_t>( desc.Type ) ].pHeap ) ) ) )
                {
                    res = Results::FAIL;
                }

                desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
                desc.NumDescriptors = Config::Driver::Binding::MAX_SAMPLER_COUNT;
                m_descHeaps[ static_cast<uint32_t>( desc.Type ) ].allocator.Create( desc.NumDescriptors );
                if( FAILED( pNativeDevice->CreateDescriptorHeap( &desc, IID_PPV_ARGS( &m_descHeaps[ static_cast<uint32_t>( desc.Type ) ].pHeap ) ) ) )
                {
                    res = Results::FAIL;
                }

                desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                desc.NumDescriptors = Config::Driver::Binding::MAX_RENDER_TARGET_VIEW_COUNT;
                m_descHeaps[ static_cast<uint32_t>( desc.Type ) ].allocator.Create( desc.NumDescriptors );
                if( FAILED( pNativeDevice->CreateDescriptorHeap( &desc, IID_PPV_ARGS( &m_descHeaps[ static_cast<uint32_t>( desc.Type ) ].pHeap ) ) ) )
                {
                    res = Results::FAIL;
                }

                desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
                desc.NumDescriptors = Config::Driver::Binding::MAX_DEPTH_STENCIL_TARGET_VIEW_COUNT;
                m_descHeaps[ static_cast<uint32_t>( desc.Type ) ].allocator.Create( desc.NumDescriptors );
                if( FAILED( pNativeDevice->CreateDescriptorHeap( &desc, IID_PPV_ARGS( &m_descHeaps[ static_cast<uint32_t>( desc.Type ) ].pHeap ) ) ) )
                {
                    res = Results::FAIL;
                }

                if( res != Results::OK )
                {
                    for( index_t ndx = 0; ndx < 4; ++ndx )
                    {
                        RELEASE_COM_PTR( m_descHeaps[ ndx ].pHeap );
                        m_descHeaps[ ndx ].allocator.Destroy();
                    }
                }

                return res;
            }

            uint32_t D3D12Device::CreateD3D12RTV( const TextureViewDesc& desc )
            {
                D3D12_RENDER_TARGET_VIEW_DESC viewDesc;
                viewDesc.Format = MapBigosFormatToD3D12Format( desc.format );

                switch( desc.textureType )
                {
                    // TODO: Does vulkan support buffer render target?
                    case TextureTypes::TEXTURE_1D:
                    {
                        viewDesc.ViewDimension      = D3D12_RTV_DIMENSION_TEXTURE1D;
                        viewDesc.Texture1D.MipSlice = desc.range.mipLevel;
                        break;
                    }
                    case TextureTypes::TEXTURE_1D_ARRAY:
                    {
                        viewDesc.ViewDimension                  = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
                        viewDesc.Texture1DArray.MipSlice        = desc.range.mipLevel;
                        viewDesc.Texture1DArray.FirstArraySlice = desc.range.arrayLayer;
                        viewDesc.Texture1DArray.ArraySize       = desc.range.arrayLayerCount;
                        break;
                    }
                    case TextureTypes::TEXTURE_2D:
                    {
                        viewDesc.ViewDimension        = D3D12_RTV_DIMENSION_TEXTURE2D;
                        viewDesc.Texture2D.MipSlice   = desc.range.mipLevel;
                        viewDesc.Texture2D.PlaneSlice = MapBigosTextureComponentFlagsToD3D12PlaneSlice( desc.range.components );
                        break;
                    }
                    case TextureTypes::TEXTURE_2D_ARRAY:
                    {
                        viewDesc.ViewDimension                  = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                        viewDesc.Texture2DArray.MipSlice        = desc.range.mipLevel;
                        viewDesc.Texture2DArray.PlaneSlice      = MapBigosTextureComponentFlagsToD3D12PlaneSlice( desc.range.components );
                        viewDesc.Texture2DArray.FirstArraySlice = desc.range.arrayLayer;
                        viewDesc.Texture2DArray.ArraySize       = desc.range.arrayLayerCount;
                        break;
                    }
                    case TextureTypes::TEXTURE_3D:
                    {
                        viewDesc.ViewDimension         = D3D12_RTV_DIMENSION_TEXTURE3D;
                        viewDesc.Texture3D.MipSlice    = desc.range.mipLevel;
                        viewDesc.Texture3D.FirstWSlice = desc.range.arrayLayer;
                        viewDesc.Texture3D.WSize       = desc.range.arrayLayerCount;
                        break;
                    }
                    default:
                    {
                        BGS_ASSERT( 0 );
                        return MAX_UINT32;
                    }
                }

                uint32_t ndx = MAX_UINT32;
                RESULT   res = m_descHeaps[ BGS_ENUM_INDEX( D3D12_DESCRIPTOR_HEAP_TYPE_RTV ) ].allocator.Allocate( &ndx );
                BGS_ASSERT( res == Results::OK );
                ID3D12Device*               pNativeDevice = m_handle.GetNativeHandle();
                D3D12_CPU_DESCRIPTOR_HANDLE handle =
                    m_descHeaps[ BGS_ENUM_INDEX( D3D12_DESCRIPTOR_HEAP_TYPE_RTV ) ].pHeap->GetCPUDescriptorHandleForHeapStart();
                handle.ptr += ndx * pNativeDevice->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );
                pNativeDevice->CreateRenderTargetView( desc.hResource.GetNativeHandle()->pNativeResource, &viewDesc, handle );

                return ndx;
            }

            uint32_t D3D12Device::CreateD3D12DSV( const TextureViewDesc& desc )
            {
                D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc;
                viewDesc.Format = MapBigosFormatToD3D12Format( desc.format );

                switch( desc.textureType )
                {
                    // TODO: Does vulkan support buffer render target?
                    case TextureTypes::TEXTURE_1D:
                    {
                        viewDesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE1D;
                        viewDesc.Texture1D.MipSlice = desc.range.mipLevel;
                        break;
                    }
                    case TextureTypes::TEXTURE_1D_ARRAY:
                    {
                        viewDesc.ViewDimension                  = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
                        viewDesc.Texture1DArray.MipSlice        = desc.range.mipLevel;
                        viewDesc.Texture1DArray.FirstArraySlice = desc.range.arrayLayer;
                        viewDesc.Texture1DArray.ArraySize       = desc.range.arrayLayerCount;
                        break;
                    }
                    case TextureTypes::TEXTURE_2D:
                    {
                        viewDesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
                        viewDesc.Texture2D.MipSlice = desc.range.mipLevel;
                        break;
                    }
                    case TextureTypes::TEXTURE_2D_ARRAY:
                    {
                        viewDesc.ViewDimension                  = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                        viewDesc.Texture2DArray.MipSlice        = desc.range.mipLevel;
                        viewDesc.Texture2DArray.FirstArraySlice = desc.range.arrayLayer;
                        viewDesc.Texture2DArray.ArraySize       = desc.range.arrayLayerCount;
                        break;
                    }
                    default:
                    {
                        BGS_ASSERT( 0 );
                        return MAX_UINT32;
                    }
                }

                uint32_t ndx = MAX_UINT32;
                RESULT   res = m_descHeaps[ BGS_ENUM_INDEX( D3D12_DESCRIPTOR_HEAP_TYPE_DSV ) ].allocator.Allocate( &ndx );
                BGS_ASSERT( res == Results::OK );
                ID3D12Device*               pNativeDevice = m_handle.GetNativeHandle();
                D3D12_CPU_DESCRIPTOR_HANDLE handle =
                    m_descHeaps[ BGS_ENUM_INDEX( D3D12_DESCRIPTOR_HEAP_TYPE_DSV ) ].pHeap->GetCPUDescriptorHandleForHeapStart();
                handle.ptr += ndx * pNativeDevice->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_DSV );
                pNativeDevice->CreateDepthStencilView( desc.hResource.GetNativeHandle()->pNativeResource, &viewDesc, handle );

                return ndx;
            }

            uint32_t D3D12Device::CreateD3D12CBV( const BufferViewDesc& desc )
            {
                D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc;
                viewDesc.BufferLocation = desc.hResource.GetNativeHandle()->pNativeResource->GetGPUVirtualAddress() + desc.range.offset;
                viewDesc.SizeInBytes    = static_cast<uint32_t>( desc.range.size );

                uint32_t ndx = MAX_UINT32;
                RESULT   res = m_descHeaps[ BGS_ENUM_INDEX( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ) ].allocator.Allocate( &ndx );
                BGS_ASSERT( res == Results::OK );
                ID3D12Device*               pNativeDevice = m_handle.GetNativeHandle();
                D3D12_CPU_DESCRIPTOR_HANDLE handle =
                    m_descHeaps[ BGS_ENUM_INDEX( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ) ].pHeap->GetCPUDescriptorHandleForHeapStart();
                handle.ptr += ndx * pNativeDevice->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
                pNativeDevice->CreateConstantBufferView( &viewDesc, handle );

                return ndx;
            }

            uint32_t D3D12Device::CreateD3D12SRV( const TextureViewDesc& desc )
            {
                D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc;
                viewDesc.Format                  = MapBigosFormatToD3D12Format( desc.format );
                viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

                switch( desc.textureType )
                {
                    // TODO: Does vulkan support buffer render target?
                    case TextureTypes::TEXTURE_1D:
                    {
                        viewDesc.ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE1D;
                        viewDesc.Texture1D.MostDetailedMip     = desc.range.mipLevel;
                        viewDesc.Texture1D.MipLevels           = desc.range.mipLevelCount;
                        viewDesc.Texture1D.ResourceMinLODClamp = 0.0f;
                        break;
                    }
                    case TextureTypes::TEXTURE_1D_ARRAY:
                    {
                        viewDesc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                        viewDesc.Texture1DArray.MostDetailedMip     = desc.range.mipLevel;
                        viewDesc.Texture1DArray.MipLevels           = desc.range.mipLevelCount;
                        viewDesc.Texture1DArray.FirstArraySlice     = desc.range.arrayLayer;
                        viewDesc.Texture1DArray.ArraySize           = desc.range.arrayLayerCount;
                        viewDesc.Texture1DArray.ResourceMinLODClamp = 0.0f;
                        break;
                    }
                    case TextureTypes::TEXTURE_2D:
                    {
                        viewDesc.ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE2D;
                        viewDesc.Texture2D.MostDetailedMip     = desc.range.mipLevel;
                        viewDesc.Texture2D.MipLevels           = desc.range.mipLevelCount;
                        viewDesc.Texture2D.PlaneSlice          = MapBigosTextureComponentFlagsToD3D12PlaneSlice( desc.range.components );
                        viewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
                        break;
                    }
                    case TextureTypes::TEXTURE_2D_ARRAY:
                    {
                        viewDesc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                        viewDesc.Texture2DArray.MostDetailedMip     = desc.range.mipLevel;
                        viewDesc.Texture2DArray.MipLevels           = desc.range.mipLevelCount;
                        viewDesc.Texture2DArray.PlaneSlice          = MapBigosTextureComponentFlagsToD3D12PlaneSlice( desc.range.components );
                        viewDesc.Texture2DArray.FirstArraySlice     = desc.range.arrayLayer;
                        viewDesc.Texture2DArray.ArraySize           = desc.range.arrayLayerCount;
                        viewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
                        break;
                    }
                    case TextureTypes::TEXTURE_3D:
                    {
                        viewDesc.ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE3D;
                        viewDesc.Texture3D.MostDetailedMip     = desc.range.mipLevel;
                        viewDesc.Texture3D.MipLevels           = desc.range.mipLevelCount;
                        viewDesc.Texture3D.ResourceMinLODClamp = 0.0f;
                        break;
                    }
                    case TextureTypes::TEXURTE_CUBE:
                    {
                        viewDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURECUBE;
                        viewDesc.TextureCube.MostDetailedMip     = desc.range.mipLevel;
                        viewDesc.TextureCube.MipLevels           = desc.range.mipLevelCount;
                        viewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
                        break;
                    }
                    case TextureTypes::TEXURTE_CUBE_ARRAY:
                    {
                        viewDesc.ViewDimension                        = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                        viewDesc.TextureCubeArray.MostDetailedMip     = desc.range.mipLevel;
                        viewDesc.TextureCubeArray.MipLevels           = desc.range.mipLevelCount;
                        viewDesc.TextureCubeArray.First2DArrayFace    = desc.range.arrayLayer;
                        viewDesc.TextureCubeArray.NumCubes            = desc.range.arrayLayerCount;
                        viewDesc.TextureCubeArray.ResourceMinLODClamp = 0.0f;
                        break;
                    }
                    default:
                    {
                        BGS_ASSERT( 0 );
                        return MAX_UINT32;
                    }
                }

                uint32_t ndx = MAX_UINT32;
                RESULT   res = m_descHeaps[ BGS_ENUM_INDEX( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ) ].allocator.Allocate( &ndx );
                BGS_ASSERT( res == Results::OK );
                ID3D12Device*               pNativeDevice = m_handle.GetNativeHandle();
                D3D12_CPU_DESCRIPTOR_HANDLE handle =
                    m_descHeaps[ BGS_ENUM_INDEX( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ) ].pHeap->GetCPUDescriptorHandleForHeapStart();
                handle.ptr += ndx * pNativeDevice->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
                pNativeDevice->CreateShaderResourceView( desc.hResource.GetNativeHandle()->pNativeResource, &viewDesc, handle );

                return ndx;
            }

            uint32_t D3D12Device::CreateD3D12SRV( const TexelBufferViewDesc& desc )
            {
                D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc;
                viewDesc.Format                     = MapBigosFormatToD3D12Format( desc.format );
                viewDesc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                viewDesc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
                viewDesc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_NONE;
                viewDesc.Buffer.StructureByteStride = 0;
                viewDesc.Buffer.FirstElement        = desc.range.offset / GetBigosFormatSize( desc.format );
                viewDesc.Buffer.NumElements         = static_cast<uint32_t>( desc.range.size / GetBigosFormatSize( desc.format ) );

                uint32_t ndx = MAX_UINT32;
                RESULT   res = m_descHeaps[ BGS_ENUM_INDEX( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ) ].allocator.Allocate( &ndx );
                BGS_ASSERT( res == Results::OK );
                ID3D12Device*               pNativeDevice = m_handle.GetNativeHandle();
                D3D12_CPU_DESCRIPTOR_HANDLE handle =
                    m_descHeaps[ BGS_ENUM_INDEX( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ) ].pHeap->GetCPUDescriptorHandleForHeapStart();
                handle.ptr += ndx * pNativeDevice->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
                pNativeDevice->CreateShaderResourceView( desc.hResource.GetNativeHandle()->pNativeResource, &viewDesc, handle );

                return ndx;
            }

            uint32_t D3D12Device::CreateD3D12SRV( const BufferViewDesc& desc )
            {
                D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc;
                viewDesc.Format                     = DXGI_FORMAT_R32_TYPELESS;
                viewDesc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                viewDesc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
                viewDesc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_RAW;
                viewDesc.Buffer.StructureByteStride = 0;
                // 4 is size in bytes of DXGI_FORMAT_R32_TYPELESS
                viewDesc.Buffer.FirstElement = desc.range.offset / 4;
                viewDesc.Buffer.NumElements  = static_cast<uint32_t>( desc.range.size / 4 );

                uint32_t ndx = MAX_UINT32;
                RESULT   res = m_descHeaps[ BGS_ENUM_INDEX( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ) ].allocator.Allocate( &ndx );
                BGS_ASSERT( res == Results::OK );
                ID3D12Device*               pNativeDevice = m_handle.GetNativeHandle();
                D3D12_CPU_DESCRIPTOR_HANDLE handle =
                    m_descHeaps[ BGS_ENUM_INDEX( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ) ].pHeap->GetCPUDescriptorHandleForHeapStart();
                handle.ptr += ndx * pNativeDevice->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
                pNativeDevice->CreateShaderResourceView( desc.hResource.GetNativeHandle()->pNativeResource, &viewDesc, handle );

                return ndx;
            }

            uint32_t D3D12Device::CreateD3D12UAV( const TextureViewDesc& desc )
            {
                D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc;
                viewDesc.Format = MapBigosFormatToD3D12Format( desc.format );

                switch( desc.textureType )
                {
                    // TODO: Does vulkan support buffer render target?
                    case TextureTypes::TEXTURE_1D:
                    {
                        viewDesc.ViewDimension      = D3D12_UAV_DIMENSION_TEXTURE1D;
                        viewDesc.Texture1D.MipSlice = desc.range.mipLevel;
                        break;
                    }
                    case TextureTypes::TEXTURE_1D_ARRAY:
                    {
                        viewDesc.ViewDimension                  = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
                        viewDesc.Texture1DArray.MipSlice        = desc.range.mipLevel;
                        viewDesc.Texture1DArray.FirstArraySlice = desc.range.arrayLayer;
                        viewDesc.Texture1DArray.ArraySize       = desc.range.arrayLayerCount;
                        break;
                    }
                    case TextureTypes::TEXTURE_2D:
                    {
                        viewDesc.ViewDimension        = D3D12_UAV_DIMENSION_TEXTURE2D;
                        viewDesc.Texture2D.MipSlice   = desc.range.mipLevel;
                        viewDesc.Texture2D.PlaneSlice = MapBigosTextureComponentFlagsToD3D12PlaneSlice( desc.range.components );
                        break;
                    }
                    case TextureTypes::TEXTURE_2D_ARRAY:
                    {
                        viewDesc.ViewDimension                  = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                        viewDesc.Texture2DArray.MipSlice        = desc.range.mipLevel;
                        viewDesc.Texture2DArray.PlaneSlice      = MapBigosTextureComponentFlagsToD3D12PlaneSlice( desc.range.components );
                        viewDesc.Texture2DArray.FirstArraySlice = desc.range.arrayLayer;
                        viewDesc.Texture2DArray.ArraySize       = desc.range.arrayLayerCount;
                        break;
                    }
                    case TextureTypes::TEXTURE_3D:
                    {
                        viewDesc.ViewDimension         = D3D12_UAV_DIMENSION_TEXTURE3D;
                        viewDesc.Texture3D.MipSlice    = desc.range.mipLevel;
                        viewDesc.Texture3D.FirstWSlice = desc.range.arrayLayer;
                        viewDesc.Texture3D.WSize       = desc.range.arrayLayerCount;
                        break;
                    }
                    default:
                    {
                        BGS_ASSERT( 0 );
                        return MAX_UINT32;
                    }
                }

                uint32_t ndx = MAX_UINT32;
                RESULT   res = m_descHeaps[ BGS_ENUM_INDEX( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ) ].allocator.Allocate( &ndx );
                BGS_ASSERT( res == Results::OK );
                ID3D12Device*               pNativeDevice = m_handle.GetNativeHandle();
                D3D12_CPU_DESCRIPTOR_HANDLE handle =
                    m_descHeaps[ BGS_ENUM_INDEX( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ) ].pHeap->GetCPUDescriptorHandleForHeapStart();
                handle.ptr += ndx * pNativeDevice->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
                pNativeDevice->CreateUnorderedAccessView( desc.hResource.GetNativeHandle()->pNativeResource, nullptr, &viewDesc, handle );

                return ndx;
            }

            uint32_t D3D12Device::CreateD3D12UAV( const TexelBufferViewDesc& desc )
            {
                D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc;
                viewDesc.Format                      = MapBigosFormatToD3D12Format( desc.format );
                viewDesc.ViewDimension               = D3D12_UAV_DIMENSION_BUFFER;
                viewDesc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_RAW;
                viewDesc.Buffer.CounterOffsetInBytes = 0;
                viewDesc.Buffer.StructureByteStride  = 0;
                viewDesc.Buffer.FirstElement         = desc.range.offset / GetBigosFormatSize( desc.format );
                viewDesc.Buffer.NumElements          = static_cast<uint32_t>( desc.range.size / GetBigosFormatSize( desc.format ) );

                uint32_t ndx = MAX_UINT32;
                RESULT   res = m_descHeaps[ BGS_ENUM_INDEX( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ) ].allocator.Allocate( &ndx );
                BGS_ASSERT( res == Results::OK );
                ID3D12Device*               pNativeDevice = m_handle.GetNativeHandle();
                D3D12_CPU_DESCRIPTOR_HANDLE handle =
                    m_descHeaps[ BGS_ENUM_INDEX( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ) ].pHeap->GetCPUDescriptorHandleForHeapStart();
                handle.ptr += ndx * pNativeDevice->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
                pNativeDevice->CreateUnorderedAccessView( desc.hResource.GetNativeHandle()->pNativeResource, nullptr, &viewDesc, handle );

                return ndx;
            }

            uint32_t D3D12Device::CreateD3D12UAV( const BufferViewDesc& desc )
            {
                D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc;
                viewDesc.Format                     = DXGI_FORMAT_R32_TYPELESS;
                viewDesc.ViewDimension              = D3D12_UAV_DIMENSION_BUFFER;
                viewDesc.Buffer.Flags               = D3D12_BUFFER_UAV_FLAG_RAW;
                viewDesc.Buffer.StructureByteStride = 0;
                // 4 is size in bytes of DXGI_FORMAT_R32_TYPELESS
                viewDesc.Buffer.FirstElement = desc.range.offset / 4;
                viewDesc.Buffer.NumElements  = static_cast<uint32_t>( desc.range.size / 4 );

                uint32_t ndx = MAX_UINT32;
                RESULT   res = m_descHeaps[ BGS_ENUM_INDEX( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ) ].allocator.Allocate( &ndx );
                BGS_ASSERT( res == Results::OK );
                ID3D12Device*               pNativeDevice = m_handle.GetNativeHandle();
                D3D12_CPU_DESCRIPTOR_HANDLE handle =
                    m_descHeaps[ BGS_ENUM_INDEX( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ) ].pHeap->GetCPUDescriptorHandleForHeapStart();
                handle.ptr += ndx * pNativeDevice->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
                pNativeDevice->CreateUnorderedAccessView( desc.hResource.GetNativeHandle()->pNativeResource, nullptr, &viewDesc, handle );

                return ndx;
            }

            void D3D12Device::QueryD3D12BindingsSize()
            {
                ID3D12Device*  pNativeDevice              = m_handle.GetNativeHandle();
                const uint64_t shaderResBindingSize       = pNativeDevice->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
                const uint64_t samplerBindingSize         = pNativeDevice->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER );
                m_limits.samplerBindingSize               = samplerBindingSize;
                m_limits.sampledTextureBindingSize        = shaderResBindingSize;
                m_limits.storageTextureBindingSize        = shaderResBindingSize;
                m_limits.constantTexelBufferBindingSize   = shaderResBindingSize;
                m_limits.storageTexelBufferBindingSize    = shaderResBindingSize;
                m_limits.constantBufferBindingSize        = shaderResBindingSize;
                m_limits.readOnlyStorageBufferBindingSize = shaderResBindingSize;
                m_limits.readWriteStorageBufferBindingSize = shaderResBindingSize;
            }

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS