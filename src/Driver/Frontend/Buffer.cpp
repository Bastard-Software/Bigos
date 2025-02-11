#include "Driver/Frontend/Buffer.h"

#include "Driver/Frontend/RenderDevice.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            Buffer::Buffer()
                : m_desc()
                , m_pParent( nullptr )
                , m_hMemory()
                , m_hResource()
                , m_hConstantAccess()
                , m_hReadAccess()
                , m_hReadWriteAccess()
            {
            }

            RESULT Buffer::Map( void** ppHost )
            {
                BGS_ASSERT( m_desc.usage & BGS_FLAG( Backend::ResourceUsageFlagBits::CONSTANT_BUFFER ),
                            "Unable to unmap buffer that is not constant buffer." );

                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                BIGOS::Driver::Backend::MapResourceDesc mapConstant;
                mapConstant.hResource          = m_hResource;
                mapConstant.bufferRange.size   = m_desc.size;
                mapConstant.bufferRange.offset = 0;

                if( BGS_FAILED( pAPIDevice->MapResource( mapConstant, ppHost ) ) )
                {
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void Buffer::Unmap()
            {
                BGS_ASSERT( m_desc.usage & BGS_FLAG( Backend::ResourceUsageFlagBits::CONSTANT_BUFFER ),
                            "Unable to unmap buffer that is not constant buffer." );

                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                BIGOS::Driver::Backend::MapResourceDesc mapConstant;
                mapConstant.hResource          = m_hResource;
                mapConstant.bufferRange.size   = m_desc.size;
                mapConstant.bufferRange.offset = 0;

                pAPIDevice->UnmapResource( mapConstant );
            }

            RESULT Buffer::Create( const BufferDesc& desc, RenderDevice* pDevice )
            {
                BGS_ASSERT( pDevice != nullptr, "Render device (pDevice) must be a valid pointer." );
                m_pParent                    = pDevice;
                m_desc                       = desc;
                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                Backend::ResourceUsageFlags usage = m_desc.usage;
                if( !( m_desc.usage & BGS_FLAG( Backend::ResourceUsageFlagBits::CONSTANT_BUFFER ) ) )
                {
                    usage |= BGS_FLAG( Backend::ResourceUsageFlagBits::TRANSFER_DST );
                }

                BIGOS::Driver::Backend::ResourceDesc buffDesc;
                buffDesc.format          = BIGOS::Driver::Backend::Formats::UNKNOWN;
                buffDesc.size.width      = m_desc.size;
                buffDesc.size.height     = 1;
                buffDesc.size.depth      = 1;
                buffDesc.arrayLayerCount = 1;
                buffDesc.mipLevelCount   = 1;
                buffDesc.resourceUsage   = m_desc.usage;
                buffDesc.resourceLayout  = BIGOS::Driver::Backend::ResourceLayouts::LINEAR;
                buffDesc.resourceType    = BIGOS::Driver::Backend::ResourceTypes::BUFFER;
                buffDesc.sampleCount     = BIGOS::Driver::Backend::SampleCount::COUNT_1;
                buffDesc.sharingMode     = BIGOS::Driver::Backend::ResourceSharingModes::EXCLUSIVE_ACCESS;
                if( BGS_FAILED( pAPIDevice->CreateResource( buffDesc, &m_hResource ) ) )
                {
                    return BIGOS::Results::FAIL;
                }

                BIGOS::Driver::Backend::ResourceAllocationInfo allocInfo;
                pAPIDevice->GetResourceAllocationInfo( m_hResource, &allocInfo );

                BIGOS::Driver::Backend::AllocateMemoryDesc allocDesc;
                allocDesc.size      = allocInfo.size;
                allocDesc.alignment = allocInfo.alignment;
                allocDesc.heapType  = m_desc.usage & BGS_FLAG( Backend::ResourceUsageFlagBits::CONSTANT_BUFFER ) ? Backend::MemoryHeapTypes::UPLOAD
                                                                                                                 : Backend::MemoryHeapTypes::DEFAULT;
                allocDesc.heapUsage = Backend::MemoryHeapUsages::BUFFERS;
                if( BGS_FAILED( pAPIDevice->AllocateMemory( allocDesc, &m_hMemory ) ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                BIGOS::Driver::Backend::BindResourceMemoryDesc bindDesc;
                bindDesc.hMemory      = m_hMemory;
                bindDesc.hResource    = m_hResource;
                bindDesc.memoryOffset = 0;
                if( BGS_FAILED( pAPIDevice->BindResourceMemory( bindDesc ) ) )
                {
                    Destroy();
                    return BIGOS::Results::FAIL;
                }

                BIGOS::Driver::Backend::BufferViewDesc viewDesc;
                viewDesc.hResource    = m_hResource;
                viewDesc.range.offset = 0;
                viewDesc.range.size   = m_desc.size;
                if( m_desc.usage & BGS_FLAG( Backend::ResourceUsageFlagBits::CONSTANT_BUFFER ) )
                {
                    viewDesc.usage = BGS_FLAG( BIGOS::Driver::Backend::ResourceViewUsageFlagBits::CONSTANT_BUFFER );
                    if( BGS_FAILED( pAPIDevice->CreateResourceView( viewDesc, &m_hConstantAccess ) ) )
                    {
                        Destroy();
                        return Results::FAIL;
                    }
                }
                if( m_desc.usage & BGS_FLAG( Backend::ResourceUsageFlagBits::READ_ONLY_STORAGE_BUFFER ) )
                {
                    viewDesc.usage = BGS_FLAG( BIGOS::Driver::Backend::ResourceViewUsageFlagBits::READ_ONLY_STORAGE_BUFFER );
                    if( BGS_FAILED( pAPIDevice->CreateResourceView( viewDesc, &m_hReadAccess ) ) )
                    {
                        Destroy();
                        return Results::FAIL;
                    }
                }
                if( m_desc.usage & BGS_FLAG( Backend::ResourceUsageFlagBits::READ_WRITE_STORAGE_BUFFER ) )
                {
                    viewDesc.usage = BGS_FLAG( BIGOS::Driver::Backend::ResourceViewUsageFlagBits::READ_WRITE_STORAGE_BUFFER );
                    if( BGS_FAILED( pAPIDevice->CreateResourceView( viewDesc, &m_hReadWriteAccess ) ) )
                    {
                        Destroy();
                        return Results::FAIL;
                    }
                }

                return Results::OK;
            }

            void Buffer::Destroy()
            {
                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                if( m_hMemory != Backend::MemoryHandle() )
                {
                    pAPIDevice->FreeMemory( &m_hMemory );
                }
                if( m_hResource != Backend::ResourceHandle() )
                {
                    pAPIDevice->DestroyResource( &m_hResource );
                }
                if( m_hConstantAccess != Backend::ResourceViewHandle() )
                {
                    pAPIDevice->DestroyResourceView( &m_hConstantAccess );
                }
                if( m_hReadAccess != Backend::ResourceViewHandle() )
                {
                    pAPIDevice->DestroyResourceView( &m_hReadAccess );
                }
                if( m_hReadWriteAccess != Backend::ResourceViewHandle() )
                {
                    pAPIDevice->DestroyResourceView( &m_hReadWriteAccess );
                }
            }

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS