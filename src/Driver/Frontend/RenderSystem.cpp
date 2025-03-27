#include "Driver/Frontend/RenderSystem.h"

#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Memory.h"
#include "Driver/Backend/D3D12/D3D12Factory.h"
#include "Driver/Backend/Vulkan/VulkanFactory.h"
#include "Driver/Frontend/Buffer.h"
#include "Driver/Frontend/Camera/Camera.h"
#include "Driver/Frontend/Context.h"
#include "Driver/Frontend/Pipeline.h"
#include "Driver/Frontend/RenderPass.h"
#include "Driver/Frontend/RenderTarget.h"
#include "Driver/Frontend/Shader/Shader.h"
#include "Driver/Frontend/Swapchain.h"
#include "Driver/Frontend/Texture.h"
#include "Shader/ShaderCompilerFactory.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            RenderSystem::RenderSystem()
                : m_desc()
                , m_driverDesc()
                , m_pFactory( nullptr )
                , m_pDevice( nullptr )
                , m_adapters()
                , m_pGraphicsContext( nullptr )
                , m_pComputeContext( nullptr )
                , m_pCopyContext( nullptr )
                , m_cameras()
                , m_pParent( nullptr )
                , m_pShaderCompilerFactory( nullptr )
                , m_pDefaultAllocator( nullptr )
                , m_pCompiler( nullptr )
            {
            }

            RESULT RenderSystem::InitializeDriver( const DriverDesc& desc )
            {
                if( ( desc == m_driverDesc ) && ( m_pFactory != nullptr ) && ( m_pDevice != nullptr ) )
                {
                    return Results::OK;
                }

                m_driverDesc = desc;
                FreeDriver();

                Backend::FactoryDesc factoryDesc;
                factoryDesc.apiType = m_driverDesc.apiType;
                factoryDesc.flags   = m_driverDesc.debug ? BGS_FLAG( Backend::FactoryFlagBits::ENABLE_DEBUG_LAYERS_IF_AVAILABLE )
                                                         : BGS_FLAG( Backend::FactoryFlagBits::NONE );
                if( BGS_FAILED( CreateFactory( factoryDesc, &m_pFactory ) ) )
                {
                    return Results::FAIL;
                }

                Backend::DeviceDesc deviceDesc;
                deviceDesc.pAdapter = m_adapters[ 0 ];
                if( BGS_FAILED( m_pFactory->CreateDevice( deviceDesc, &m_pDevice ) ) )
                {
                    FreeDriver();
                    return Results::FAIL;
                }
                if( BGS_FAILED( CreateContexts() ) )
                {
                    FreeDriver();
                    return Results::FAIL;
                }

                return Results::OK;
            }

            RESULT RenderSystem::CreateBuffer( const BufferDesc& desc, Buffer** ppBuffer )
            {
                BGS_ASSERT( ppBuffer != nullptr, "Buffer (ppBuffer) must be a valid address." );
                BGS_ASSERT( *ppBuffer == nullptr, "There is a valid pointer at the given address. Buffer (*ppBuffer) must be nullptr." );

                Buffer* pBuffer = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( m_pDefaultAllocator, &pBuffer ) ) )
                {
                    return Results::NO_MEMORY;
                }

                if( BGS_FAILED( pBuffer->Create( desc, this ) ) )
                {
                    Memory::FreeObject( m_pDefaultAllocator, &pBuffer );
                    return Results::FAIL;
                }

                ( *ppBuffer ) = pBuffer;

                return Results::OK;
            }

            void RenderSystem::DestroyBuffer( Buffer** ppBuffer )
            {
                BGS_ASSERT( ppBuffer != nullptr, "Buffer (ppBuffer) must be a valid address." );
                BGS_ASSERT( *ppBuffer != nullptr, "Buffer (*ppBuffer) must be a valid pointer." );

                Buffer* pBuffer = ( *ppBuffer );
                pBuffer->Destroy();
                Memory::FreeObject( m_pDefaultAllocator, &pBuffer );
            }

            RESULT RenderSystem::CreateTexture( const TextureDesc& desc, Texture** ppTexture )
            {
                BGS_ASSERT( ppTexture != nullptr, "Texture (ppTexture) must be a valid address." );
                BGS_ASSERT( *ppTexture == nullptr, "There is a valid pointer at the given address. Texture (*ppTExture) must be nullptr." );

                Texture* pTexture = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( m_pDefaultAllocator, &pTexture ) ) )
                {
                    return Results::NO_MEMORY;
                }

                if( BGS_FAILED( pTexture->Create( desc, this ) ) )
                {
                    Memory::FreeObject( m_pDefaultAllocator, &pTexture );
                    return Results::FAIL;
                }

                ( *ppTexture ) = pTexture;

                return Results::OK;
            }

            void RenderSystem::DestroyTexture( Texture** ppTexture )
            {
                BGS_ASSERT( ppTexture != nullptr, "Texture (ppTexture) must be a valid address." );
                BGS_ASSERT( *ppTexture != nullptr, "Texture (*ppTexture) must be a valid pointer." );

                Texture* pTexture = ( *ppTexture );
                pTexture->Destroy();
                Memory::FreeObject( m_pDefaultAllocator, &pTexture );
            }

            RESULT RenderSystem::CreateRenderTarget( const RenderTargetDesc& desc, RenderTarget** ppRenderTarget )
            {
                BGS_ASSERT( ppRenderTarget != nullptr, "Render target (ppRenderTarget) must be a valid address." );
                BGS_ASSERT( *ppRenderTarget == nullptr,
                            "There is a valid pointer at the given address. Render target (*ppRenderTarget) must be nullptr." );

                RenderTarget* pRenderTarget = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( m_pDefaultAllocator, &pRenderTarget ) ) )
                {
                    return Results::NO_MEMORY;
                }

                if( BGS_FAILED( pRenderTarget->Create( desc, this ) ) )
                {
                    Memory::FreeObject( m_pDefaultAllocator, &pRenderTarget );
                    return Results::FAIL;
                }

                ( *ppRenderTarget ) = pRenderTarget;

                return Results::OK;
            }

            void RenderSystem::DestroyRenderTarget( RenderTarget** ppRenderTarget )
            {
                BGS_ASSERT( ppRenderTarget != nullptr, "Render target (ppRenderTarget) must be a valid address." );
                BGS_ASSERT( *ppRenderTarget != nullptr, "Render target (*ppRenderTarget) must be a valid pointer." );

                RenderTarget* pRenderTarget = ( *ppRenderTarget );
                pRenderTarget->Destroy();
                Memory::FreeObject( m_pDefaultAllocator, &pRenderTarget );
            }

            RESULT RenderSystem::CreateRenderPass( const RenderPassDesc& desc, RenderPass** ppRenderPass )
            {
                BGS_ASSERT( ppRenderPass != nullptr, "Render pass (ppRenderPass) must be a valid address." );
                BGS_ASSERT( *ppRenderPass == nullptr,
                            "There is a valid pointer at the given address. Render target (*ppRenderPass) must be nullptr." );

                RenderPass* pRenderPass = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( m_pDefaultAllocator, &pRenderPass ) ) )
                {
                    return Results::NO_MEMORY;
                }

                if( BGS_FAILED( pRenderPass->Create( desc, this ) ) )
                {
                    Memory::FreeObject( m_pDefaultAllocator, &pRenderPass );
                    return Results::FAIL;
                }

                ( *ppRenderPass ) = pRenderPass;

                return Results::OK;
            }

            void RenderSystem::DestroyRenderPass( RenderPass** ppRenderPass )
            {
                BGS_ASSERT( ppRenderPass != nullptr, "Render pass (ppRenderPass) must be a valid address." );
                BGS_ASSERT( *ppRenderPass != nullptr, "Render pass (*ppRenderPass) must be a valid pointer." );

                RenderPass* pRenderPass = ( *ppRenderPass );
                pRenderPass->Destroy();
                Memory::FreeObject( m_pDefaultAllocator, &pRenderPass );
            }

            RESULT RenderSystem::CreateShader( const ShaderDesc& desc, Shader** ppShader )
            {
                BGS_ASSERT( ppShader != nullptr, "Shader (ppShader) must be a valid address." );
                BGS_ASSERT( *ppShader == nullptr, "There is a valid pointer at the given address. Shader (*ppShader) must be nullptr." );

                Shader* pShader = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( m_pDefaultAllocator, &pShader ) ) )
                {
                    return Results::NO_MEMORY;
                }

                if( BGS_FAILED( pShader->Create( desc, this ) ) )
                {
                    Memory::FreeObject( m_pDefaultAllocator, &pShader );
                    return Results::FAIL;
                }

                ( *ppShader ) = pShader;

                return Results::OK;
            }

            void RenderSystem::DestroyShader( Shader** ppShader )
            {
                BGS_ASSERT( ppShader != nullptr, "Shader (ppShader) must be a valid address." );
                BGS_ASSERT( *ppShader != nullptr, "Buffer (*ppShader) must be a valid pointer." );

                Shader* pShader = ( *ppShader );
                pShader->Destroy();
                Memory::FreeObject( m_pDefaultAllocator, &pShader );
            }

            RESULT RenderSystem::CreatePipeline( const GraphicsPipelineDesc& desc, Pipeline** ppPipeline )
            {
                BGS_ASSERT( ppPipeline != nullptr, "Pipeline (ppPipeline) must be a valid address." );
                BGS_ASSERT( *ppPipeline == nullptr, "There is a valid pointer at the given address. Pipeline (*ppPipeline) must be nullptr." );

                Pipeline* pPipeline = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( m_pDefaultAllocator, &pPipeline ) ) )
                {
                    return Results::NO_MEMORY;
                }

                if( BGS_FAILED( pPipeline->Create( desc, this ) ) )
                {
                    Memory::FreeObject( m_pDefaultAllocator, &pPipeline );
                    return Results::FAIL;
                }

                ( *ppPipeline ) = pPipeline;

                return Results::OK;
            }

            RESULT RenderSystem::CreatePipeline( const ComputePipelineDesc& desc, Pipeline** ppPipeline )
            {
                BGS_ASSERT( ppPipeline != nullptr, "Pipeline (ppPipeline) must be a valid address." );
                BGS_ASSERT( *ppPipeline == nullptr, "There is a valid pointer at the given address. Pipeline (*ppPipeline) must be nullptr." );

                Pipeline* pPipeline = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( m_pDefaultAllocator, &pPipeline ) ) )
                {
                    return Results::NO_MEMORY;
                }

                if( BGS_FAILED( pPipeline->Create( desc, this ) ) )
                {
                    Memory::FreeObject( m_pDefaultAllocator, &pPipeline );
                    return Results::FAIL;
                }

                ( *ppPipeline ) = pPipeline;

                return Results::OK;
            }

            void RenderSystem::DestroyPipeline( Pipeline** ppPipeline )
            {
                BGS_ASSERT( ppPipeline != nullptr, "Pipeline (ppPipeline) must be a valid address." );
                BGS_ASSERT( *ppPipeline != nullptr, "Pipeline (*ppPipeline) must be a valid pointer." );

                Pipeline* pPipeline = ( *ppPipeline );
                pPipeline->Destroy();
                Memory::FreeObject( m_pDefaultAllocator, &pPipeline );
            }

            RESULT RenderSystem::CreateSwapchain( const SwapchainDesc& desc, Swapchain** ppSwapchain )
            {
                BGS_ASSERT( ppSwapchain != nullptr, "Swapchain (ppSwapchain) must be a valid address." );
                BGS_ASSERT( *ppSwapchain == nullptr, "There is a valid pointer at the given address. Swapchain (*ppSwapchain) must be nullptr." );

                Swapchain* pSwapchain = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( m_pDefaultAllocator, &pSwapchain ) ) )
                {
                    return Results::NO_MEMORY;
                }

                if( BGS_FAILED( pSwapchain->Create( desc, this ) ) )
                {
                    Memory::FreeObject( m_pDefaultAllocator, &pSwapchain );
                    return Results::FAIL;
                }

                ( *ppSwapchain ) = pSwapchain;

                return Results::OK;
            }

            void RenderSystem::DestroySwapchain( Swapchain** ppSwapchain )
            {
                BGS_ASSERT( ppSwapchain != nullptr, "Swapchain (ppSwapchain) must be a valid address." );
                BGS_ASSERT( *ppSwapchain != nullptr, "Swapchain (*ppSwapchain) must be a valid pointer." );

                Swapchain* pSwapchain = ( *ppSwapchain );
                pSwapchain->Destroy();
                Memory::FreeObject( m_pDefaultAllocator, &pSwapchain );
            }

            RESULT RenderSystem::CreateCamera( const CameraDesc& desc, Camera** ppCamera )
            {
                BGS_ASSERT( ppCamera != nullptr, "Camera (ppCamera) must be a valid address." );

                Camera* pCamera = ( *ppCamera );
                if( pCamera != nullptr )
                {
                    for( index_t ndx = 0; ndx < m_cameras.size(); ++ndx )
                    {
                        // TODO: Find camera in array using hash
                        if( m_cameras[ ndx ] == pCamera )
                        {
                            return Results::OK;
                        }
                    }

                    // TDOD: Warrning that camera wasn't created by framework?
                    m_cameras.push_back( pCamera );

                    return Results::OK;
                }
                else
                {
                    if( BGS_FAILED( Memory::AllocateObject( m_pDefaultAllocator, &pCamera ) ) )
                    {
                        return Results::NO_MEMORY;
                    }

                    if( BGS_FAILED( pCamera->Create( desc, this ) ) )
                    {
                        Memory::FreeObject( m_pDefaultAllocator, &pCamera );
                        return Results::FAIL;
                    }
                }

                m_cameras.push_back( pCamera );
                ( *ppCamera ) = pCamera;

                return Results::OK;
            }

            void RenderSystem::DestroyCamera( Camera** ppCamera )
            {
                BGS_ASSERT( ( ppCamera != nullptr ) && ( *ppCamera != nullptr ), " Camera (ppCamera) must be an valid address of a valid pointer." );

                if( ppCamera != nullptr )
                {

                    Camera* pCamera = ( *ppCamera );

                    for( index_t ndx = 0; ndx < m_cameras.size(); ++ndx )
                    {
                        // TODO: Find device in array using hash
                        if( m_cameras[ ndx ] == pCamera )
                        {
                            m_cameras[ ndx ] = m_cameras.back();
                            m_cameras.pop_back();
                            pCamera->Destroy();
                            Memory::FreeObject( m_pDefaultAllocator, &pCamera );
                            ppCamera = nullptr;
                            break;
                        }
                    }
                }
            }

            RESULT RenderSystem::Create( const RenderSystemDesc& desc, Core::Memory::IAllocator* pAllocator, BigosEngine* pEngine )
            {
                BGS_ASSERT( pEngine != nullptr, "Bigos engine (pEngine) must be a valid pointer." );
                BGS_ASSERT( pAllocator != nullptr, "Allocator (pAllocator) must be a valid pointer." );

                m_desc              = desc;
                m_pDefaultAllocator = pAllocator;
                m_pParent           = pEngine;

                if( BGS_FAILED( CreateShaderCompilerFactory( desc.compilerFactoryDesc, &m_pShaderCompilerFactory ) ) )
                {
                    DestroyFactory( &m_pFactory );
                    return Results::FAIL;
                }

                ShaderCompilerDesc compDesc;
                compDesc.type = CompilerTypes::DXC;
                if( BGS_FAILED( m_pShaderCompilerFactory->CreateCompiler( compDesc, &m_pCompiler ) ) )
                {
                    DestroyShaderCompilerFactory( &m_pShaderCompilerFactory );
                    DestroyFactory( &m_pFactory );
                }

                return Results::OK;
            }

            void RenderSystem::Destroy()
            {
                FreeDriver();

                for( index_t ndx = 0; ndx < m_cameras.size(); ++ndx )
                {
                    DestroyCamera( &m_cameras[ ndx ] );
                }

                if( m_pShaderCompilerFactory != nullptr )
                {
                    DestroyShaderCompilerFactory( &m_pShaderCompilerFactory );
                }
                m_adapters.clear();

                m_pDefaultAllocator = nullptr;
                m_pParent           = nullptr;
            }

            RESULT RenderSystem::CreateFactory( const Backend::FactoryDesc& desc, Backend::IFactory** ppFactory )
            {
                BGS_ASSERT( ppFactory != nullptr, "Factory (ppFactory) must be a valid address." );
                BGS_ASSERT( *ppFactory == nullptr, "There is a pointer at the given address. Factory (*ppFactory) must be nullptr." );
                if( m_pFactory != nullptr )
                {
                    *ppFactory = m_pFactory;
                    return Results::OK;
                }

                if( ( ppFactory == nullptr ) || ( *ppFactory != nullptr ) )
                {
                    return Results::FAIL;
                }

                const Backend::API_TYPE apiType = desc.apiType;
                if( apiType == Backend::APITypes::VULKAN )
                {
                    Backend::VulkanFactory* pFactory = nullptr;
                    if( BGS_FAILED( Memory::AllocateObject( m_pDefaultAllocator, &pFactory ) ) )
                    {
                        return Results::NO_MEMORY;
                    }

                    if( BGS_FAILED( pFactory->Create( desc, this ) ) )
                    {
                        Memory::FreeObject( m_pDefaultAllocator, &pFactory );
                        return Results::FAIL;
                    }

                    m_pFactory = pFactory;
                }
                else if( apiType == Backend::APITypes::D3D12 )
                {
                    Backend::D3D12Factory* pFactory = nullptr;
                    if( BGS_FAILED( Memory::AllocateObject( m_pDefaultAllocator, &pFactory ) ) )
                    {
                        return Results::NO_MEMORY;
                    }

                    if( BGS_FAILED( pFactory->Create( desc, this ) ) )
                    {
                        Memory::FreeObject( m_pDefaultAllocator, &pFactory );
                        return Results::FAIL;
                    }

                    m_pFactory = pFactory;
                }
                else
                {
                    return Results::NOT_FOUND;
                }

                BGS_ASSERT( m_pFactory != nullptr );
                *ppFactory = m_pFactory;
                m_adapters = m_pFactory->GetAdapters();

                return Results::OK;
            }

            void RenderSystem::DestroyFactory( Backend::IFactory** ppFactory )
            {
                BGS_ASSERT( ppFactory != nullptr, "Factory (ppFactory) must be a valid address." );
                BGS_ASSERT( *ppFactory != nullptr, "Factory (*ppFactory) must be a valid pointer." );
                BGS_ASSERT( *ppFactory == m_pFactory, "Factory (*ppFactory) has not been created by Bigos Framework." );

                if( ( ppFactory != nullptr ) && ( *ppFactory != nullptr ) && ( *ppFactory == m_pFactory ) )
                {
                    const Backend::API_TYPE apiType = m_driverDesc.apiType;
                    if( apiType == Backend::APITypes::VULKAN )
                    {
                        Backend::VulkanFactory* pFactory = static_cast<Backend::VulkanFactory*>( *ppFactory );
                        pFactory->Destroy();
                        Core::Memory::FreeObject( m_pDefaultAllocator, &pFactory );
                        pFactory = nullptr;
                    }
                    else if( apiType == Backend::APITypes::D3D12 )
                    {
                        Backend::D3D12Factory* pFactory = static_cast<Backend::D3D12Factory*>( *ppFactory );
                        pFactory->Destroy();
                        Core::Memory::FreeObject( m_pDefaultAllocator, &pFactory );
                        pFactory = nullptr;
                    }
                    else
                    {
                        BGS_ASSERT( 0 );
                    }
                }
            }

            RESULT RenderSystem::CreateContexts()
            {
                BGS_ASSERT( m_pDevice != nullptr );
                BGS_ASSERT( m_pGraphicsContext == nullptr );
                if( BGS_FAILED( Memory::AllocateObject( m_pDefaultAllocator, &m_pGraphicsContext ) ) )
                {
                    return Results::NO_MEMORY;
                }
                if( BGS_FAILED( m_pGraphicsContext->Create( m_pDevice, this ) ) )
                {
                    Memory::FreeObject( m_pDefaultAllocator, &m_pGraphicsContext );
                    return Results::FAIL;
                }

                BGS_ASSERT( m_pComputeContext == nullptr );
                if( BGS_FAILED( Memory::AllocateObject( m_pDefaultAllocator, &m_pComputeContext ) ) )
                {
                    return Results::NO_MEMORY;
                }
                if( BGS_FAILED( m_pComputeContext->Create( m_pDevice, this ) ) )
                {
                    DestroyContexts();
                    Memory::FreeObject( m_pDefaultAllocator, &m_pComputeContext );
                    return Results::FAIL;
                }

                BGS_ASSERT( m_pCopyContext == nullptr );
                if( BGS_FAILED( Memory::AllocateObject( m_pDefaultAllocator, &m_pCopyContext ) ) )
                {
                    return Results::NO_MEMORY;
                }
                if( BGS_FAILED( m_pCopyContext->Create( m_pDevice, this ) ) )
                {
                    DestroyContexts();
                    Memory::FreeObject( m_pDefaultAllocator, &m_pCopyContext );
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void RenderSystem::DestroyContexts()
            {
                if( m_pGraphicsContext != nullptr )
                {
                    m_pGraphicsContext->Destroy();
                    Memory::FreeObject( m_pDefaultAllocator, &m_pGraphicsContext );
                }
                if( m_pComputeContext != nullptr )
                {
                    m_pComputeContext->Destroy();
                    Memory::FreeObject( m_pDefaultAllocator, &m_pComputeContext );
                }
                if( m_pCopyContext != nullptr )
                {
                    m_pCopyContext->Destroy();
                    Memory::FreeObject( m_pDefaultAllocator, &m_pCopyContext );
                }
            }

            RESULT RenderSystem::CreateShaderCompilerFactory( const ShaderCompilerFactoryDesc& desc, ShaderCompilerFactory** ppFactory )
            {
                BGS_ASSERT( ppFactory != nullptr, "Shader compiler factory (ppFactory) must be a valid address." );
                BGS_ASSERT( *ppFactory == nullptr, "There is a pointer at the given address. Shader compiler factory (*ppFactory) must be nullptr." );
                if( ( ppFactory == nullptr ) || ( *ppFactory != nullptr ) )
                {
                    return Results::FAIL;
                }

                if( m_pShaderCompilerFactory != nullptr )
                {
                    *ppFactory = m_pShaderCompilerFactory;
                    return Results::OK;
                }

                if( BGS_FAILED( Memory::AllocateObject( m_pDefaultAllocator, &m_pShaderCompilerFactory ) ) )
                {
                    return Results::NO_MEMORY;
                }

                if( BGS_FAILED( m_pShaderCompilerFactory->Create( desc, this ) ) )
                {
                    Memory::FreeObject( m_pDefaultAllocator, &m_pShaderCompilerFactory );
                    return Results::FAIL;
                }

                *ppFactory = m_pShaderCompilerFactory;

                return Results::OK;
            }

            void RenderSystem::DestroyShaderCompilerFactory( ShaderCompilerFactory** ppFactory )
            {
                BGS_ASSERT( ppFactory != nullptr, "Shader compiler factory (ppFactory) must be a valid address." );
                BGS_ASSERT( *ppFactory != nullptr, "Shader compiler factory (*ppFactory) must be a valid pointer." );
                BGS_ASSERT( *ppFactory == m_pShaderCompilerFactory, "Shader compiler factory (*ppFactory) has not been created by Bigos Framework." );
                if( ( ppFactory != nullptr ) && ( *ppFactory != nullptr ) && ( *ppFactory == m_pShaderCompilerFactory ) )
                {
                    m_pShaderCompilerFactory->Destroy();
                    Memory::FreeObject( m_pDefaultAllocator, &m_pShaderCompilerFactory );
                }
            }

            void RenderSystem::FreeDriver()
            {
                DestroyContexts();

                // Free all the resources created on this device
                if( m_pDevice != nullptr )
                {
                    m_pFactory->DestroyDevice( &m_pDevice );
                }

                if( m_pFactory != nullptr )
                {
                    DestroyFactory( &m_pFactory );
                }
            }

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS