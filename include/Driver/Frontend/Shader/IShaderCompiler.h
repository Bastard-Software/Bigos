#pragma once

#include "Driver/Frontend/RenderSystemTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {
            class BGS_API BGS_API_INTERFACE IShaderCompiler
            {
            public:
                ~IShaderCompiler() = default;

                virtual RESULT Compile( const CompileShaderDesc& desc, ShaderCompilerOutput** ppOutput ) = 0;
                virtual void   DestroyOutput( ShaderCompilerOutput** ppOutput )                          = 0;

                const ShaderCompilerDesc& GetDesc() const { return m_desc; }

            protected:
                ShaderCompilerDesc m_desc;
            };
        } // namespace Frontend
    }     // namespace Driver
} // namespace BIGOS