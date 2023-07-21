#pragma once
#include "Core/Memory/MemoryTypes.h"
#include "Driver/Frontend/DriverSystemTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {
            class BGS_API DriverSystem final
            {
                friend class BigosFramework;
                friend class Backend::VulkanFactory;
                friend class Backend::D3D12Factory;

            public:
                RESULT CreateDevice( const DeviceDesc& desc, Device** ppDevice );
                void   DestroyDevice( Device** ppDevice );

                BigosFramework*     GetParentPtr() { return m_pParent; }
                Memory::IAllocator* GetDefaultAllocatorPtr() { return m_pDefaultAllocator; }
                Backend::IFactory*  GetFactoryPtr() { return m_pFactory; }

                const DeviceArray&  GetDevices() const { return m_devices; }
                const AdapterArray& GetAdapters() const { return m_adapters; }

                const DriverSystemDesc& GetDesc() const { return m_desc; }

            protected:
                RESULT Create( const DriverSystemDesc& desc, Core::Memory::IAllocator* pAllocator, BigosFramework* pParent );
                void   Destroy();

            private:
                RESULT CreateFactory( const Backend::FactoryDesc& desc, Backend::IFactory** ppFactory ); // Change, factory should be private
                void   DestroyFactory( Backend::IFactory** ppFactory );

            private:
                DriverSystemDesc    m_desc;
                Backend::IFactory*  m_pFactory;
                AdapterArray        m_adapters;
                DeviceArray         m_devices;
                BigosFramework*     m_pParent;
                Memory::IAllocator* m_pDefaultAllocator;
            };

        } // namespace Frontend
    }     // namespace Driver
} // namespace BIGOS