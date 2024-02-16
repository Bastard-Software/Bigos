#include "Dxc.h"

#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Memory.h"
#include "Core/Utils/String.h"
#include "Driver/Frontend/RenderSystem.h"
#include "Platform/Platform.h"
#include "ShaderCompilerFactory.h"

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

                RELEASE_COM_PTR( pRes );
                RELEASE_COM_PTR( pSrc );

                ShaderCompilerOutput* pOutput = nullptr;
                if( pBlob == nullptr )
                {
                    return Results::FAIL;
                }
                else
                {
                    const uint32_t allocSize = static_cast<uint32_t>( pBlob->GetBufferSize() + sizeof( ShaderCompilerOutput ) );
                    byte_t*        pMem      = nullptr;
                    if( BGS_FAILED( Core::Memory::AllocateBytes( m_pParent->GetParent()->GetDefaultAllocator(), &pMem, allocSize ) ) )
                    {
                        RELEASE_COM_PTR( pBlob );
                        return Results::NO_MEMORY;
                    }

                    pOutput               = reinterpret_cast<ShaderCompilerOutput*>( pMem );
                    pOutput->pByteCode    = pMem + sizeof( ShaderCompilerOutput );
                    pOutput->byteCodeSize = static_cast<uint32_t>( pBlob->GetBufferSize() );

                    Core::Memory::Copy( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), pOutput->pByteCode, pOutput->byteCodeSize );
                }
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

        } // namespace Frontend
    }     // namespace Driver
} // namespace BIGOS
