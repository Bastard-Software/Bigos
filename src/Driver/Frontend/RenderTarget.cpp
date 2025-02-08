#include "Driver/Frontend/RenderTarget.h"

#include "Driver/Backend/APICommon.h"
#include "Driver/Frontend/RenderDevice.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {
            RenderTarget::RenderTarget()
                : m_desc()
                , m_targetDesc()
                , m_pParent( nullptr )
                , m_hMemory()
                , m_hResource()
                , m_hView()
                , m_hSampleView()
            {
            }

            RESULT RenderTarget::Create( const RenderTargetDesc& desc, RenderDevice* pDevice )
            {
                BGS_ASSERT( pDevice != nullptr, "Render device (pDevice) must be a valid pointer." );
                m_pParent                    = pDevice;
                m_desc                       = desc;
                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                const bool_t isDepthStencil = Backend::IsDepthStencilFormat( m_desc.format );

                m_targetDesc.format          = m_desc.format;
                m_targetDesc.size.width      = m_desc.width;
                m_targetDesc.size.height     = m_desc.height;
                m_targetDesc.size.depth      = 1;
                m_targetDesc.mipLevelCount   = 1;
                m_targetDesc.arrayLayerCount = 1;
                m_targetDesc.resourceType    = Backend::ResourceTypes::TEXTURE_2D;
                m_targetDesc.resourceLayout  = Backend::ResourceLayouts::OPTIMAL;
                m_targetDesc.sharingMode     = Backend::ResourceSharingModes::EXCLUSIVE_ACCESS;
                m_targetDesc.sampleCount     = m_desc.sampleCount;
                if( isDepthStencil )
                {
                    m_targetDesc.resourceUsage        = BGS_FLAG( Backend::ResourceUsageFlagBits::DEPTH_STENCIL_TARGET );
                    m_targetDesc.depthStencilClrValue = { 1.0f, 0 };
                }
                else
                {
                    m_targetDesc.resourceUsage = BGS_FLAG( Backend::ResourceUsageFlagBits::COLOR_RENDER_TARGET );
                    m_targetDesc.colorClrValue = { 1.0f, 1.0f, 1.0f, 1.0f };
                }
                if( m_desc.allowSampling )
                {
                    m_targetDesc.resourceUsage |= BGS_FLAG( Backend::ResourceUsageFlagBits::SAMPLED_TEXTURE );
                }
                if( m_desc.allowCopying )
                {
                    m_targetDesc.resourceUsage |= BGS_FLAG( Backend::ResourceUsageFlagBits::TRANSFER_SRC );
                }
                if( BGS_FAILED( pAPIDevice->CreateResource( m_targetDesc, &m_hResource ) ) )
                {
                    return Results::FAIL;
                }

                Backend::ResourceAllocationInfo allocInf;
                pAPIDevice->GetResourceAllocationInfo( m_hResource, &allocInf );
                Backend::AllocateMemoryDesc allocDesc;
                allocDesc.size      = allocInf.size;
                allocDesc.alignment = allocInf.alignment;
                allocDesc.heapType  = Backend::MemoryHeapTypes::DEFAULT;
                allocDesc.heapUsage = Backend::MemoryHeapUsages::RENDER_TARGETS;
                if( BGS_FAILED( pAPIDevice->AllocateMemory( allocDesc, &m_hMemory ) ) )
                {
                    Destroy();
                    return Results::FAIL;
                }
                Backend::BindResourceMemoryDesc bindMemDesc;
                bindMemDesc.hMemory      = m_hMemory;
                bindMemDesc.hResource    = m_hResource;
                bindMemDesc.memoryOffset = 0;
                if( BGS_FAILED( pAPIDevice->BindResourceMemory( bindMemDesc ) ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                Backend::TextureComponentFlags component = 0;
                if( isDepthStencil )
                {
                    if( Backend::IsDepthFormat( m_desc.format ) )
                    {
                        component |= BGS_FLAG( Backend::TextureComponentFlagBits::DEPTH );
                    }
                    if( Backend::IsStencilFormat( m_desc.format ) )
                    {
                        component |= BGS_FLAG( Backend::TextureComponentFlagBits::STENCIL );
                    }
                }
                else
                {
                    component = BGS_FLAG( Backend::TextureComponentFlagBits::COLOR );
                }
                Backend::TextureViewDesc viewDesc;
                viewDesc.usage                 = isDepthStencil ? BGS_FLAG( Backend::ResourceViewUsageFlagBits::DEPTH_STENCIL_TARGET )
                                                                : BGS_FLAG( Backend::ResourceViewUsageFlagBits::COLOR_RENDER_TARGET );
                viewDesc.format                = m_desc.format;
                viewDesc.textureType           = Backend::TextureTypes::TEXTURE_2D;
                viewDesc.range.components      = component;
                viewDesc.range.mipLevel        = 0;
                viewDesc.range.mipLevelCount   = 1;
                viewDesc.range.arrayLayer      = 0;
                viewDesc.range.arrayLayerCount = 1;
                viewDesc.hResource             = m_hResource;
                if( BGS_FAILED( pAPIDevice->CreateResourceView( viewDesc, &m_hView ) ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                if( m_desc.allowSampling )
                {
                    viewDesc.usage = BGS_FLAG( Backend::ResourceViewUsageFlagBits::SAMPLED_TEXTURE );
                    if( BGS_FAILED( pAPIDevice->CreateResourceView( viewDesc, &m_hSampleView ) ) )
                    {
                        Destroy();
                        return Results::FAIL;
                    }
                }

                return Results::OK;
            }

            void RenderTarget::Destroy()
            {
                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                if( m_hSampleView != Backend::ResourceViewHandle() )
                {
                    pAPIDevice->DestroyResourceView( &m_hSampleView );
                }
                if( m_hView != Backend::ResourceViewHandle() )
                {
                    pAPIDevice->DestroyResourceView( &m_hView );
                }
                if( m_hMemory != Backend::MemoryHandle() )
                {
                    pAPIDevice->FreeMemory( &m_hMemory );
                }
                if( m_hResource != Backend::ResourceHandle() )
                {
                    pAPIDevice->DestroyResource( &m_hResource );
                }
            }

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS