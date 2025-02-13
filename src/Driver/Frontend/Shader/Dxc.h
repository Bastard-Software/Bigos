#pragma once

#include "Driver/Frontend/RenderSystemTypes.h"
#include "Platform/PlatformTypes.h"

#include "Driver/Frontend/Shader/IShaderCompiler.h"
#include <dxc/dxcapi.h>

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {
            class BGS_API DXCompiler final : public IShaderCompiler
            {
                friend class ShaderCompilerFactory;

            public:
                DXCompiler();
                ~DXCompiler() = default;

                virtual RESULT Compile( const CompileShaderDesc& desc, ShaderCompilerOutput** ppOutput ) override;
                virtual void   DestroyOutput( ShaderCompilerOutput** ppOutput ) override;

            protected:
                RESULT Create( const ShaderCompilerDesc& desc, ShaderCompilerFactory* pFactory );
                void   Destroy();

            private:
                ShaderBindingArray GetDXILReflection( IDxcResult* pRes );
                ShaderBindingArray GetSPIRVReflection( void* pSrc, uint32_t srcSize );

                ShaderInputBindingArray GetDXILInputReflection( IDxcResult* pRes );
                ShaderInputBindingArray GetSPIRVInputReflection( void* pSrc, uint32_t srcSize );

            private:
                ShaderCompilerFactory* m_pParent;
                IDxcUtils*             m_pUtils;
                IDxcCompiler3*         m_pCompiler;
                LibraryHandle          m_libraryHandle;
                // TODO:
            };
        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS