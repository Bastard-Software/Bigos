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
                RESULT CreateDevice( const RenderDeviceDesc& desc, RenderDevice** ppDevice );
                void   DestroyDevice( RenderDevice** ppDevice );

                BigosFramework*     GetParentPtr() { return m_pParent; }
                Memory::IAllocator* GetDefaultAllocatorPtr() { return m_pDefaultAllocator; }
                Backend::IFactory*  GetFactoryPtr() { return m_pFactory; }

                const RenderDeviceArray& GetDevices() const { return m_devices; }
                const AdapterArray&      GetAdapters() const { return m_adapters; }

                const RenderSystemDesc& GetDesc() const { return m_desc; }

            protected:
                RESULT Create( const RenderSystemDesc& desc, Core::Memory::IAllocator* pAllocator, BigosFramework* pParent );
                void   Destroy();

            private:
                RESULT CreateFactory( const Backend::FactoryDesc& desc, Backend::IFactory** ppFactory ); // Change, factory should be private
                void   DestroyFactory( Backend::IFactory** ppFactory );

            private:
                RenderSystemDesc    m_desc;
                Backend::IFactory*  m_pFactory;
                AdapterArray        m_adapters;
                RenderDeviceArray   m_devices;
                BigosFramework*     m_pParent;
                Memory::IAllocator* m_pDefaultAllocator;
            };

        } // namespace Frontend
    }     // namespace Driver
} // namespace BIGOS