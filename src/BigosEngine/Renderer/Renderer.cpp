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

        const String hlslShader = R"(
            // Sta³y bufor (cbuffer) - przekazuje macierze do transformacji
            cbuffer MatrixBuffer : register(b0) {
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
            SamplerState texSampler : register(s0);
            Texture2D texture0 : register(t0);
            Texture2D texture1 : register(t1);
            RWTexture2D<float4> storageTexture : register(u0);
            RWByteAddressBuffer rawStorageBuffer : register(u1);

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
        shaderDesc.source.sourceSize  = hlslShader.size();
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
        if( m_pVS != nullptr )
        {
            m_pDevice->DestroyShader( &m_pVS );
        }
        if( m_pPS != nullptr )
        {
            m_pDevice->DestroyShader( &m_pPS );
        }
    }

} // namespace BIGOS