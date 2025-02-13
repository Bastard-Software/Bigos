#include "Dxc.h"

#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Memory.h"
#include "Core/Utils/String.h"
#include "Driver/Frontend/RenderSystem.h"
#include "Platform/Platform.h"
#include "ShaderCompilerFactory.h"

#if( BGS_VULKAN_API )
#    include "ThirdParty/SPIRV-Cross/spirv_cross.hpp"
#    include "ThirdParty/SPIRV-Cross/spirv_parser.hpp"
#endif

#if( BGS_D3D12_API )
#    include <d3d12shader.h>
#endif

#define RELEASE_COM_PTR( _x )                                                                                                                        \
    {                                                                                                                                                \
        if( ( _x ) != nullptr )                                                                                                                      \
        {                                                                                                                                            \
            ( _x )->Release();                                                                                                                       \
            ( _x ) = nullptr;                                                                                                                        \
        }                                                                                                                                            \
    }

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {
            static const wchar_t* MapBigosShaderModelToWString( SHADER_MODEL model )
            {
                static const wchar_t* translateTable[ BGS_ENUM_COUNT( ShaderModels ) ] = {
                    L"_6_0", // SHADER_MODEL_6_0
                    L"_6_1", // SHADER_MODEL_6_1
                    L"_6_2", // SHADER_MODEL_6_2
                    L"_6_3", // SHADER_MODEL_6_3
                    L"_6_4", // SHADER_MODEL_6_4
                    L"_6_5", // SHADER_MODEL_6_5
                    L"_6_6", // SHADER_MODEL_6_6
                    L"_6_7", // SHADER_MODEL_6_7
                    L"_6_8", // SHADER_MODEL_6_7
                };

                return translateTable[ BGS_ENUM_INDEX( model ) ];
            }

            static const wchar_t* MapBigosShaderTypeToWString( Backend::SHADER_TYPE type )
            {
                static const wchar_t* translateTable[ BGS_ENUM_COUNT( Backend::ShaderTypes ) ] = {
                    L"vs", // VERTEX_SHADER
                    L"ps", // PIXEL_SHADER
                    L"gs", // GEOMETRY_SHADER
                    L"hs", // HULL_SHADER
                    L"ds", // DOMAIN_SHADER
                    L"cs", // COMPUTE_SHADER
                };

                return translateTable[ BGS_ENUM_INDEX( type ) ];
            }

            static const Backend::BINDING_TYPE MapDXILInputBindingDescToBigosBindingType( const D3D12_SHADER_INPUT_BIND_DESC bindingDesc )
            {
                static const Backend::BINDING_TYPE translateTable[] = {
                    Backend::BindingTypes::CONSTANT_BUFFER,           // D3D_SIT_CBUFFER
                    Backend::BindingTypes::CONSTANT_TEXEL_BUFFER,     // D3D_SIT_TBUFFER
                    Backend::BindingTypes::SAMPLED_TEXTURE,           // D3D_SIT_TEXTURE
                    Backend::BindingTypes::SAMPLER,                   // D3D_SIT_SAMPLER
                    Backend::BindingTypes::_MAX_ENUM,                 // D3D_SIT_UAV_RWTYPED
                    Backend::BindingTypes::_MAX_ENUM,                 // D3D_SIT_STRUCTURED
                    Backend::BindingTypes::_MAX_ENUM,                 // D3D_SIT_UAV_RWSTRUCTURED
                    Backend::BindingTypes::_MAX_ENUM,                 // D3D_SIT_BYTEADDRESS
                    Backend::BindingTypes::READ_WRITE_STORAGE_BUFFER, // D3D_SIT_UAV_RWBYTEADDRESS
                    Backend::BindingTypes::_MAX_ENUM,                 // D3D_SIT_UAV_APPEND_STRUCTURED
                    Backend::BindingTypes::_MAX_ENUM,                 // D3D_SIT_UAV_CONSUME_STRUCTURED
                    Backend::BindingTypes::_MAX_ENUM,                 // D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER
                    Backend::BindingTypes::_MAX_ENUM,                 // D3D_SIT_RTACCELERATIONSTRUCTURE
                    Backend::BindingTypes::_MAX_ENUM,                 // D3D_SIT_UAV_FEEDBACKTEXTURE
                };

                Backend::BINDING_TYPE type;
                if( ( bindingDesc.Type == D3D_SIT_TEXTURE ) && ( bindingDesc.Dimension == D3D_SRV_DIMENSION_BUFFER ) )
                {
                    type = Backend::BindingTypes::READ_ONLY_STORAGE_BUFFER;
                }
                else if( bindingDesc.Type == D3D_SIT_UAV_RWTYPED )
                {
                    if( bindingDesc.Dimension == D3D_SRV_DIMENSION_BUFFER )
                    {
                        type = Backend::BindingTypes::STORAGE_TEXEL_BUFFER;
                    }
                    else
                    {
                        type = Backend::BindingTypes::STORAGE_TEXTURE;
                    }
                }
                else
                {
                    type = translateTable[ BGS_ENUM_INDEX( bindingDesc.Type ) ];
                }
                BGS_ASSERT( type != Backend::BindingTypes::_MAX_ENUM );

                return type;
            }

            static const SHADER_INPUT_BASE_TYPE MapSPIRVTypeToBigosShaderInputBaseDataType( spirv_cross::SPIRType type )
            {
                if( type.basetype == spirv_cross::SPIRType::UInt )
                {
                    return ShaderInputBaseTypes::UINT;
                }
                else if( type.basetype == spirv_cross::SPIRType::Int )
                {
                    return ShaderInputBaseTypes::INT;
                }
                else if( type.basetype == spirv_cross::SPIRType::Float )
                {
                    return ShaderInputBaseTypes::FLOAT;
                }

                return ShaderInputBaseTypes::UNKNOWN;
            }

            static const SHADER_INPUT_BASE_TYPE MapDXILTypeToBigosShaderInputBaseDataType( D3D_REGISTER_COMPONENT_TYPE type )
            {
                static SHADER_INPUT_BASE_TYPE translateTable[] = {
                    ShaderInputBaseTypes::UNKNOWN, // D3D_REGISTER_COMPONENT_UNKNOWN
                    ShaderInputBaseTypes::UINT,    // D3D_REGISTER_COMPONENT_UINT32
                    ShaderInputBaseTypes ::INT,    // D3D_REGISTER_COMPONENT_SINT32
                    ShaderInputBaseTypes ::FLOAT   // D3D_REGISTER_COMPONENT_FLOAT32
                };

                return translateTable[ BGS_ENUM_INDEX( type ) ];
            }

            static const uint32_t GetComponentCountFromDXILMask( BYTE mask )
            {
                uint32_t compCnt = 0;
                for( index_t ndx = 0; ndx < 4; ++ndx )
                {
                    if( mask & ( 1 << ndx ) )
                    {
                        compCnt++;
                    }
                }

                return compCnt;
            }

            static const String GetSemanticNameFromSPIRVName( const String& name )
            {
                const String prefix = "in.var.";
                if( name.rfind( prefix, 0 ) == 0 )
                {
                    return name.substr( prefix.length() );
                }
                return name;
            }

            DXCompiler::DXCompiler()
                : m_pParent( nullptr )
                , m_pUtils( nullptr )
                , m_pCompiler( nullptr )
                , m_libraryHandle( nullptr )
            {
            }

            RESULT DXCompiler::Compile( const CompileShaderDesc& desc, ShaderCompilerOutput** ppOutput )
            {
                BGS_ASSERT( ppOutput != nullptr, "Shader compiler output (ppOutput) must be a valid address." );
                BGS_ASSERT( *ppOutput == nullptr, "There is a pointer at the given address. Shader compiler output (*ppOutput) must be nullptr." );
                BGS_ASSERT( desc.source.pSourceCode != nullptr, "Shader source (desc.source.pSourceCode) must be a valid pointer".);
                if( ( ppOutput == nullptr ) || ( *ppOutput != nullptr ) || ( desc.source.pSourceCode == nullptr ) )
                {
                    return Results::FAIL;
                }

                uint32_t       argCnt = 0;
                const wchar_t* compArgs[ Config::Driver::Shader::MAX_SHADER_COMPILER_ARGUMENT_COUNT + 8 ]; // +8 for internally added args
                if( desc.ppArgs != nullptr )
                {
                    for( argCnt; argCnt < desc.argCount; ++argCnt )
                    {
                        compArgs[ argCnt ] = desc.ppArgs[ argCnt ];
                    }
                }

                // Handling entry point
                uint32_t entryLen = static_cast<uint32_t>( Core::Utils::String::Length( desc.pEntryPoint ) + 1 ); // +1 for string terminating char
                BGS_ASSERT( entryLen < Config::Driver::Shader::MAX_SHADER_ENTRY_POINT_NAME_LENGHT + 1 );          // +1 for string terminating char
                if( entryLen >= Config::Driver::Shader::MAX_SHADER_ENTRY_POINT_NAME_LENGHT + 1 )
                {
                    return Results::FAIL;
                }
                wchar_t  entryPoint[ Config::Driver::Shader::MAX_SHADER_ENTRY_POINT_NAME_LENGHT + 1 ]; // +1 for string terminating char
                wchar_t* pEntry = entryPoint;
                Core::Utils::String::Utf8ToUtf16( pEntry, entryLen, desc.pEntryPoint, Config::Driver::Shader::MAX_SHADER_ENTRY_POINT_NAME_LENGHT );
                compArgs[ argCnt++ ] = L"-E";
                compArgs[ argCnt++ ] = pEntry;

                // Handling target (shader model and type)
                wchar_t  target[ 32 ];
                wchar_t* pTarget = target;
                Core::Utils::String::Copy( pTarget, 2 * sizeof( wchar_t ), MapBigosShaderTypeToWString( desc.type ) );
                Core::Utils::String::Copy( pTarget + 2, 4 * sizeof( wchar_t ),
                                           MapBigosShaderModelToWString( desc.model ) ); // +2 copying to the same buffer with 2 char offset
                compArgs[ argCnt++ ] = L"-T";
                compArgs[ argCnt++ ] = pTarget;

#if( BGS_SHADER_DEBUG )
                if( desc.compileDebug == BGS_TRUE )
                {
                    compArgs[ argCnt++ ] = DXC_ARG_DEBUG;
                }
#endif

                if( desc.outputFormat == ShaderFormats::SPIRV )
                {
#if( BGS_VULKAN_API )
                    compArgs[ argCnt++ ] = L"-spirv";
#endif
                }
                else if( desc.outputFormat == ShaderFormats::DXIL )
                {
#if( BGS_D3D12_API )
                    // TODO: Add if needed
#endif
                }
                else
                {
                    BGS_ASSERT( 0 );
                    return Results::FAIL;
                }

                IDxcBlobEncoding* pSrc = nullptr;
                if( FAILED( m_pUtils->CreateBlob( desc.source.pSourceCode, desc.source.sourceSize, CP_UTF8, &pSrc ) ) )
                {
                    return Results::FAIL;
                }

                DxcBuffer srcBuff;
                srcBuff.Ptr      = pSrc->GetBufferPointer();
                srcBuff.Size     = pSrc->GetBufferSize();
                srcBuff.Encoding = 0; // Means "non character text"

                IDxcResult* pRes = nullptr;
                // TODO: Better error handling
                IDxcBlobUtf8* pErr = nullptr;
                if( FAILED( m_pCompiler->Compile( &srcBuff, compArgs, argCnt, nullptr, IID_PPV_ARGS( &pRes ) ) ) )
                {
                    if( SUCCEEDED( pRes->GetOutput( DXC_OUT_ERRORS, IID_PPV_ARGS( &pErr ), nullptr ) ) )
                    {
                        if( ( pErr != nullptr ) && ( pErr->GetStringLength() > 0 ) )
                        {
                            // TODO: Log better way
                            printf( "%s", static_cast<const char*>( pErr->GetStringPointer() ) );
                            RELEASE_COM_PTR( pErr );
                        }
                    }

                    RELEASE_COM_PTR( pRes );
                    RELEASE_COM_PTR( pSrc );

                    return Results::FAIL;
                }

                if( SUCCEEDED( pRes->GetOutput( DXC_OUT_ERRORS, IID_PPV_ARGS( &pErr ), nullptr ) ) )
                {
                    if( ( pErr != nullptr ) && ( pErr->GetStringLength() > 0 ) )
                    {
                        // TODO: Log better way
                        printf( "%s", static_cast<const char*>( pErr->GetStringPointer() ) );
                        RELEASE_COM_PTR( pErr );
                    }
                }

                IDxcBlob* pBlob = nullptr;
                pRes->GetResult( &pBlob );

                ShaderCompilerOutput* pOutput = nullptr;
                if( pBlob == nullptr )
                {
                    return Results::FAIL;
                }
                else
                {
                    ShaderBindingArray      bindings;
                    ShaderInputBindingArray inputBindings;
                    if( desc.reflect )
                    {
                        if( desc.outputFormat == ShaderFormats::DXIL )
                        {
                            bindings = GetDXILReflection( pRes );
                            if( desc.type == Backend::ShaderTypes::VERTEX )
                            {
                                inputBindings = GetDXILInputReflection( pRes );
                            }
                        }
                        else if( desc.outputFormat == ShaderFormats::SPIRV )
                        {
                            bindings = GetSPIRVReflection( pBlob->GetBufferPointer(), static_cast<uint32_t>( pBlob->GetBufferSize() ) );
                            if( desc.type == Backend::ShaderTypes::VERTEX )
                            {
                                inputBindings = GetSPIRVInputReflection( pBlob->GetBufferPointer(), static_cast<uint32_t>( pBlob->GetBufferSize() ) );
                            }
                        }
                    }

                    const uint32_t bindingsBufferSize     = static_cast<uint32_t>( bindings.size() * sizeof( ShaderBindingInfo ) );
                    const uint32_t inputBindingBufferSize = static_cast<uint32_t>( inputBindings.size() * sizeof( ShaderInputBindingInfo ) );
                    const uint32_t allocSize = static_cast<uint32_t>( pBlob->GetBufferSize() + sizeof( ShaderCompilerOutput ) + bindingsBufferSize +
                                                                      inputBindingBufferSize );
                    byte_t*        pMem      = nullptr;
                    if( BGS_FAILED( Core::Memory::AllocateBytes( m_pParent->GetParent()->GetDefaultAllocator(), &pMem, allocSize ) ) )
                    {
                        RELEASE_COM_PTR( pBlob );
                        return Results::NO_MEMORY;
                    }

                    pOutput = reinterpret_cast<ShaderCompilerOutput*>( pMem );
                    pMem += sizeof( ShaderCompilerOutput );
                    pOutput->pByteCode = pMem;
                    pMem += static_cast<uint32_t>( pBlob->GetBufferSize() );
                    pOutput->byteCodeSize = static_cast<uint32_t>( pBlob->GetBufferSize() );
                    pOutput->pBindings    = reinterpret_cast<ShaderBindingInfo*>( pMem );
                    pMem += bindingsBufferSize;
                    pOutput->bindingCount   = static_cast<uint32_t>( bindings.size() );
                    pOutput->pInputBindings = reinterpret_cast<ShaderInputBindingInfo*>( pMem );
                    pMem += inputBindingBufferSize;
                    pOutput->inputBindingCount = static_cast<uint32_t>( inputBindings.size() );

                    Core::Memory::Copy( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), pOutput->pByteCode, pOutput->byteCodeSize );
                    if( ( bindings.data() != nullptr ) && ( bindingsBufferSize > 0 ) )
                    {
                        Core::Memory::Copy( bindings.data(), bindingsBufferSize, pOutput->pBindings, bindingsBufferSize );
                    }
                    else
                    {
                        pOutput->pBindings = nullptr;
                    }
                    if( ( inputBindings.data() != nullptr ) && ( inputBindingBufferSize > 0 ) )
                    {
                        Core::Memory::Copy( inputBindings.data(), inputBindingBufferSize, pOutput->pInputBindings, inputBindingBufferSize );
                    }
                    else
                    {
                        pOutput->pInputBindings = nullptr;
                    }
                }

                RELEASE_COM_PTR( pRes );
                RELEASE_COM_PTR( pSrc );
                RELEASE_COM_PTR( pBlob );
                *ppOutput = pOutput;

                return Results::OK;
            }

            void DXCompiler::DestroyOutput( ShaderCompilerOutput** ppOutput )
            {
                BGS_ASSERT( ppOutput != nullptr, "Shader compiler output (ppOutput) must be a valid address." );
                BGS_ASSERT( *ppOutput != nullptr, "Shader compiler output (*ppOutput) must be a valid pointer." );
                if( ( ppOutput != nullptr ) && ( *ppOutput != nullptr ) )
                {
                    Core::Memory::Free( m_pParent->GetParent()->GetDefaultAllocator(), ppOutput );
                }
            }

            RESULT DXCompiler::Create( const ShaderCompilerDesc& desc, ShaderCompilerFactory* pFactory )
            {
                BGS_ASSERT( pFactory != nullptr, "Factory (*pFactory) must be a valid pointer." );
                if( pFactory == nullptr )
                {
                    return Results::FAIL;
                }
                m_desc    = desc;
                m_pParent = pFactory;

                m_libraryHandle = Platform::LoadLibrary( "dxcompiler.dll" );
                if( m_libraryHandle == nullptr )
                {
                    return Results::FAIL;
                }

                DxcCreateInstanceProc DxcCreateInstance =
                    reinterpret_cast<DxcCreateInstanceProc>( Platform::GetProcAddress( m_libraryHandle, "DxcCreateInstance" ) );
                if( DxcCreateInstance == nullptr )
                {
                    Platform::FreeLibrary( m_libraryHandle );
                    return Results::FAIL;
                }

                if( FAILED( DxcCreateInstance( CLSID_DxcUtils, IID_PPV_ARGS( &m_pUtils ) ) ) )
                {
                    Platform::FreeLibrary( m_libraryHandle );
                    return Results::FAIL;
                }

                if( FAILED( DxcCreateInstance( CLSID_DxcCompiler, IID_PPV_ARGS( &m_pCompiler ) ) ) )
                {
                    RELEASE_COM_PTR( m_pUtils );
                    Platform::FreeLibrary( m_libraryHandle );
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void DXCompiler::Destroy()
            {
                RELEASE_COM_PTR( m_pUtils );
                RELEASE_COM_PTR( m_pCompiler );
                Platform::FreeLibrary( m_libraryHandle );

                m_pUtils        = nullptr;
                m_pCompiler     = nullptr;
                m_libraryHandle = nullptr;
            }

            ShaderBindingArray DXCompiler::GetDXILReflection( IDxcResult* pRes )
            {
                ShaderBindingArray bindingArray;

                IDxcBlob* pReflectionBlob = nullptr;
                pRes->GetOutput( DXC_OUT_REFLECTION, IID_PPV_ARGS( &pReflectionBlob ), nullptr );
                BGS_ASSERT( pReflectionBlob != nullptr )
                if( pReflectionBlob == nullptr )
                {
                    return bindingArray;
                }
                DxcBuffer               reflectionBuffer = { pReflectionBlob->GetBufferPointer(), pReflectionBlob->GetBufferSize(), 0 };
                ID3D12ShaderReflection* pReflection      = nullptr;
                m_pUtils->CreateReflection( &reflectionBuffer, IID_PPV_ARGS( &pReflection ) );
                if( pReflectionBlob == nullptr )
                {
                    RELEASE_COM_PTR( pReflectionBlob );
                    return bindingArray;
                }
                D3D12_SHADER_DESC shaderDesc;
                if( FAILED( pReflection->GetDesc( &shaderDesc ) ) )
                {
                    RELEASE_COM_PTR( pReflectionBlob );
                    RELEASE_COM_PTR( pReflection );
                    return bindingArray;
                }

                for( index_t ndx = 0; ndx < shaderDesc.BoundResources; ++ndx )
                {
                    D3D12_SHADER_INPUT_BIND_DESC bindingDesc;
                    pReflection->GetResourceBindingDesc( static_cast<uint32_t>( ndx ), &bindingDesc );
                    for( index_t ndy = 0; ndy < static_cast<index_t>( bindingDesc.BindCount ); ++ndy )
                    {

                        ShaderBindingInfo binding;
                        binding.baseShaderRegister = bindingDesc.BindPoint + static_cast<uint32_t>( ndy );
                        binding.baseBindingSlot    = MAX_UINT32; // Not used in D3D12
                        binding.set                = bindingDesc.Space;
                        binding.type               = MapDXILInputBindingDescToBigosBindingType( bindingDesc );
                        Core::Utils::String::Copy( binding.pName, Core::Utils::String::Length( bindingDesc.Name ) + 1, bindingDesc.Name );

                        bindingArray.push_back( binding );
                    }
                }

                RELEASE_COM_PTR( pReflectionBlob );
                RELEASE_COM_PTR( pReflection );

                return bindingArray;
            }

            ShaderBindingArray DXCompiler::GetSPIRVReflection( void* pSrc, uint32_t srcSize )
            {
                ShaderBindingArray bindingArray;

                spirv_cross::Compiler        compiler( reinterpret_cast<uint32_t*>( pSrc ), srcSize / 4 );
                spirv_cross::ShaderResources resources = compiler.get_shader_resources();

                // Textures
                for( index_t ndx = 0; ndx < resources.separate_images.size(); ++ndx )
                {
                    ShaderBindingInfo            binding;
                    const spirv_cross::Resource& res = resources.separate_images[ ndx ];
                    binding.baseShaderRegister       = MAX_UINT32; // Not used in vulkan
                    binding.baseBindingSlot          = compiler.get_decoration( res.id, spv::DecorationBinding );
                    binding.set                      = compiler.get_decoration( res.id, spv::DecorationDescriptorSet );
                    spirv_cross::SPIRType type       = compiler.get_type( res.type_id );
                    if( type.basetype == spirv_cross::SPIRType::Image && type.image.dim == spv::DimBuffer )
                    {
                        binding.type = Backend::BindingTypes::CONSTANT_TEXEL_BUFFER;
                    }
                    else
                    {
                        binding.type = Backend::BindingTypes::SAMPLED_TEXTURE;
                    }
                    Core::Utils::String::Copy( binding.pName, res.name.length() + 1, res.name.c_str() );

                    bindingArray.push_back( binding );
                }

                // Storage texture
                for( index_t ndx = 0; ndx < resources.storage_images.size(); ++ndx )
                {
                    ShaderBindingInfo            binding;
                    const spirv_cross::Resource& res = resources.storage_images[ ndx ];
                    binding.baseShaderRegister       = MAX_UINT32; // Not used in vulkan
                    binding.baseBindingSlot          = compiler.get_decoration( res.id, spv::DecorationBinding );
                    binding.set                      = compiler.get_decoration( res.id, spv::DecorationDescriptorSet );
                    spirv_cross::SPIRType type       = compiler.get_type( res.type_id );
                    if( type.basetype == spirv_cross::SPIRType::Image && type.image.dim == spv::DimBuffer )
                    {
                        binding.type = Backend::BindingTypes::STORAGE_TEXEL_BUFFER;
                    }
                    else
                    {
                        binding.type = Backend::BindingTypes::STORAGE_TEXTURE;
                    }
                    Core::Utils::String::Copy( binding.pName, res.name.length() + 1, res.name.c_str() );

                    bindingArray.push_back( binding );
                }

                // Samplers
                for( index_t ndx = 0; ndx < resources.separate_samplers.size(); ++ndx )
                {
                    ShaderBindingInfo            binding;
                    const spirv_cross::Resource& res = resources.separate_samplers[ ndx ];
                    binding.baseShaderRegister       = MAX_UINT32; // Not used in vulkan
                    binding.baseBindingSlot          = compiler.get_decoration( res.id, spv::DecorationBinding );
                    binding.set                      = compiler.get_decoration( res.id, spv::DecorationDescriptorSet );
                    binding.type                     = Backend::BindingTypes::SAMPLER;
                    Core::Utils::String::Copy( binding.pName, res.name.length() + 1, res.name.c_str() );

                    bindingArray.push_back( binding );
                }

                // Constant buffers
                for( index_t ndx = 0; ndx < resources.uniform_buffers.size(); ++ndx )
                {
                    ShaderBindingInfo            binding;
                    const spirv_cross::Resource& res = resources.uniform_buffers[ ndx ];
                    binding.baseShaderRegister       = MAX_UINT32; // Not used in vulkan
                    binding.baseBindingSlot          = compiler.get_decoration( res.id, spv::DecorationBinding );
                    binding.set                      = compiler.get_decoration( res.id, spv::DecorationDescriptorSet );
                    binding.type                     = Backend::BindingTypes::CONSTANT_BUFFER;
                    Core::Utils::String::Copy( binding.pName, res.name.length() + 1, res.name.c_str() );

                    bindingArray.push_back( binding );
                }

                // Storage buffers
                for( index_t ndx = 0; ndx < resources.storage_buffers.size(); ++ndx )
                {
                    ShaderBindingInfo            binding;
                    const spirv_cross::Resource& res = resources.storage_buffers[ ndx ];
                    binding.baseShaderRegister       = MAX_UINT32; // Not used in vulkan
                    binding.baseBindingSlot          = compiler.get_decoration( res.id, spv::DecorationBinding );
                    binding.set                      = compiler.get_decoration( res.id, spv::DecorationDescriptorSet );
                    binding.type = Backend::BindingTypes::READ_WRITE_STORAGE_BUFFER; // TODO: Do we know if it is read only or read write?
                    Core::Utils::String::Copy( binding.pName, res.name.length() + 1, res.name.c_str() );

                    bindingArray.push_back( binding );
                }

                return bindingArray;
            }

            ShaderInputBindingArray DXCompiler::GetDXILInputReflection( IDxcResult* pRes )
            {
                ShaderInputBindingArray bindingArray;

                IDxcBlob* pReflectionBlob = nullptr;
                pRes->GetOutput( DXC_OUT_REFLECTION, IID_PPV_ARGS( &pReflectionBlob ), nullptr );
                BGS_ASSERT( pReflectionBlob != nullptr )
                if( pReflectionBlob == nullptr )
                {
                    return bindingArray;
                }
                DxcBuffer               reflectionBuffer = { pReflectionBlob->GetBufferPointer(), pReflectionBlob->GetBufferSize(), 0 };
                ID3D12ShaderReflection* pReflection      = nullptr;
                m_pUtils->CreateReflection( &reflectionBuffer, IID_PPV_ARGS( &pReflection ) );
                if( pReflectionBlob == nullptr )
                {
                    RELEASE_COM_PTR( pReflectionBlob );
                    return bindingArray;
                }
                D3D12_SHADER_DESC shaderDesc;
                if( FAILED( pReflection->GetDesc( &shaderDesc ) ) )
                {
                    RELEASE_COM_PTR( pReflectionBlob );
                    RELEASE_COM_PTR( pReflection );
                    return bindingArray;
                }

                for( index_t ndx = 0; ndx < shaderDesc.InputParameters; ++ndx )
                {
                    D3D12_SIGNATURE_PARAMETER_DESC inputDesc;
                    pReflection->GetInputParameterDesc( static_cast<uint32_t>( ndx ), &inputDesc );

                    ShaderInputBindingInfo binding;
                    String                 semName = ( inputDesc.SemanticName );
                    semName += inputDesc.SemanticIndex ? std::to_string( inputDesc.SemanticIndex ) : "";
                    binding.location       = inputDesc.Register;
                    binding.type           = MapDXILTypeToBigosShaderInputBaseDataType( inputDesc.ComponentType );
                    binding.componentCount = GetComponentCountFromDXILMask( inputDesc.Mask );
                    Core::Utils::String::Copy( binding.pSemanticName, semName.length() + 1, semName.c_str() );

                    bindingArray.push_back( binding );
                }

                RELEASE_COM_PTR( pReflectionBlob );
                RELEASE_COM_PTR( pReflection );

                return bindingArray;
            }

            ShaderInputBindingArray DXCompiler::GetSPIRVInputReflection( void* pSrc, uint32_t srcSize )
            {
                ShaderInputBindingArray bindingArray;

                spirv_cross::Compiler        compiler( reinterpret_cast<uint32_t*>( pSrc ), srcSize / 4 );
                spirv_cross::ShaderResources resources = compiler.get_shader_resources();

                for( index_t ndx = 0; ndx < resources.stage_inputs.size(); ++ndx )
                {
                    ShaderInputBindingInfo       binding;
                    const spirv_cross::Resource& res     = resources.stage_inputs[ ndx ];
                    String                       semName = GetSemanticNameFromSPIRVName( res.name );
                    binding.location                     = compiler.get_decoration( res.id, spv::DecorationLocation );
                    spirv_cross::SPIRType type           = compiler.get_type( res.type_id );
                    binding.type                         = MapSPIRVTypeToBigosShaderInputBaseDataType( type );
                    binding.componentCount               = type.vecsize;
                    Core::Utils::String::Copy( binding.pSemanticName, semName.length() + 1, semName.c_str() );

                    bindingArray.push_back( binding );
                }

                return bindingArray;
            }

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS
