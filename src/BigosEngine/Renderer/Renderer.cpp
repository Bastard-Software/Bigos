#include "BigosEngine/Renderer/Renderer.h"

#include "Driver/Frontend/RenderDevice.h"
#include "Driver/Frontend/RenderSystem.h"

namespace BIGOS
{
    Renderer::Renderer()
        : m_pRenderSystem(nullptr)
        , m_pDevice(nullptr)
        , m_pColorRT(nullptr)
        , m_pDepthRT(nullptr)
    {
    }

    RESULT Renderer::Create( Driver::Frontend::RenderSystem* pSystem )
    {
        BGS_ASSERT( pSystem != nullptr );
        m_pRenderSystem = pSystem;

        Driver::Frontend::RenderDeviceDesc devDesc;
        devDesc.adapter.index = 0;
        if (BGS_FAILED(m_pRenderSystem->CreateDevice(devDesc, &m_pDevice)))
        {
            return Results::FAIL;
        }

        Driver::Frontend::RenderTargetDesc rtDesc;
        rtDesc.format = Driver::Backend::Formats::R8G8B8A8_UNORM;
        rtDesc.allowSampling = BGS_TRUE;
        rtDesc.allowCopying  = BGS_TRUE;
        rtDesc.width         = 1280;
        rtDesc.height        = 720;
        rtDesc.sampleCount   = Driver::Backend::SampleCount::COUNT_2;
        if( BGS_FAILED( m_pDevice->CreateRenderTarget(rtDesc, &m_pColorRT ) ) )
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

        return Results::OK;
    }

    void Renderer::Destroy()
    {
        if (m_pColorRT != nullptr)
        {
            m_pDevice->DestroyRenderTarget( &m_pColorRT );
        }
        if( m_pDepthRT != nullptr )
        {
            m_pDevice->DestroyRenderTarget( &m_pDepthRT );
        }
    }

} // namespace BIGOS