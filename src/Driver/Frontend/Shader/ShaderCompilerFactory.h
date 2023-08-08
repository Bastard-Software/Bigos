#pragma once

#include "Driver/Frontend/RenderSystemTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {
            class ShaderCompilerFactory final
            {
                friend class RenderSystem;

                using ShaderCompilerPtr = IShaderCompiler*;
                using CompilerArray     = ShaderCompilerPtr[ BGS_ENUM_COUNT( CompilerTypes ) ];

            public:
                ShaderCompilerFactory();
                ~ShaderCompilerFactory() = default;

                ShaderCompilerPtr GetCompiler( COMPILER_TYPE type ) const { return m_compilers[ BGS_ENUM_INDEX( type ) ]; }

                RESULT CreateCompiler( const ShaderCompilerDesc& desc, IShaderCompiler** ppCompiler );
                void   DestroyCompiler( IShaderCompiler** ppCompiler );

                RenderSystem* GetParent() { return m_pParent; }

            protected:
                RESULT Create( const ShaderCompilerFactoryDesc& desc, RenderSystem* pRenderSystem );
                void   Destroy();

            private:
                ShaderCompilerFactoryDesc m_desc;
                CompilerArray             m_compilers;
                RenderSystem*             m_pParent;
            };
        } // namespace Frontend
    }     // namespace Driver
} // namespace BIGOS