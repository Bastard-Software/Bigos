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
                friend class BigosFramework;
                friend class Backend::VulkanFactory;
                friend class Backend::D3D12Factory;

            public:
                RenderSystem();
                ~RenderSystem() = default;

                RESULT CreateDevice( const RenderDeviceDesc& desc, RenderDevice** ppDevice );
                void   DestroyDevice( RenderDevice** ppDevice );

                RESULT CreateCamera( const CameraDesc& desc, Camera** ppCamera );
                void   DestroyCamera( Camera** ppCamera);

                void SetDefaultCompiler( IShaderCompiler* pCompiler ) { m_pCompiler = pCompiler; }

                BigosFramework*     GetParent() { return m_pParent; }
                Memory::IAllocator* GetDefaultAllocator() { return m_pDefaultAllocator; }
                Backend::IFactory*  GetFactory() { return m_pFactory; }
                IShaderCompiler*    GetDefaultCompiler() { return m_pCompiler; }
                RenderDevice*       GetDevaultDevice() { return m_devices[ 0 ]; }

                const RenderDeviceArray& GetDevices() const { return m_devices; }
                const AdapterArray&      GetAdapters() const { return m_adapters; }

                const RenderSystemDesc& GetDesc() const { return m_desc; }

            protected:
                RESULT Create( const RenderSystemDesc& desc, Core::Memory::IAllocator* pAllocator, BigosFramework* pFramework );
                void   Destroy();

            private:
                RESULT CreateFactory( const Backend::FactoryDesc& desc, Backend::IFactory** ppFactory ); // Change, factory should be private
                void   DestroyFactory( Backend::IFactory** ppFactory );

                RESULT CreateShaderCompilerFactory( const ShaderCompilerFactoryDesc& desc, ShaderCompilerFactory** ppFactory );
                void   DestroyShaderCompilerFactory( ShaderCompilerFactory** ppFactory );

            private:
                RenderSystemDesc       m_desc;
                Backend::IFactory*     m_pFactory;
                AdapterArray           m_adapters;
                RenderDeviceArray      m_devices;
                CameraArray            m_cameras;
                BigosFramework*        m_pParent;
                ShaderCompilerFactory* m_pShaderCompilerFactory;
                Memory::IAllocator*    m_pDefaultAllocator;
                IShaderCompiler*       m_pCompiler;
            };

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS