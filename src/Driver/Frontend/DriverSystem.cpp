#include "Driver/Frontend/DriverSystem.h"

#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Memory.h"
#include "Driver/Backend/D3D12/D3D12Factory.h"
#include "Driver/Backend/Vulkan/VulkanFactory.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            RESULT DriverSystem::Create( const DriverSystemDesc& desc, Core::Memory::IAllocator* pAllocator, BigosFramework* pParent )
            {
                BGS_ASSERT( pParent != nullptr );
                BGS_ASSERT( pAllocator != nullptr );
                m_pDefaultAllocator = pAllocator;
                m_pParent           = pParent;
                m_desc              = desc;

                if( BGS_FAILED( CreateFactory( desc.factoryDesc, &m_pFactory ) ) )
                {
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void DriverSystem::Destroy()
            {
                if( m_pFactory != nullptr )
                {
                    DestroyFactory( &m_pFactory );
                }

                m_pDefaultAllocator = nullptr;
                m_pParent           = nullptr;
            }

            RESULT DriverSystem::CreateFactory( const Backend::FactoryDesc& desc, Backend::IFactory** ppFactory )
            {
                BGS_ASSERT( ( ppFactory != nullptr ) && ( *ppFactory == nullptr ) );
                BGS_ASSERT( m_pFactory == nullptr );

                const Backend::API_TYPE apiType = desc.apiType;
                if( apiType == Backend::APITypes::VULKAN )
                {
                    Backend::VulkanFactory* pFactory = nullptr;
                    if( BGS_FAILED( Memory::AllocateObject( m_pDefaultAllocator, &pFactory ) ) )
                    {
                        return Results::NO_MEMORY;
                    }

                    if( BGS_FAILED( pFactory->Create( desc, this ) ) )
                    {
                        Memory::FreeObject( m_pDefaultAllocator, &pFactory );
                        return Results::FAIL;
                    }

                    m_pFactory = pFactory;
                }
                else if( apiType == Backend::APITypes::D3D12 )
                {
                    Backend::D3D12Factory* pFactory = nullptr;
                    if( BGS_FAILED( Memory::AllocateObject( m_pDefaultAllocator, &pFactory ) ) )
                    {
                        return Results::NO_MEMORY;
                    }

                    if( BGS_FAILED( pFactory->Create( desc, this ) ) )
                    {
                        Memory::FreeObject( m_pDefaultAllocator, &pFactory );
                        return Results::FAIL;
                    }

                    m_pFactory = pFactory;
                }
                else
                {
                    return Results::NOT_FOUND;
                }

                BGS_ASSERT( m_pFactory != nullptr );
                *ppFactory = m_pFactory;

                return Results::OK;
            }

            void DriverSystem::DestroyFactory( Backend::IFactory** ppFactory )
            {
                const Backend::API_TYPE apiType = m_desc.factoryDesc.apiType;
                if( apiType == Backend::APITypes::VULKAN )
                {
                    Backend::VulkanFactory* pFactory = static_cast<Backend::VulkanFactory*>( *ppFactory );
                    pFactory->Destroy();
                    Core::Memory::FreeObject( m_pDefaultAllocator, &pFactory );
                    pFactory = nullptr;
                }
                else if( apiType == Backend::APITypes::D3D12 )
                {
                    Backend::D3D12Factory* pFactory = static_cast<Backend::D3D12Factory*>( *ppFactory );
                    pFactory->Destroy();
                    Core::Memory::FreeObject( m_pDefaultAllocator, &pFactory );
                    pFactory = nullptr;
                }
                else
                {
                    BGS_ASSERT( 0 );
                }
            }

        } // namespace Frontend
    }     // namespace Driver
} // namespace BIGOS