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
            public:
                friend class BigosFramework;
                friend class Backend::VulkanFactory;
                friend class Backend::D3D12Factory;

                BigosFramework*     GetParentPtr() { return m_pParent; }
                Memory::IAllocator* GetDefaultAllocatorPtr() { return m_pDefaultAllocator; }
                Backend::IFactory*  GetFactoryPtr() { return m_pFactory; }

                const DriverSystemDesc& GetDesc() const { return m_desc; }

            protected:
                RESULT Create( const DriverSystemDesc& desc, Core::Memory::IAllocator* pAllocator, BigosFramework* pParent );
                void   Destroy();

            private:
                RESULT CreateFactory( const Backend::FactoryDesc& desc, Backend::IFactory** ppFactory );
                void   DestroyFactory( Backend::IFactory** ppFactory );

            private:
                DriverSystemDesc    m_desc;
                Backend::IFactory*  m_pFactory;
                BigosFramework*     m_pParent;
                Memory::IAllocator* m_pDefaultAllocator;
            };

        } // namespace Frontend
    }     // namespace Driver
} // namespace BIGOS