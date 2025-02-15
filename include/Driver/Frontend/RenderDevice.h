#pragma once

#include "Driver/Frontend/RenderSystemTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            class BGS_API RenderDevice final
            {
                friend class RenderSystem;

            public:
                RenderDevice();
                ~RenderDevice() = default;

                RESULT CreateBuffer( const BufferDesc& desc, Buffer** ppBuffer );
                void   DestroyBuffer( Buffer** ppBuffer );

                RESULT CreateRenderTarget( const RenderTargetDesc& desc, RenderTarget** ppRenderTarget );
                void   DestroyRenderTarget( RenderTarget** ppRenderTarget );

                RESULT CreateShader( const ShaderDesc& desc, Shader** ppShader );
                void   DestroyShader( Shader** ppShader );

                GraphicsContext* GetGraphicsContext() { return m_pGraphicsContext; }
                ComputeContext*  GetComputeContext() { return m_pComputeContext; }
                CopyContext*     GetCopyContext() { return m_pCopyContext; }

                Backend::IDevice* GetNativeAPIDevice() { return m_pAPIDevice; }
                RenderSystem*     GetParent() { return m_pParent; }

            protected:
                RESULT Create( const RenderDeviceDesc& desc, Backend::IFactory* pFactory, RenderSystem* pDriverSystem );
                void   Destroy();

            private:
                RESULT CreateContexts();
                void   DestroyContexts();

            private:
                RenderDeviceDesc   m_desc;
                Backend::IFactory* m_pFactory;
                RenderSystem*      m_pParent;
                Backend::IDevice*  m_pAPIDevice;
                GraphicsContext*   m_pGraphicsContext;
                ComputeContext*    m_pComputeContext;
                CopyContext*       m_pCopyContext;
            };

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS