#include "Driver/Frontend/DriverSystem.h"

#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Memory.h"
#include "Driver/Backend/D3D12/D3D12Factory.h"
#include "Driver/Backend/Vulkan/VulkanFactory.h"
#include "Driver/Frontend/Device.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            RESULT DriverSystem::CreateDevice( const DeviceDesc& desc, Device** ppDevice )
            {
                BGS_ASSERT( m_pFactory != nullptr,
                            "Invalid factory (m_pFactory) pointer. Factory hasn't been created properly or has been destroyed during runtime." );
                BGS_ASSERT( ppDevice != nullptr, "ppDevice must be a valid address." );

                if( m_pFactory == nullptr )
                {
                    return Results::FAIL;
                }

                Device* pDevice = ( *ppDevice );
                if( pDevice != nullptr )
                {
                    for( index_t ndx = 0; ndx < m_devices.size(); ++ndx )
                    {
                        // TODO: Find device in array using hash
                        if( m_devices[ ndx ] == pDevice )
                        {
                            return Results::OK;
                        }
                    }

                    // TDOD: Warrning that device wasn't created by framework?
                    m_devices.push_back( pDevice );

                    return Results::OK;
                }
                else
                {
                    if( BGS_FAILED( Memory::AllocateObject( m_pDefaultAllocator, &pDevice ) ) )
                    {
                        return Results::NO_MEMORY;
                    }

                    if( BGS_FAILED( pDevice->Create( desc, m_pFactory, this ) ) )
                    {
                        Memory::FreeObject( m_pDefaultAllocator, &pDevice );
                        return Results::FAIL;
                    }
                }

                BGS_ASSERT( pDevice != nullptr );
                m_devices.push_back( pDevice );
                ( *ppDevice ) = pDevice;

                return Results::OK;
            }

            void DriverSystem::DestroyDevice( Device** ppDevice )
            {
                BGS_ASSERT( ( ppDevice != nullptr ) && ( *ppDevice != nullptr ), "ppDevice must be an valid address of a valid pointer." );

                Device* pDevice = ( *ppDevice );

                for (index_t ndx = 0; ndx < m_devices.size(); ++ndx)
                {
                    // TODO: Find device in array using hash
                    if (m_devices[ndx] == pDevice)
                    {
                        m_devices[ ndx ] = m_devices.back();
                        m_devices.pop_back();
                        pDevice->Destroy();
                        Memory::FreeObject( m_pDefaultAllocator, &pDevice );
                        break;
                    }
                }
            }

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

                for (index_t ndx = 0; ndx < m_devices.size(); ++ndx)
                {
                    DestroyDevice( &m_devices[ ndx ] );
                }

                m_devices.clear();
                m_adapters.clear();

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
                m_adapters = m_pFactory->GetAdapters();

                return Results::OK;
            }

            void DriverSystem::DestroyFactory( Backend::IFactory** ppFactory )
            {
                BGS_ASSERT( ( ppFactory != nullptr ) && ( *ppFactory != nullptr ), "ppDevice must be an valid address of a valid pointer." );

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