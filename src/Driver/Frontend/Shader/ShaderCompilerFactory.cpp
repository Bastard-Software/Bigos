#include "ShaderCompilerFactory.h"

#include "Driver/Frontend/RenderSystemTypes.h"

#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Memory.h"
#include "Driver/Frontend/RenderSystem.h"
#include "Dxc.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            ShaderCompilerFactory::ShaderCompilerFactory()
                : m_compilers()
                , m_pParent( nullptr )
            {
            }

            RESULT ShaderCompilerFactory::CreateCompiler( const ShaderCompilerDesc& desc, IShaderCompiler** ppCompiler )
            {
                BGS_ASSERT( ppCompiler != nullptr, "Shader compiler (ppCompiler) must be a valid address." );
                BGS_ASSERT( *ppCompiler == nullptr, "There is a pointer at the given address. Shader compiler (*ppCompiler) must be nullptr." );
                if( ( ppCompiler == nullptr ) || ( *ppCompiler != nullptr ) )
                {
                    return Results::FAIL;
                }

                if( desc.type == CompilerTypes::DXC )
                {
                    DXCompiler* pDxc = nullptr;
                    if( BGS_FAILED( Memory::AllocateObject( m_pParent->GetDefaultAllocator(), &pDxc ) ) )
                    {
                        return Results::NO_MEMORY;
                    }

                    if( BGS_FAILED( pDxc->Create( desc, this ) ) )
                    {
                        Memory::FreeObject( m_pParent->GetDefaultAllocator(), &pDxc );
                        return Results::FAIL;
                    }

                    m_compilers[ BGS_ENUM_INDEX( CompilerTypes::DXC ) ] = pDxc;
                }
                else if( desc.type == CompilerTypes::GLSLC )
                {
                    // TODO: implement with glsl support
                }
                else
                {
                    return Results::NOT_FOUND;
                }

                *ppCompiler = m_compilers[ BGS_ENUM_INDEX( desc.type ) ];

                return Results::OK;
            }

            void ShaderCompilerFactory::DestroyCompiler( IShaderCompiler** ppCompiler )
            {
                BGS_ASSERT( ppCompiler != nullptr, "Shader compiler (ppCompiler) must be a valid address." );
                BGS_ASSERT( *ppCompiler != nullptr, "Factory (*ppCompiler) must be a valid pointer." );
                BGS_ASSERT( *ppCompiler == m_compilers[ BGS_ENUM_INDEX( ( *ppCompiler )->GetDesc().type ) ],
                            "Factory (*ppCompiler) has not been created by Bigos Framework." );

                const COMPILER_TYPE type = ( *ppCompiler )->GetDesc().type;
                if( ( ppCompiler != nullptr ) && ( *ppCompiler != nullptr ) &&
                    ( *ppCompiler == m_compilers[ BGS_ENUM_INDEX( ( *ppCompiler )->GetDesc().type ) ] ) )
                {
                    if( type == CompilerTypes::DXC )
                    {
                        DXCompiler* pDxc = static_cast<DXCompiler*>( m_compilers[ BGS_ENUM_INDEX( ( *ppCompiler )->GetDesc().type ) ] );
                        pDxc->Destroy();
                        Core::Memory::FreeObject( m_pParent->GetDefaultAllocator(), &pDxc );
                        pDxc = nullptr;
                    }
                    else if( type == CompilerTypes::GLSLC )
                    {
                        // TODO: Implement
                    }
                    else
                    {
                        BGS_ASSERT( 0 );
                    }
                }
            }

            RESULT ShaderCompilerFactory::Create( const ShaderCompilerFactoryDesc& desc, RenderSystem* pRenderSystem )
            {
                BGS_ASSERT( pRenderSystem != nullptr, "Render system (pRenderSystem) must be a valid pointer." );
                if( pRenderSystem == nullptr )
                {
                    return Results::OK;
                }

                m_desc    = desc;
                m_pParent = pRenderSystem;

                // Setting array to null
                Memory::Set( m_compilers, 0, sizeof( ShaderCompilerPtr ) * BGS_ENUM_COUNT( CompilerTypes ) );

                return Results::OK;
            }

            void ShaderCompilerFactory::Destroy()
            {
                {
                    DestroyCompiler( &m_compilers[ BGS_ENUM_INDEX( CompilerTypes::DXC ) ] );
                }

                {
                    // TODO: Destroy glslc shader compiler
                }

                // Setting array to null
                Memory::Set( m_compilers, 0, sizeof( ShaderCompilerPtr ) * BGS_ENUM_COUNT( CompilerTypes ) );
            }

        } // namespace Frontend
    }     // namespace Driver
} // namespace BIGOS