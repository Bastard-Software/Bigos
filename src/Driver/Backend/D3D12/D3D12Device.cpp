#define BGS_USE_D3D12_HANDLES 1

#include "D3D12Device.h"

#include "BigosFramework/Config.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Memory.h"
#include "D3D12CommandBuffer.h"
#include "D3D12Common.h"
#include "D3D12Factory.h"
#include "D3D12Fence.h"
#include "D3D12Pipeline.h"
#include "D3D12Queue.h"
#include "D3D12Resource.h"
#include "D3D12Shader.h"
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
                    return Results::FAIL;
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

            RESULT D3D12Device::CreatePipeline( const PipelineDesc& desc, PipelineHandle* pHandle )
            {
                BGS_ASSERT( pHandle != nullptr, "Pipeline (pHandle) must be a valid address." );

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
                        // TODO: Implement
                        return Results::FAIL;
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
                pHandle;
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
                BGS_ASSERT( desc.pWaitValues != nullptr, "Uint array (desc.pWaitValues) must be a valid pointer." );
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
                BGS_ASSERT( handle != ResourceHandle(), "Resource (handle) must be a valid handle." )
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

                return Results::OK;
            }

            void D3D12Device::Destroy()
            {
                BGS_ASSERT( m_handle != DeviceHandle() );

                if( m_handle != DeviceHandle() )
                {
                    ID3D12Device* pNativeDevice = m_handle.GetNativeHandle();
                    RELEASE_COM_PTR( pNativeDevice );
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
                    D3D12_RENDER_TARGET_BLEND_DESC& currAttach = blendState.RenderTarget[ ndx ];

                    currAttach.SrcBlend              = MapBigosBlendFactorToD3D12Blend( currDesc.srcColorBlendFactor );
                    currAttach.DestBlend             = MapBigosBlendFactorToD3D12Blend( currDesc.dstColorBlendFactor );
                    currAttach.BlendOp               = MapBigosBlendOperationTypeToD3D12BlendOp( currDesc.colorBlendOp );
                    currAttach.SrcBlendAlpha         = MapBigosBlendFactorToD3D12Blend( currDesc.srcAlphaBlendFactor );
                    currAttach.DestBlendAlpha        = MapBigosBlendFactorToD3D12Blend( currDesc.dstAlphaBlendFactor );
                    currAttach.BlendOpAlpha          = MapBigosBlendOperationTypeToD3D12BlendOp( currDesc.alphaBlendOp );
                    currAttach.RenderTargetWriteMask = static_cast<UINT8>( static_cast<uint32_t>( currDesc.writeFlag ) );
                    currAttach.BlendEnable           = currDesc.blendEnable;
                    currAttach.LogicOpEnable         = gpDesc.blendState.enableLogicOperations;
                    currAttach.LogicOp               = MapBigosLogicOperationTypeToD3D12LogicOp( gpDesc.blendState.logicOperation );
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

                D3D12Pipeline* pPipeline;
                if( BGS_FAILED( Core::Memory::AllocateObject( m_pParent->GetParent()->GetDefaultAllocator(), &pPipeline ) ) )
                {
                    RELEASE_COM_PTR( pNativePipeline );
                    return Results::NO_MEMORY;
                }

                pPipeline->pPipeline      = pNativePipeline;
                pPipeline->pRootSignature = gpDesc.hPipelineLayout != PipelineLayoutHandle() ? gpDesc.hPipelineLayout.GetNativeHandle() : nullptr;

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

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS