#include "Driver/Frontend/Shader/Shader.h"

#include "Core/CoreTypes.h"

#include "Driver/Frontend/RenderDevice.h"
#include "Driver/Frontend/RenderSystem.h"
#include "Driver/Frontend/Shader/IShaderCompiler.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {
            static const char* GetShaderMainFromType( Backend::SHADER_TYPE type )
            {
                static const char* translateTable[ BGS_ENUM_COUNT( Backend::ShaderTypes ) ] = {
                    "VSMain", // VERTEX_SHADER
                    "PSMain", // PIXEL_SHADER
                    "GSMain", // GEOMETRY_SHADER
                    "HSMain", // HULL_SHADER
                    "DSMain", // DOMAIN_SHADER
                    "CSMain", // COMPUTE_SHADER
                };

                return translateTable[ BGS_ENUM_INDEX( type ) ];
            }

            Shader::Shader()
                : m_compileDesc()
                , m_hShader()
                , m_pParent( nullptr )
                , m_pCompiledShader( nullptr )
                , m_pEntryPoint( nullptr )
                , m_type( Backend::ShaderTypes::_MAX_ENUM )
            {
            }

            RESULT Shader::Create( const ShaderDesc& desc, RenderDevice* pDevice )
            {
                BGS_ASSERT( pDevice != nullptr, "Render device (pDevice) must be a valid pointer." );
                m_pParent                      = pDevice;
                m_type                         = desc.type;
                Backend::IDevice*   pAPIDevice = m_pParent->GetNativeAPIDevice();
                IShaderCompiler*    pCompiler  = m_pParent->GetParent()->GetDefaultCompiler();
                const SHADER_FORMAT outputFormat =
                    m_pParent->GetParent()->GetDesc().factoryDesc.apiType == Backend::APITypes::D3D12 ? ShaderFormats::DXIL : ShaderFormats::SPIRV;
                m_pEntryPoint = GetShaderMainFromType( m_type );

                m_compileDesc.type               = m_type;
                m_compileDesc.argCount           = 0;
                m_compileDesc.ppArgs             = nullptr;
                m_compileDesc.outputFormat       = outputFormat;
                m_compileDesc.model              = ShaderModels::SHADER_MODEL_6_5;
                m_compileDesc.reflect            = BGS_TRUE;
                m_compileDesc.compileDebug       = BGS_TRUE;
                m_compileDesc.pEntryPoint        = m_pEntryPoint;
                m_compileDesc.source.pSourceCode = desc.source.pSourceCode;
                m_compileDesc.source.sourceSize  = desc.source.sourceSize;
                if( BGS_FAILED( pCompiler->Compile( m_compileDesc, &m_pCompiledShader ) ) )
                {
                    return Results::FAIL;
                }

                Backend::ShaderDesc shaderDesc;
                shaderDesc.pByteCode = m_pCompiledShader->pByteCode;
                shaderDesc.codeSize  = m_pCompiledShader->byteCodeSize;
                if( BGS_FAILED( pAPIDevice->CreateShader( shaderDesc, &m_hShader ) ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void Shader::Destroy()
            {
                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                if( m_pCompiledShader != nullptr )
                {
                    m_pParent->GetParent()->GetDefaultCompiler()->DestroyOutput( &m_pCompiledShader );
                }
                if( m_hShader != Backend::ShaderHandle() )
                {
                    pAPIDevice->DestroyShader( &m_hShader );
                }
            }

            SHADER_LANGUAGE Shader::DetectShaderLanguage( const String& shaderCode )
            {
                if( std::regex_search(
                        shaderCode,
                        std::regex(
                            R"(\b(SV_Position|SV_Target|cbuffer|RWTexture2D|StructuredBuffer|SV_DispatchThreadID|numthreads|TriangleStream|LineStream|PointStream|patch|OutputPatch|InputPatch|SV_DomainLocation)\b)" ) ) )
                {
                    return ShaderLanguages::HLSL;
                }

                if( std::regex_search(
                        shaderCode,
                        std::regex(
                            R"(\b(layout\(location\)|in\s|out\s|uniform\s|vec[234]\s|sampler2D|layout\(local_size_x\)|gl_GlobalInvocationID|layout\(vertices\)|gl_TessLevelInner|gl_TessCoord|layout\(points|lines|triangles\) in|EmitVertex|EndPrimitive)\b)" ) ) )
                {
                    return ShaderLanguages::GLSL;
                }

                return ShaderLanguages::_MAX_ENUM;
            }

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS