#pragma once
#include "Driver/Frontend/DriverSystemTypes.h"

#include "Core/Memory/IAllocator.h"

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

                const DriverSystemDesc& GetDesc() const { return m_desc; }

                BigosFramework* GetParentPtr() { return m_pParent; }

            protected:
                RESULT Create( const DriverSystemDesc& desc, Core::Memory::IAllocator* pAllocator, BigosFramework* pParent );
                void   Destroy();

            private:
                RESULT CreateFactory( const Backend::FactoryDesc& desc, Backend::IFactory** ppFactory );
                template<class FactoryT>
                void DestroyFactory( FactoryT** ppFactory );

            private:
                DriverSystemDesc    m_desc;
                Backend::IFactory*  m_pFactory;
                BigosFramework*     m_pParent;
                Memory::IAllocator* m_pDefaultAllocator;
            };

            template<class FactoryT>
            void DriverSystem::DestroyFactory( FactoryT** ppFactory )
            {
                BGS_ASSERT( ( ppFactory != nullptr ) && ( *ppFactory != nullptr ) );

                if( *ppFactory != nullptr )
                {
                    FactoryT* pFactory = *ppFactory;
                    pFactory->Destroy();
                    Core::Memory::FreeObject( m_pDefaultAllocator, &pFactory );
                    pFactory = nullptr;
                }
            }

        } // namespace Frontend
    }     // namespace Driver
} // namespace BIGOS