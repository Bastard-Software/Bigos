#include "Driver/Frontend/RenderDevice.h"

#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Memory.h"
#include "Driver/Frontend/Buffer.h"
#include "Driver/Frontend/Contexts.h"
#include "Driver/Frontend/Pipeline.h"
#include "Driver/Frontend/RenderPass.h"
#include "Driver/Frontend/RenderSystem.h"
#include "Driver/Frontend/RenderTarget.h"
#include "Driver/Frontend/Shader/Shader.h"
#include "Driver/Frontend/Swapchain.h"
#include "Driver/Frontend/Texture.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {
            RenderDevice::RenderDevice()
                : m_desc()
                , m_pFactory( nullptr )
                , m_pParent( nullptr )
                , m_pAPIDevice( nullptr )
                , m_pGraphicsContext( nullptr )
                , m_pComputeContext( nullptr )
                , m_pCopyContext( nullptr )
            {
            }

            RESULT RenderDevice::CreateBuffer( const BufferDesc& desc, Buffer** ppBuffer )
            {
                BGS_ASSERT( ppBuffer != nullptr, "Buffer (ppBuffer) must be a valid address." );
                BGS_ASSERT( *ppBuffer == nullptr, "There is a valid pointer at the given address. Buffer (*ppBuffer) must be nullptr." );

                Memory::IAllocator* pAllocator = m_pParent->GetDefaultAllocator();
                Buffer*             pBuffer    = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( pAllocator, &pBuffer ) ) )
                {
                    return Results::NO_MEMORY;
                }

                if( BGS_FAILED( pBuffer->Create( desc, this ) ) )
                {
                    Memory::FreeObject( pAllocator, &pBuffer );
                    return Results::FAIL;
                }

                ( *ppBuffer ) = pBuffer;

                return Results::OK;
            }

            void RenderDevice::DestroyBuffer( Buffer** ppBuffer )
            {
                BGS_ASSERT( ppBuffer != nullptr, "Buffer (ppBuffer) must be a valid address." );
                BGS_ASSERT( *ppBuffer != nullptr, "Buffer (*ppBuffer) must be a valid pointer." );

                Buffer* pBuffer = ( *ppBuffer );
                pBuffer->Destroy();
                Memory::FreeObject( m_pParent->GetDefaultAllocator(), &pBuffer );
            }

            RESULT RenderDevice::CreateTexture( const TextureDesc& desc, Texture** ppTexture )
            {
                BGS_ASSERT( ppTexture != nullptr, "Texture (ppTexture) must be a valid address." );
                BGS_ASSERT( *ppTexture == nullptr, "There is a valid pointer at the given address. Texture (*ppTExture) must be nullptr." );

                Memory::IAllocator* pAllocator = m_pParent->GetDefaultAllocator();
                Texture*            pTexture   = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( pAllocator, &pTexture ) ) )
                {
                    return Results::NO_MEMORY;
                }

                if( BGS_FAILED( pTexture->Create( desc, this ) ) )
                {
                    Memory::FreeObject( pAllocator, &pTexture );
                    return Results::FAIL;
                }

                ( *ppTexture ) = pTexture;

                return Results::OK;
            }

            void RenderDevice::DestroyTexture( Texture** ppTexture )
            {
                BGS_ASSERT( ppTexture != nullptr, "Texture (ppTexture) must be a valid address." );
                BGS_ASSERT( *ppTexture != nullptr, "Texture (*ppTexture) must be a valid pointer." );

                Texture* pTexture = ( *ppTexture );
                pTexture->Destroy();
                Memory::FreeObject( m_pParent->GetDefaultAllocator(), &pTexture );
            }

            RESULT RenderDevice::CreateRenderTarget( const RenderTargetDesc& desc, RenderTarget** ppRenderTarget )
            {
                BGS_ASSERT( ppRenderTarget != nullptr, "Render target (ppRenderTarget) must be a valid address." );
                BGS_ASSERT( *ppRenderTarget == nullptr,
                            "There is a valid pointer at the given address. Render target (*ppRenderTarget) must be nullptr." );

                Memory::IAllocator* pAllocator    = m_pParent->GetDefaultAllocator();
                RenderTarget*       pRenderTarget = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( pAllocator, &pRenderTarget ) ) )
                {
                    return Results::NO_MEMORY;
                }

                if( BGS_FAILED( pRenderTarget->Create( desc, this ) ) )
                {
                    Memory::FreeObject( pAllocator, &pRenderTarget );
                    return Results::FAIL;
                }

                ( *ppRenderTarget ) = pRenderTarget;

                return Results::OK;
            }

            void RenderDevice::DestroyRenderTarget( RenderTarget** ppRenderTarget )
            {
                BGS_ASSERT( ppRenderTarget != nullptr, "Render target (ppRenderTarget) must be a valid address." );
                BGS_ASSERT( *ppRenderTarget != nullptr, "Render target (*ppRenderTarget) must be a valid pointer." );

                RenderTarget* pRenderTarget = ( *ppRenderTarget );
                pRenderTarget->Destroy();
                Memory::FreeObject( m_pParent->GetDefaultAllocator(), &pRenderTarget );
            }

            RESULT RenderDevice::CreateRenderPass( const RenderPassDesc& desc, RenderPass** ppRenderPass )
            {
                BGS_ASSERT( ppRenderPass != nullptr, "Render pass (ppRenderPass) must be a valid address." );
                BGS_ASSERT( *ppRenderPass == nullptr,
                            "There is a valid pointer at the given address. Render target (*ppRenderPass) must be nullptr." );

                Memory::IAllocator* pAllocator  = m_pParent->GetDefaultAllocator();
                RenderPass*         pRenderPass = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( pAllocator, &pRenderPass ) ) )
                {
                    return Results::NO_MEMORY;
                }

                if( BGS_FAILED( pRenderPass->Create( desc, this ) ) )
                {
                    Memory::FreeObject( pAllocator, &pRenderPass );
                    return Results::FAIL;
                }

                ( *ppRenderPass ) = pRenderPass;

                return Results::OK;
            }

            void RenderDevice::DestroyRenderPass( RenderPass** ppRenderPass )
            {
                BGS_ASSERT( ppRenderPass != nullptr, "Render pass (ppRenderPass) must be a valid address." );
                BGS_ASSERT( *ppRenderPass != nullptr, "Render pass (*ppRenderPass) must be a valid pointer." );

                RenderPass* pRenderPass = ( *ppRenderPass );
                pRenderPass->Destroy();
                Memory::FreeObject( m_pParent->GetDefaultAllocator(), &pRenderPass );
            }

            RESULT RenderDevice::CreateShader( const ShaderDesc& desc, Shader** ppShader )
            {
                BGS_ASSERT( ppShader != nullptr, "Shader (ppShader) must be a valid address." );
                BGS_ASSERT( *ppShader == nullptr, "There is a valid pointer at the given address. Shader (*ppShader) must be nullptr." );

                Memory::IAllocator* pAllocator = m_pParent->GetDefaultAllocator();
                Shader*             pShader    = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( pAllocator, &pShader ) ) )
                {
                    return Results::NO_MEMORY;
                }

                if( BGS_FAILED( pShader->Create( desc, this ) ) )
                {
                    Memory::FreeObject( pAllocator, &pShader );
                    return Results::FAIL;
                }

                ( *ppShader ) = pShader;

                return Results::OK;
            }

            void RenderDevice::DestroyShader( Shader** ppShader )
            {
                BGS_ASSERT( ppShader != nullptr, "Shader (ppShader) must be a valid address." );
                BGS_ASSERT( *ppShader != nullptr, "Buffer (*ppShader) must be a valid pointer." );

                Shader* pShader = ( *ppShader );
                pShader->Destroy();
                Memory::FreeObject( m_pParent->GetDefaultAllocator(), &pShader );
            }

            RESULT RenderDevice::CreatePipeline( const GraphicsPipelineDesc& desc, Pipeline** ppPipeline )
            {
                BGS_ASSERT( ppPipeline != nullptr, "Pipeline (ppPipeline) must be a valid address." );
                BGS_ASSERT( *ppPipeline == nullptr, "There is a valid pointer at the given address. Pipeline (*ppPipeline) must be nullptr." );

                Memory::IAllocator* pAllocator = m_pParent->GetDefaultAllocator();
                Pipeline*           pPipeline  = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( pAllocator, &pPipeline ) ) )
                {
                    return Results::NO_MEMORY;
                }

                if( BGS_FAILED( pPipeline->Create( desc, this ) ) )
                {
                    Memory::FreeObject( pAllocator, &pPipeline );
                    return Results::FAIL;
                }

                ( *ppPipeline ) = pPipeline;

                return Results::OK;
            }

            RESULT RenderDevice::CreatePipeline( const ComputePipelineDesc& desc, Pipeline** ppPipeline )
            {
                BGS_ASSERT( ppPipeline != nullptr, "Pipeline (ppPipeline) must be a valid address." );
                BGS_ASSERT( *ppPipeline == nullptr, "There is a valid pointer at the given address. Pipeline (*ppPipeline) must be nullptr." );

                Memory::IAllocator* pAllocator = m_pParent->GetDefaultAllocator();
                Pipeline*           pPipeline  = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( pAllocator, &pPipeline ) ) )
                {
                    return Results::NO_MEMORY;
                }

                if( BGS_FAILED( pPipeline->Create( desc, this ) ) )
                {
                    Memory::FreeObject( pAllocator, &pPipeline );
                    return Results::FAIL;
                }

                ( *ppPipeline ) = pPipeline;

                return Results::OK;
            }

            void RenderDevice::DestroyPipeline( Pipeline** ppPipeline )
            {
                BGS_ASSERT( ppPipeline != nullptr, "Pipeline (ppPipeline) must be a valid address." );
                BGS_ASSERT( *ppPipeline != nullptr, "Pipeline (*ppPipeline) must be a valid pointer." );

                Pipeline* pPipeline = ( *ppPipeline );
                pPipeline->Destroy();
                Memory::FreeObject( m_pParent->GetDefaultAllocator(), &pPipeline );
            }

            RESULT RenderDevice::CreateSwapchain( const SwapchainDesc& desc, Swapchain** ppSwapchain )
            {
                BGS_ASSERT( ppSwapchain != nullptr, "Swapchain (ppSwapchain) must be a valid address." );
                BGS_ASSERT( *ppSwapchain == nullptr, "There is a valid pointer at the given address. Swapchain (*ppSwapchain) must be nullptr." );

                Memory::IAllocator* pAllocator = m_pParent->GetDefaultAllocator();
                Swapchain*          pSwapchain = nullptr;
                if( BGS_FAILED( Memory::AllocateObject( pAllocator, &pSwapchain ) ) )
                {
                    return Results::NO_MEMORY;
                }

                if( BGS_FAILED( pSwapchain->Create( desc, this ) ) )
                {
                    Memory::FreeObject( pAllocator, &pSwapchain );
                    return Results::FAIL;
                }

                ( *ppSwapchain ) = pSwapchain;

                return Results::OK;
            }

            void RenderDevice::DestroySwapchain( Swapchain** ppSwapchain )
            {
                BGS_ASSERT( ppSwapchain != nullptr, "Swapchain (ppSwapchain) must be a valid address." );
                BGS_ASSERT( *ppSwapchain != nullptr, "Swapchain (*ppSwapchain) must be a valid pointer." );

                Swapchain* pSwapchain = ( *ppSwapchain );
                pSwapchain->Destroy();
                Memory::FreeObject( m_pParent->GetDefaultAllocator(), &pSwapchain );
            }

            RESULT RenderDevice::Create( const RenderDeviceDesc& desc, Backend::IFactory* pFactory, RenderSystem* pDriverSystem )
            {
                BGS_ASSERT( pDriverSystem != nullptr, "Render system (pRenderSystem) must be a valid pointer." );
                BGS_ASSERT( pFactory != nullptr, "Factory (pFactory) must be a valid pointer." );

                m_desc     = desc;
                m_pFactory = pFactory;
                m_pParent  = pDriverSystem;

                Backend::DeviceDesc backendDesc;
                backendDesc.pAdapter = m_pFactory->GetAdapters()[ m_desc.adapter.index ];

                Backend::IDevice* pDevice = nullptr;
                if( BGS_FAILED( m_pFactory->CreateDevice( backendDesc, &pDevice ) ) )
                {
                    Results::FAIL;
                }
                m_pAPIDevice = pDevice;

                if( BGS_FAILED( CreateContexts() ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void RenderDevice::Destroy()
            {
                DestroyContexts();

                if( ( m_pFactory != nullptr ) && ( m_pAPIDevice != nullptr ) )
                {
                    m_pFactory->DestroyDevice( &m_pAPIDevice );
                }

                m_pParent  = nullptr;
                m_pFactory = nullptr;
            }

            RESULT RenderDevice::CreateContexts()
            {
                Memory::IAllocator* pAllocator = m_pParent->GetDefaultAllocator();
                BGS_ASSERT( m_pGraphicsContext == nullptr );
                if( BGS_FAILED( Memory::AllocateObject( pAllocator, &m_pGraphicsContext ) ) )
                {
                    return Results::NO_MEMORY;
                }
                if( BGS_FAILED( m_pGraphicsContext->Create( this ) ) )
                {
                    Memory::FreeObject( pAllocator, &m_pGraphicsContext );
                    return Results::FAIL;
                }

                BGS_ASSERT( m_pComputeContext == nullptr );
                if( BGS_FAILED( Memory::AllocateObject( pAllocator, &m_pComputeContext ) ) )
                {
                    return Results::NO_MEMORY;
                }
                if( BGS_FAILED( m_pComputeContext->Create( this ) ) )
                {
                    DestroyContexts();
                    Memory::FreeObject( pAllocator, &m_pComputeContext );
                    return Results::FAIL;
                }

                BGS_ASSERT( m_pCopyContext == nullptr );
                if( BGS_FAILED( Memory::AllocateObject( pAllocator, &m_pCopyContext ) ) )
                {
                    return Results::NO_MEMORY;
                }
                if( BGS_FAILED( m_pCopyContext->Create( this ) ) )
                {
                    DestroyContexts();
                    Memory::FreeObject( pAllocator, &m_pCopyContext );
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void RenderDevice::DestroyContexts()
            {
                Memory::IAllocator* pAllocator = m_pParent->GetDefaultAllocator();

                if( m_pGraphicsContext != nullptr )
                {
                    m_pGraphicsContext->Destroy();
                    Memory::FreeObject( pAllocator, &m_pGraphicsContext );
                }
                if( m_pComputeContext != nullptr )
                {
                    m_pComputeContext->Destroy();
                    Memory::FreeObject( pAllocator, &m_pComputeContext );
                }
                if( m_pCopyContext != nullptr )
                {
                    m_pCopyContext->Destroy();
                    Memory::FreeObject( pAllocator, &m_pCopyContext );
                }
            }

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS