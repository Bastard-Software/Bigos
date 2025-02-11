#include "BigosEngine/Renderer/Renderer.h"

#include "Driver/Frontend/RenderDevice.h"
#include "Driver/Frontend/RenderSystem.h"

namespace BIGOS
{
    Renderer::Renderer()
        : m_pRenderSystem( nullptr )
        , m_pDevice( nullptr )
        , m_pColorRT( nullptr )
        , m_pDepthRT( nullptr )
        , m_pVertexBuffer( nullptr )
        , m_pIndexBuffer( nullptr )
        , m_pConstantBuffer( nullptr )
        , m_pStorageBuffer( nullptr )
    {
    }

    RESULT Renderer::Create( Driver::Frontend::RenderSystem* pSystem )
    {
        BGS_ASSERT( pSystem != nullptr );
        m_pRenderSystem = pSystem;

        Driver::Frontend::RenderDeviceDesc devDesc;
        devDesc.adapter.index = 0;
        if( BGS_FAILED( m_pRenderSystem->CreateDevice( devDesc, &m_pDevice ) ) )
        {
            return Results::FAIL;
        }

        Driver::Frontend::RenderTargetDesc rtDesc;
        rtDesc.format        = Driver::Backend::Formats::R8G8B8A8_UNORM;
        rtDesc.allowSampling = BGS_TRUE;
        rtDesc.allowCopying  = BGS_TRUE;
        rtDesc.width         = 1280;
        rtDesc.height        = 720;
        rtDesc.sampleCount   = Driver::Backend::SampleCount::COUNT_2;
        if( BGS_FAILED( m_pDevice->CreateRenderTarget( rtDesc, &m_pColorRT ) ) )
        {
            return Results::FAIL;
        }

        rtDesc.format       = Driver::Backend::Formats::D24_UNORM_S8_UINT;
        rtDesc.allowCopying = BGS_FALSE;
        rtDesc.sampleCount  = Driver::Backend::SampleCount::COUNT_1;
        if( BGS_FAILED( m_pDevice->CreateRenderTarget( rtDesc, &m_pDepthRT ) ) )
        {
            Destroy();
            return Results::FAIL;
        }

        Driver::Frontend::BufferDesc bufferDesc;
        bufferDesc.size  = 65536;
        bufferDesc.usage = BGS_FLAG( Driver::Backend::ResourceUsageFlagBits::VERTEX_BUFFER );
        if( BGS_FAILED( m_pDevice->CreateBuffer( bufferDesc, &m_pVertexBuffer ) ) )
        {
            Destroy();
            return Results::FAIL;
        }
        bufferDesc.usage = BGS_FLAG( Driver::Backend::ResourceUsageFlagBits::INDEX_BUFFER );
        if( BGS_FAILED( m_pDevice->CreateBuffer( bufferDesc, &m_pIndexBuffer ) ) )
        {
            Destroy();
            return Results::FAIL;
        }
        bufferDesc.usage = BGS_FLAG( Driver::Backend::ResourceUsageFlagBits::READ_WRITE_STORAGE_BUFFER );
        if( BGS_FAILED( m_pDevice->CreateBuffer( bufferDesc, &m_pStorageBuffer ) ) )
        {
            Destroy();
            return Results::FAIL;
        }
        bufferDesc.size  = 256;
        bufferDesc.usage = BGS_FLAG( Driver::Backend::ResourceUsageFlagBits::CONSTANT_BUFFER );
        if( BGS_FAILED( m_pDevice->CreateBuffer( bufferDesc, &m_pConstantBuffer ) ) )
        {
            Destroy();
            return Results::FAIL;
        }

        return Results::OK;
    }

    void Renderer::Destroy()
    {
        if( m_pColorRT != nullptr )
        {
            m_pDevice->DestroyRenderTarget( &m_pColorRT );
        }
        if( m_pDepthRT != nullptr )
        {
            m_pDevice->DestroyRenderTarget( &m_pDepthRT );
        }
        if( m_pVertexBuffer != nullptr )
        {
            m_pDevice->DestroyBuffer( &m_pVertexBuffer );
        }
        if( m_pIndexBuffer != nullptr )
        {
            m_pDevice->DestroyBuffer( &m_pIndexBuffer );
        }
        if( m_pConstantBuffer != nullptr )
        {
            m_pDevice->DestroyBuffer( &m_pConstantBuffer );
        }
        if( m_pStorageBuffer != nullptr )
        {
            m_pDevice->DestroyBuffer( &m_pStorageBuffer );
        }
    }

} // namespace BIGOS