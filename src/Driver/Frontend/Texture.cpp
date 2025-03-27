#pragma once

#include "Driver/Frontend/Texture.h"

#include "Driver/Frontend/RenderSystem.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            static const Backend::RESOURCE_TYPE MapTextureTypeToResourceType( TEXTURE_TYPE type )
            {
                static const Backend::RESOURCE_TYPE translateTable[ BGS_ENUM_COUNT( TextureTypes ) ] = {
                    Backend::ResourceTypes::TEXTURE_1D,   // TEXTURE_1D
                    Backend::ResourceTypes::TEXTURE_2D,   // TEXTURE_2D
                    Backend::ResourceTypes::TEXTURE_3D,   // TEXTURE_3D
                    Backend::ResourceTypes::TEXTURE_CUBE, // TEXTURE_CUBE
                };

                return translateTable[ BGS_ENUM_INDEX( type ) ];
            }

            static const Backend::TEXTURE_TYPE MapTextureTypeToBackendTextureType( TEXTURE_TYPE type )
            {
                static const Backend::TEXTURE_TYPE translateTable[ BGS_ENUM_COUNT( TextureTypes ) ] = {
                    Backend::TextureTypes::TEXTURE_1D,   // TEXTURE_1D
                    Backend::TextureTypes::TEXTURE_2D,   // TEXTURE_2D
                    Backend::TextureTypes::TEXTURE_3D,   // TEXTURE_3D
                    Backend::TextureTypes::TEXTURE_CUBE, // TEXTURE_CUBE
                };

                return translateTable[ BGS_ENUM_INDEX( type ) ];
            }

            static const Backend::TEXTURE_TYPE MapTextureTypeToBackendTextureArrayType( TEXTURE_TYPE type )
            {
                static const Backend::TEXTURE_TYPE translateTable[ BGS_ENUM_COUNT( TextureTypes ) ] = {
                    Backend::TextureTypes::TEXTURE_1D_ARRAY, // TEXTURE_1D
                    Backend::TextureTypes::TEXTURE_2D_ARRAY, // TEXTURE_2D
                    Backend::TextureTypes::_MAX_ENUM,        // TEXTURE_3D
                    Backend::TextureTypes::_MAX_ENUM,        // TEXTURE_CUBE
                };

                return translateTable[ BGS_ENUM_INDEX( type ) ];
            }

            static const Backend::TEXTURE_TYPE GetTextureViewType( const TextureDesc& desc )
            {
                Backend::TEXTURE_TYPE type = Backend::TextureTypes::_MAX_ENUM;
                if( desc.type == TextureTypes::TEXTURE_CUBE )
                {
                    type = Backend::TextureTypes::TEXTURE_CUBE;
                }
                else
                {
                    if( desc.arrayLayerCount == 1 )
                    {
                        type = MapTextureTypeToBackendTextureType( desc.type );
                    }
                    else
                    {
                        type = MapTextureTypeToBackendTextureArrayType( desc.type );
                    }
                }

                BGS_ASSERT( type != Backend::TextureTypes::_MAX_ENUM );

                return type;
            }

            Texture::Texture()
                : m_desc()
                , m_pParent( nullptr )
                , m_hMemory()
                , m_hResource()
                , m_hSampleAccess()
                , m_hStorageAccess()
            {
            }

            RESULT Texture::Create( const TextureDesc& desc, RenderSystem* pSystem )
            {
                BGS_ASSERT( pSystem != nullptr, "Render device (pDevice) must be a valid pointer." );
                m_pParent                    = pSystem;
                m_desc                       = desc;
                Backend::IDevice* pAPIDevice = m_pParent->GetDevice();

                Backend::ResourceUsageFlags usage = BGS_FLAG( Backend::ResourceUsageFlagBits::TRANSFER_DST );
                if( m_desc.usage & BGS_FLAG( TextureUsageFlagBits::SAMPLED ) )
                {
                    usage |= BGS_FLAG( Backend::ResourceUsageFlagBits::SAMPLED_TEXTURE );
                }
                if( m_desc.usage & BGS_FLAG( TextureUsageFlagBits::STORAGE ) )
                {
                    usage |= BGS_FLAG( Backend::ResourceUsageFlagBits::STORAGE_TEXTURE );
                    usage |= BGS_FLAG( Backend::ResourceUsageFlagBits::TRANSFER_SRC );
                }

                BIGOS::Driver::Backend::ResourceDesc texDesc;
                texDesc.format          = m_desc.format;
                texDesc.size            = m_desc.size;
                texDesc.arrayLayerCount = m_desc.arrayLayerCount;
                texDesc.mipLevelCount   = m_desc.mipLevelCount;
                texDesc.resourceUsage   = usage;
                texDesc.resourceLayout  = BIGOS::Driver::Backend::ResourceLayouts::OPTIMAL;
                texDesc.resourceType    = MapTextureTypeToResourceType( m_desc.type );
                texDesc.sampleCount     = m_desc.sampleCount;
                texDesc.sharingMode     = BIGOS::Driver::Backend::ResourceSharingModes::EXCLUSIVE_ACCESS;
                if( BGS_FAILED( pAPIDevice->CreateResource( texDesc, &m_hResource ) ) )
                {
                    return Results::FAIL;
                }

                Backend::ResourceAllocationInfo allocInf;
                pAPIDevice->GetResourceAllocationInfo( m_hResource, &allocInf );
                Backend::AllocateMemoryDesc allocDesc;
                allocDesc.size      = allocInf.size;
                allocDesc.alignment = allocInf.alignment;
                allocDesc.heapType  = Backend::MemoryHeapTypes::DEFAULT;
                allocDesc.heapUsage = Backend::MemoryHeapUsages::TEXTURES;
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

                Backend::TextureViewDesc viewDesc;
                viewDesc.format                = m_desc.format;
                viewDesc.textureType           = GetTextureViewType( m_desc );
                viewDesc.range.components      = BGS_FLAG( Backend::TextureComponentFlagBits::COLOR );
                viewDesc.range.mipLevel        = 0;
                viewDesc.range.mipLevelCount   = m_desc.mipLevelCount;
                viewDesc.range.arrayLayer      = 0;
                viewDesc.range.arrayLayerCount = m_desc.arrayLayerCount;
                viewDesc.hResource             = m_hResource;
                if( m_desc.usage & BGS_FLAG( TextureUsageFlagBits::SAMPLED ) )
                {
                    viewDesc.usage = BGS_FLAG( Backend::ResourceViewUsageFlagBits::SAMPLED_TEXTURE );
                    if( BGS_FAILED( pAPIDevice->CreateResourceView( viewDesc, &m_hSampleAccess ) ) )
                    {
                        Destroy();
                        return Results::FAIL;
                    }
                }
                if( m_desc.usage & BGS_FLAG( TextureUsageFlagBits::SAMPLED ) )
                {
                    viewDesc.usage = BGS_FLAG( Backend::ResourceViewUsageFlagBits::SAMPLED_TEXTURE );
                    if( BGS_FAILED( pAPIDevice->CreateResourceView( viewDesc, &m_hStorageAccess ) ) )
                    {
                        Destroy();
                        return Results::FAIL;
                    }
                }

                return Results::OK;
            }

            void Texture::Destroy()
            {
                Backend::IDevice* pAPIDevice = m_pParent->GetDevice();

                if( m_hSampleAccess != Backend::ResourceViewHandle() )
                {
                    pAPIDevice->DestroyResourceView( &m_hSampleAccess );
                }
                if( m_hStorageAccess != Backend::ResourceViewHandle() )
                {
                    pAPIDevice->DestroyResourceView( &m_hStorageAccess );
                }
                if( m_hResource != Backend::ResourceHandle() )
                {
                    pAPIDevice->DestroyResource( &m_hResource );
                }
                if( m_hMemory != Backend::MemoryHandle() )
                {
                    pAPIDevice->FreeMemory( &m_hMemory );
                }
            }

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS