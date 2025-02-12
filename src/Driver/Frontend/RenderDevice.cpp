#include "Driver/Frontend/RenderDevice.h"

#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Memory.h"
#include "Driver/Frontend/Buffer.h"
#include "Driver/Frontend/Contexts.h"
#include "Driver/Frontend/RenderSystem.h"
#include "Driver/Frontend/RenderTarget.h"

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

                Buffer* pBuffer = ( *ppBuffer );
                pBuffer->Destroy();
                Memory::FreeObject( m_pParent->GetDefaultAllocator(), &pBuffer );
            }

            RESULT RenderDevice::CreateRenderTarget( const RenderTargetDesc& desc, RenderTarget** ppRenderTarget )
            {
                BGS_ASSERT( ppRenderTarget != nullptr, "Render target (ppRenderTarget) must be a valid address." );

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

                RenderTarget* pRenderTarget = ( *ppRenderTarget );
                pRenderTarget->Destroy();
                Memory::FreeObject( m_pParent->GetDefaultAllocator(), &pRenderTarget );
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
                BGS_ASSERT( m_pGraphicsContext != nullptr );
                if( BGS_FAILED( Memory::AllocateObject( pAllocator, &m_pGraphicsContext ) ) )
                {
                    return Results::NO_MEMORY;
                }
                if( BGS_FAILED( m_pGraphicsContext->Create( this ) ) )
                {
                    Memory::FreeObject( pAllocator, &m_pGraphicsContext );
                    return Results::FAIL;
                }

                BGS_ASSERT( m_pComputeContext != nullptr );
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

                BGS_ASSERT( m_pCopyContext != nullptr );
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