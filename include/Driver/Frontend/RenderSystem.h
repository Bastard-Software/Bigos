#pragma once
#include "Core/Memory/MemoryTypes.h"
#include "Driver/Frontend/RenderSystemTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {
            class BGS_API RenderSystem final
            {
                friend class BigosEngine;

            public:
                RenderSystem();
                ~RenderSystem() = default;

                // Driver primitives
                RESULT InitializeDriver( const DriverDesc& desc );

                RESULT CreateBuffer( const BufferDesc& desc, Buffer** ppBuffer );
                void   DestroyBuffer( Buffer** ppBuffer );

                RESULT CreateTexture( const TextureDesc& desc, Texture** ppTexture );
                void   DestroyTexture( Texture** ppTexture );

                RESULT CreateRenderTarget( const RenderTargetDesc& desc, RenderTarget** ppRenderTarget );
                void   DestroyRenderTarget( RenderTarget** ppRenderTarget );

                RESULT CreateRenderPass( const RenderPassDesc& desc, RenderPass** ppRenderPass );
                void   DestroyRenderPass( RenderPass** ppRenderPass );

                RESULT CreateShader( const ShaderDesc& desc, Shader** ppShader );
                void   DestroyShader( Shader** ppShader );

                RESULT CreatePipeline( const GraphicsPipelineDesc& desc, Pipeline** ppPipeline );
                RESULT CreatePipeline( const ComputePipelineDesc& desc, Pipeline** ppPipeline );
                void   DestroyPipeline( Pipeline** ppPipeline );

                RESULT CreateSwapchain( const SwapchainDesc& desc, Swapchain** ppSwapchain );
                void   DestroySwapchain( Swapchain** ppSwapchain );

                GraphicsContext* GetGraphicsContext() { return m_pGraphicsContext; }
                ComputeContext*  GetComputeContext() { return m_pComputeContext; }
                CopyContext*     GetCopyContext() { return m_pCopyContext; }

                const AdapterArray& GetAdapters() const { return m_adapters; } // Hide
                Backend::IDevice*   GetDevice() const { return m_pDevice; }    // Hide

                // Other render system primitives
                RESULT CreateCamera( const CameraDesc& desc, Camera** ppCamera );
                void   DestroyCamera( Camera** ppCamera );

                BigosEngine*        GetParent() { return m_pParent; }
                Memory::IAllocator* GetDefaultAllocator() { return m_pDefaultAllocator; }
                Backend::IFactory*  GetFactory() { return m_pFactory; }
                IShaderCompiler*    GetDefaultCompiler() { return m_pCompiler; }

                const RenderSystemDesc& GetDesc() const { return m_desc; }
                const DriverDesc&       GetDriverDesc() const { return m_driverDesc; }

            protected:
                RESULT Create( const RenderSystemDesc& desc, Core::Memory::IAllocator* pAllocator, BigosEngine* pEngine );
                void   Destroy();

            private:
                RESULT CreateShaderCompilerFactory( const ShaderCompilerFactoryDesc& desc, ShaderCompilerFactory** ppFactory );
                void   DestroyShaderCompilerFactory( ShaderCompilerFactory** ppFactory );

                // Driver connected stuff
                RESULT CreateFactory( const Backend::FactoryDesc& desc, Backend::IFactory** ppFactory ); // Change, factory should be private
                void   DestroyFactory( Backend::IFactory** ppFactory );

                RESULT CreateContexts();
                void   DestroyContexts();

                void FreeDriver();

            private:
                RenderSystemDesc       m_desc;
                DriverDesc             m_driverDesc;
                Backend::IFactory*     m_pFactory;
                Backend::IDevice*      m_pDevice;
                AdapterArray           m_adapters;
                GraphicsContext*       m_pGraphicsContext;
                ComputeContext*        m_pComputeContext;
                CopyContext*           m_pCopyContext;
                CameraArray            m_cameras; // Shaould be handled by resource manager
                BigosEngine*           m_pParent;
                ShaderCompilerFactory* m_pShaderCompilerFactory;
                Memory::IAllocator*    m_pDefaultAllocator;
                IShaderCompiler*       m_pCompiler;
            };

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS