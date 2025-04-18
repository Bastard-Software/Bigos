#include "BIGOS/Renderer/Renderer.h"

#include "Driver/Frontend/Context.h"
#include "Driver/Frontend/RenderSystem.h"
#include "Driver/Frontend/SyncSystem.h"
#include "Driver/Frontend/Swapchain.h"
#include "Platform/Window/Window.h"

namespace BIGOS
{
    Renderer::Renderer()
        : m_pRenderSystem( nullptr )
        , m_pGraphicsContext( nullptr )
        , m_pWindow( nullptr )
        , m_pSwapchain( nullptr )
        , m_frameCount( 3 )
        , m_width( 1280 )
        , m_height( 720 )
        , m_rtFormat( Driver::Backend::Formats::R8G8B8A8_UNORM )
        , m_dsFormat( Driver::Backend::Formats::D32_FLOAT )
        , m_frameNdx( 0 )
        , m_pColorRTs()
        , m_pDepthRT( nullptr )
        , m_pVertexBuffer( nullptr )
        , m_pIndexBuffer( nullptr )
        , m_pConstantBuffer( nullptr )
        , m_pStorageBuffer( nullptr )
        , m_pVS( nullptr )
        , m_pPS( nullptr )
        , m_pGraphicsPipeline( nullptr )
        , m_pCS( nullptr )
        , m_pComputePipeline( nullptr )
        , m_pSampledTexture( nullptr )
        , m_pStorageTexture( nullptr )
        , m_pSkyTexture( nullptr )
    {
    }

    void Renderer::Render()
    {
    }

    RESULT Renderer::Create( Driver::Frontend::RenderSystem* pSystem, Platform::Window* pWindow )
    {
        BGS_ASSERT( pSystem != nullptr );
        m_pRenderSystem = pSystem;
        m_pWindow       = pWindow;

        m_pGraphicsContext = m_pRenderSystem->GetGraphicsContext();

        if( m_pWindow != nullptr )
        {
            m_width  = m_pWindow->GetDesc().width;
            m_height = m_pWindow->GetDesc().height;

            Driver::Frontend::SwapchainDesc swapchainDesc;
            swapchainDesc.pWindow         = m_pWindow;
            swapchainDesc.backBufferCount = m_frameCount;
            swapchainDesc.format          = m_rtFormat;
            if( BGS_FAILED( m_pRenderSystem->CreateSwapchain( swapchainDesc, &m_pSwapchain ) ) )
            {
                Destroy();
                return Results::FAIL;
            }
        }
        m_pColorRTs.resize( m_frameCount );
        if( BGS_FAILED( CreateResizableResources() ) )
        {
            Destroy();
            return Results::FAIL;
        }

        Driver::Frontend::BufferDesc bufferDesc;
        bufferDesc.size  = 65536;
        bufferDesc.usage = BGS_FLAG( Driver::Backend::ResourceUsageFlagBits::VERTEX_BUFFER );
        if( BGS_FAILED( m_pRenderSystem->CreateBuffer( bufferDesc, &m_pVertexBuffer ) ) )
        {
            Destroy();
            return Results::FAIL;
        }
        bufferDesc.usage = BGS_FLAG( Driver::Backend::ResourceUsageFlagBits::INDEX_BUFFER );
        if( BGS_FAILED( m_pRenderSystem->CreateBuffer( bufferDesc, &m_pIndexBuffer ) ) )
        {
            Destroy();
            return Results::FAIL;
        }
        bufferDesc.usage = BGS_FLAG( Driver::Backend::ResourceUsageFlagBits::READ_WRITE_STORAGE_BUFFER );
        if( BGS_FAILED( m_pRenderSystem->CreateBuffer( bufferDesc, &m_pStorageBuffer ) ) )
        {
            Destroy();
            return Results::FAIL;
        }
        bufferDesc.size  = 256;
        bufferDesc.usage = BGS_FLAG( Driver::Backend::ResourceUsageFlagBits::CONSTANT_BUFFER );
        if( BGS_FAILED( m_pRenderSystem->CreateBuffer( bufferDesc, &m_pConstantBuffer ) ) )
        {
            Destroy();
            return Results::FAIL;
        }

        const String hlslShader = R"(
            // Sta�y bufor (cbuffer) - przekazuje macierze do transformacji
            cbuffer MatrixBuffer : register(b1, space0) {
                matrix modelMatrix;
                matrix viewMatrix;
                matrix projectionMatrix;
                float4 customColor;  // Dodatkowy parametr np. dla efekt�w
            }

            // Struktura wej�ciowa (vertex input)
            struct VSInput {
                float3 position  : POSITION;
                float3 normal    : NORMAL;
                float2 texcoord0 : TEXCOORD0;
                float2 texcoord1 : TEXCOORD1;
            };

            // Struktura wyj�ciowa (vertex output)
            struct VSOutput {
                float4 position  : SV_Position;
                float3 normal    : NORMAL;
                float2 texcoord0 : TEXCOORD0;
                float2 texcoord1 : TEXCOORD1;
            };

            // Vertex Shader
            VSOutput VSMain(VSInput input) {
                VSOutput output;

                float4 worldPosition = mul(float4(input.position, 1.0), modelMatrix);
                float4 viewPosition = mul(worldPosition, viewMatrix);
                output.position = mul(viewPosition, projectionMatrix);

                output.normal = normalize(mul(input.normal, (float3x3)modelMatrix));
                output.texcoord0 = input.texcoord0;
                output.texcoord1 = input.texcoord1;

                return output;
            }

            // Sampler i tekstury
            SamplerState texSampler : register(s0, space1);
            Texture2D texture0 : register(t2, space0);
            Texture2D texture1 : register(t3, space0);
            RWTexture2D<float4> storageTexture : register(u4, space0);
            RWByteAddressBuffer rawStorageBuffer : register(u5, space0);

            // Pixel Shader
            float4 PSMain(VSOutput input) : SV_Target {
                // Pobranie kolor�w z dw�ch tekstur
                float4 color0 = texture0.Sample(texSampler, input.texcoord0);
                float4 color1 = texture1.Sample(texSampler, input.texcoord1);

                // Mieszanie tekstur
                float4 finalColor = lerp(color0, color1, 0.5);

                // Zapisywanie do storage texture
                storageTexture[input.texcoord0 * 512] = finalColor; // 512 to przyk�adowa wielko�� tekstury

                // Odczyt z raw storage buffer (np. liczba punkt�w �wietlnych)
                uint data = rawStorageBuffer.Load(0); // Odczyt warto�ci z bufora
                finalColor.rgb *= (data / 255.0); // Skalowanie koloru

                return finalColor * customColor;
            })";

        Driver::Frontend::ShaderDesc shaderDesc;
        shaderDesc.type               = Driver::Backend::ShaderTypes::VERTEX;
        shaderDesc.source.pSourceCode = hlslShader.c_str();
        shaderDesc.source.sourceSize  = static_cast<uint32_t>( hlslShader.size() );
        if( BGS_FAILED( m_pRenderSystem->CreateShader( shaderDesc, &m_pVS ) ) )
        {
            Destroy();
            return Results::FAIL;
        }
        shaderDesc.type = Driver::Backend::ShaderTypes::PIXEL;
        if( BGS_FAILED( m_pRenderSystem->CreateShader( shaderDesc, &m_pPS ) ) )
        {
            Destroy();
            return Results::FAIL;
        }

        Driver::Frontend::GraphicsPipelineDesc graphicsPipelineDesc;
        graphicsPipelineDesc.pVertexShader                      = m_pVS;
        graphicsPipelineDesc.pPixelShader                       = m_pPS;
        graphicsPipelineDesc.pHullShader                        = nullptr;
        graphicsPipelineDesc.pDomainShader                      = nullptr;
        graphicsPipelineDesc.pGeometryShader                    = nullptr;
        graphicsPipelineDesc.blendState.blendEnable             = BGS_TRUE;
        graphicsPipelineDesc.depthStencilState.depthTestEnable  = BGS_TRUE;
        graphicsPipelineDesc.depthStencilState.depthWriteEnable = BGS_TRUE;
        graphicsPipelineDesc.sampleCount                        = Driver::Backend::SampleCount::COUNT_1;
        graphicsPipelineDesc.renderTargetCount                  = 1;
        graphicsPipelineDesc.pRenderTargetFormats               = &m_rtFormat;
        graphicsPipelineDesc.depthStencilFormat                 = m_dsFormat;
        if( BGS_FAILED( m_pRenderSystem->CreatePipeline( graphicsPipelineDesc, &m_pGraphicsPipeline ) ) )
        {
            Destroy();
            return Results::FAIL;
        }

        const String compHlslShader = R"(
            // Sta�y bufor (CBV)
            cbuffer ConstantBuffer : register(b0, space0) {
                float4 someConstants;
            };

            // Tekstura jako SRV
            Texture2D<float4> InputTexture : register(t1, space0);

            // Tekstura z zapisem (UAV)
            RWTexture2D<float4> OutputTexture : register(u2, space0);

            // Sampler
            SamplerState mySampler : register(s0, space1);

            // Compute Shader
            [numthreads(8, 8, 1)]
            void CSMain(uint3 threadID : SV_DispatchThreadID)
            {
                // Odczyt z constant buffera
                float4 constData = someConstants;

                // Poprawiony sampling � Compute Shader wymaga `SampleLevel()`
                float2 uv = float2(threadID.xy) / float2(256, 256);
                float4 texValue = InputTexture.SampleLevel(mySampler, uv, 0.0);

                // Prosta operacja
                float4 result = constData + texValue;

                // Zapis do tekstury (UAV)
                OutputTexture[threadID.xy] = result;
            })";

        shaderDesc.type               = Driver::Backend::ShaderTypes::COMPUTE;
        shaderDesc.source.pSourceCode = compHlslShader.c_str();
        shaderDesc.source.sourceSize  = static_cast<uint32_t>( compHlslShader.size() );
        if( BGS_FAILED( m_pRenderSystem->CreateShader( shaderDesc, &m_pCS ) ) )
        {
            Destroy();
            return Results::FAIL;
        }

        Driver::Frontend::ComputePipelineDesc compPipelineDesc;
        compPipelineDesc.pComputeShader = m_pCS;
        if( BGS_FAILED( m_pRenderSystem->CreatePipeline( compPipelineDesc, &m_pComputePipeline ) ) )
        {
            Destroy();
            return Results::FAIL;
        }

        Driver::Frontend::TextureDesc texDesc;
        texDesc.size            = { 16384, 1, 1 };
        texDesc.format          = Driver::Backend::Formats::R32_UINT;
        texDesc.mipLevelCount   = 1;
        texDesc.arrayLayerCount = 1;
        texDesc.sampleCount     = Driver::Backend::SampleCount::COUNT_1;
        texDesc.type            = Driver::Frontend::TextureTypes::TEXTURE_1D;
        texDesc.usage = BGS_FLAG( Driver::Frontend::TextureUsageFlagBits::STORAGE ) | BGS_FLAG( Driver::Frontend::TextureUsageFlagBits::SAMPLED );
        if( BGS_FAILED( m_pRenderSystem->CreateTexture( texDesc, &m_pStorageTexture ) ) )
        {
            Destroy();
            return Results::FAIL;
        }
        texDesc.size            = { 2048, 2048, 1 };
        texDesc.format          = Driver::Backend::Formats::R8G8B8A8_UNORM;
        texDesc.arrayLayerCount = 6;
        texDesc.type            = Driver::Frontend::TextureTypes::TEXTURE_CUBE;
        texDesc.usage           = BGS_FLAG( Driver::Frontend::TextureUsageFlagBits::SAMPLED );
        if( BGS_FAILED( m_pRenderSystem->CreateTexture( texDesc, &m_pSkyTexture ) ) )
        {
            Destroy();
            return Results::FAIL;
        }
        texDesc.size            = { 1024, 1024, 1 };
        texDesc.format          = Driver::Backend::Formats::R32G32B32A32_FLOAT;
        texDesc.mipLevelCount   = 4;
        texDesc.arrayLayerCount = 16;
        texDesc.sampleCount     = Driver::Backend::SampleCount::COUNT_1;
        texDesc.type            = Driver::Frontend::TextureTypes::TEXTURE_2D;
        if( BGS_FAILED( m_pRenderSystem->CreateTexture( texDesc, &m_pSampledTexture ) ) )
        {
            Destroy();
            return Results::FAIL;
        }

        Driver::Frontend::SyncSystem* pSyncSystem = m_pRenderSystem->GetSyncSystem();

        Driver::Frontend::SyncPoint grapchicsSnc0 = pSyncSystem->CreateSyncPoint( Driver::Frontend::ContextTypes::GRAPHICS, "graphics_sync_0" );
        Driver::Frontend::SyncPoint grapchicsSnc1 = pSyncSystem->CreateSyncPoint( Driver::Frontend::ContextTypes::GRAPHICS, "graphics_sync_1" );
        Driver::Frontend::SyncPoint computeSnc   = pSyncSystem->CreateSyncPoint( Driver::Frontend::ContextTypes::COMPUTE, "compute_sync" );
        Driver::Frontend::SyncPoint copySnc      = pSyncSystem->CreateSyncPoint( Driver::Frontend::ContextTypes::COPY, "copy_sync" );

        RESULT syncRes = Results::FAIL;
        syncRes = pSyncSystem->Signal( grapchicsSnc0 );
        syncRes = pSyncSystem->Signal( grapchicsSnc1 );
        syncRes = pSyncSystem->Signal( computeSnc );
        syncRes = pSyncSystem->Signal( copySnc );

        syncRes = pSyncSystem->Wait( grapchicsSnc0 );
        syncRes = pSyncSystem->Wait( { grapchicsSnc0, computeSnc, copySnc } );

        return Results::OK;
    }

    void Renderer::Destroy()
    {
        if( m_pSwapchain != nullptr )
        {
            m_pRenderSystem->DestroySwapchain( &m_pSwapchain );
        }

        DestroyResizableResources();

        if( m_pVertexBuffer != nullptr )
        {
            m_pRenderSystem->DestroyBuffer( &m_pVertexBuffer );
        }
        if( m_pIndexBuffer != nullptr )
        {
            m_pRenderSystem->DestroyBuffer( &m_pIndexBuffer );
        }
        if( m_pConstantBuffer != nullptr )
        {
            m_pRenderSystem->DestroyBuffer( &m_pConstantBuffer );
        }
        if( m_pStorageBuffer != nullptr )
        {
            m_pRenderSystem->DestroyBuffer( &m_pStorageBuffer );
        }
        if( m_pComputePipeline != nullptr )
        {
            m_pRenderSystem->DestroyPipeline( &m_pComputePipeline );
        }
        if( m_pGraphicsPipeline != nullptr )
        {
            m_pRenderSystem->DestroyPipeline( &m_pGraphicsPipeline );
        }
        if( m_pVS != nullptr )
        {
            m_pRenderSystem->DestroyShader( &m_pVS );
        }
        if( m_pPS != nullptr )
        {
            m_pRenderSystem->DestroyShader( &m_pPS );
        }
        if( m_pCS != nullptr )
        {
            m_pRenderSystem->DestroyShader( &m_pCS );
        }
        if( m_pSampledTexture != nullptr )
        {
            m_pRenderSystem->DestroyTexture( &m_pSampledTexture );
        }
        if( m_pStorageTexture != nullptr )
        {
            m_pRenderSystem->DestroyTexture( &m_pStorageTexture );
        }
        if( m_pSkyTexture != nullptr )
        {
            m_pRenderSystem->DestroyTexture( &m_pSkyTexture );
        }
    }

    RESULT Renderer::Resize( uint32_t width, uint32_t height )
    {
        m_width  = width;
        m_height = height;

        if( BGS_FAILED( m_pSwapchain->Resize( m_pWindow ) ) )
        {
            return Results::FAIL;
        }
        DestroyResizableResources();
        return CreateResizableResources();
    }

    RESULT Renderer::CreateResizableResources()
    {
        Driver::Frontend::RenderTargetDesc rtDesc;
        rtDesc.format        = m_rtFormat;
        rtDesc.allowSampling = BGS_FALSE;
        rtDesc.allowCopying  = BGS_TRUE;
        rtDesc.width         = m_width;
        rtDesc.height        = m_height;
        rtDesc.sampleCount   = Driver::Backend::SampleCount::COUNT_1;
        for( index_t ndx = 0; ndx < static_cast<index_t>( m_frameCount ); ++ndx )
        {
            if( BGS_FAILED( m_pRenderSystem->CreateRenderTarget( rtDesc, &m_pColorRTs[ ndx ] ) ) )
            {
                Destroy();
                return Results::FAIL;
            }
        }

        rtDesc.format       = m_dsFormat;
        rtDesc.allowCopying = BGS_FALSE;
        rtDesc.sampleCount  = Driver::Backend::SampleCount::COUNT_1;
        if( BGS_FAILED( m_pRenderSystem->CreateRenderTarget( rtDesc, &m_pDepthRT ) ) )
        {
            Destroy();
            return Results::FAIL;
        }

        return Results::OK;
    }

    void Renderer::DestroyResizableResources()
    {
        for( index_t ndx = 0; ndx < static_cast<index_t>( m_frameCount ); ++ndx )
        {
            if( m_pColorRTs[ ndx ] != nullptr )
            {
                m_pRenderSystem->DestroyRenderTarget( &m_pColorRTs[ ndx ] );
                m_pColorRTs[ ndx ] = nullptr;
            }
        }
        if( m_pDepthRT != nullptr )
        {
            m_pRenderSystem->DestroyRenderTarget( &m_pDepthRT );
            m_pDepthRT = nullptr;
        }
    }

} // namespace BIGOS