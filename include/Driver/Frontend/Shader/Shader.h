#pragma once

#include "Driver/Frontend/RenderSystemTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            class BGS_API Shader final
            {
                friend class RenderDevice;

            public:
                Shader();
                ~Shader() = default;

                ShaderCompilerOutput* GetCompiledShader() const { return m_pCompiledShader; }
                Backend::ShaderHandle GetHandle() const { return m_hShader; }

            protected:
                RESULT Create( const ShaderDesc& desc, RenderDevice* pDevice );
                void   Destroy();

            private:
                SHADER_LANGUAGE      DetectShaderLanguage( const String& shaderCode );

            private:
                CompileShaderDesc     m_compileDesc;
                Backend::ShaderHandle m_hShader;
                RenderDevice*         m_pParent;
                ShaderCompilerOutput* m_pCompiledShader;
                const char*           m_pEntryPoint;
                Backend::SHADER_TYPE  m_type;
            };

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS