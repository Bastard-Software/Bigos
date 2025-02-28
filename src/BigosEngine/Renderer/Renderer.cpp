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

        const Driver::Backend::FORMAT rtFormat = Driver::Backend::Formats::R8G8B8A8_UNORM;
        const Driver::Backend::FORMAT dsFormat = Driver::Backend::Formats::D32_FLOAT;

        Driver::Frontend::RenderTargetDesc rtDesc;
        rtDesc.format        = rtFormat;
        rtDesc.allowSampling = BGS_TRUE;
        rtDesc.allowCopying  = BGS_TRUE;
        rtDesc.width         = 1280;
        rtDesc.height        = 720;
        rtDesc.sampleCount   = Driver::Backend::SampleCount::COUNT_2;
        if( BGS_FAILED( m_pDevice->CreateRenderTarget( rtDesc, &m_pColorRT ) ) )
        {
            return Results::FAIL;
        }

        rtDesc.format       = dsFormat;
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

        const String hlslShader = R"(
            // Sta³y bufor (cbuffer) - przekazuje macierze do transformacji
            cbuffer MatrixBuffer : register(b1, space0) {
                matrix modelMatrix;
                matrix viewMatrix;
                matrix projectionMatrix;
                float4 customColor;  // Dodatkowy parametr np. dla efektów
            }

            // Struktura wejœciowa (vertex input)
            struct VSInput {
                float3 position  : POSITION;
                float3 normal    : NORMAL;
                float2 texcoord0 : TEXCOORD0;
                float2 texcoord1 : TEXCOORD1;
            };

            // Struktura wyjœciowa (vertex output)
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
                // Pobranie kolorów z dwóch tekstur
                float4 color0 = texture0.Sample(texSampler, input.texcoord0);
                float4 color1 = texture1.Sample(texSampler, input.texcoord1);

                // Mieszanie tekstur
                float4 finalColor = lerp(color0, color1, 0.5);

                // Zapisywanie do storage texture
                storageTexture[input.texcoord0 * 512] = finalColor; // 512 to przyk³adowa wielkoœæ tekstury

                // Odczyt z raw storage buffer (np. liczba punktów œwietlnych)
                uint data = rawStorageBuffer.Load(0); // Odczyt wartoœci z bufora
                finalColor.rgb *= (data / 255.0); // Skalowanie koloru

                return finalColor * customColor;
            })";

        Driver::Frontend::ShaderDesc shaderDesc;
        shaderDesc.type               = Driver::Backend::ShaderTypes::VERTEX;
        shaderDesc.source.pSourceCode = hlslShader.c_str();
        shaderDesc.source.sourceSize  = static_cast<uint32_t>( hlslShader.size() );
        if( BGS_FAILED( m_pDevice->CreateShader( shaderDesc, &m_pVS ) ) )
        {
            Destroy();
            return Results::FAIL;
        }
        shaderDesc.type = Driver::Backend::ShaderTypes::PIXEL;
        if( BGS_FAILED( m_pDevice->CreateShader( shaderDesc, &m_pPS ) ) )
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
        graphicsPipelineDesc.pRenderTargetFormats               = &rtFormat;
        graphicsPipelineDesc.depthStencilFormat                 = dsFormat;
        if( BGS_FAILED( m_pDevice->CreatePipeline( graphicsPipelineDesc, &m_pGraphicsPipeline ) ) )
        {
            Destroy();
            return Results::FAIL;
        }

        const String compHlslShader = R"(
            // Sta³y bufor (CBV)
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

                // Poprawiony sampling – Compute Shader wymaga `SampleLevel()`
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
        if( BGS_FAILED( m_pDevice->CreateShader( shaderDesc, &m_pCS ) ) )
        {
            Destroy();
            return Results::FAIL;
        }

        Driver::Frontend::ComputePipelineDesc compPipelineDesc;
        compPipelineDesc.pComputeShader = m_pCS;
        if( BGS_FAILED( m_pDevice->CreatePipeline( compPipelineDesc, &m_pComputePipeline ) ) )
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
        if( BGS_FAILED( m_pDevice->CreateTexture( texDesc, &m_pStorageTexture ) ) )
        {
            Destroy();
            return Results::FAIL;
        }
        texDesc.size            = { 2048, 2048, 1 };
        texDesc.format          = Driver::Backend::Formats::R8G8B8A8_UNORM;
        texDesc.arrayLayerCount = 6;
        texDesc.type            = Driver::Frontend::TextureTypes::TEXTURE_CUBE;
        texDesc.usage           = BGS_FLAG( Driver::Frontend::TextureUsageFlagBits::SAMPLED );
        if( BGS_FAILED( m_pDevice->CreateTexture( texDesc, &m_pSkyTexture ) ) )
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
        if( BGS_FAILED( m_pDevice->CreateTexture( texDesc, &m_pSampledTexture ) ) )
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
        if( m_pComputePipeline != nullptr )
        {
            m_pDevice->DestroyPipeline( &m_pComputePipeline );
        }
        if( m_pGraphicsPipeline != nullptr )
        {
            m_pDevice->DestroyPipeline( &m_pGraphicsPipeline );
        }
        if( m_pVS != nullptr )
        {
            m_pDevice->DestroyShader( &m_pVS );
        }
        if( m_pPS != nullptr )
        {
            m_pDevice->DestroyShader( &m_pPS );
        }
        if( m_pCS != nullptr )
        {
            m_pDevice->DestroyShader( &m_pCS );
        }
        if( m_pSampledTexture != nullptr )
        {
            m_pDevice->DestroyTexture( &m_pSampledTexture );
        }
        if( m_pStorageTexture != nullptr )
        {
            m_pDevice->DestroyTexture( &m_pStorageTexture );
        }
        if( m_pSkyTexture != nullptr )
        {
            m_pDevice->DestroyTexture( &m_pSkyTexture );
        }
    }

} // namespace BIGOS