#include "Driver/Frontend/RenderSystem.h"

#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Memory.h"
#include "Driver/Backend/D3D12/D3D12Factory.h"
#include "Driver/Backend/Vulkan/VulkanFactory.h"
#include "Driver/Frontend/Camera/Camera.h"
#include "Driver/Frontend/RenderDevice.h"
#include "Shader/ShaderCompilerFactory.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            RenderSystem::RenderSystem()
                : m_desc()
                , m_pFactory( nullptr )
                , m_adapters()
                , m_devices()
                , m_cameras()
                , m_pParent( nullptr )
                , m_pShaderCompilerFactory( nullptr )
                , m_pDefaultAllocator( nullptr )
                , m_pCompiler( nullptr )
            {
            }

            RESULT RenderSystem::CreateDevice( const RenderDeviceDesc& desc, RenderDevice** ppDevice )
            {
                BGS_ASSERT( m_pFactory != nullptr,
                            "Invalid factory (m_pFactory) pointer. Factory hasn't been created properly or has been destroyed during runtime." );
                BGS_ASSERT( ppDevice != nullptr, "ppDevice must be a valid address." );

                if( m_pFactory == nullptr )
                {
                    return Results::FAIL;
                }

                RenderDevice* pDevice = ( *ppDevice );
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

            void RenderSystem::DestroyDevice( RenderDevice** ppDevice )
            {
                BGS_ASSERT( ( ppDevice != nullptr ) && ( *ppDevice != nullptr ), "ppDevice must be an valid address of a valid pointer." );

                if( ppDevice != nullptr )
                {

                    RenderDevice* pDevice = ( *ppDevice );

                    for( index_t ndx = 0; ndx < m_devices.size(); ++ndx )
                    {
                        // TODO: Find device in array using hash
                        if( m_devices[ ndx ] == pDevice )
                        {
                            m_devices[ ndx ] = m_devices.back();
                            m_devices.pop_back();
                            pDevice->Destroy();
                            Memory::FreeObject( m_pDefaultAllocator, &pDevice );
                            ppDevice = nullptr;
                            break;
                        }
                    }
                }
            }

            RESULT RenderSystem::CreateCamera( const CameraDesc& desc, Camera** ppCamera )
            {
                BGS_ASSERT( ppCamera != nullptr, "Camera (ppCamera) must be a valid address." );

                Camera* pCamera = ( *ppCamera );
                if( pCamera != nullptr )
                {
                    for( index_t ndx = 0; ndx < m_cameras.size(); ++ndx )
                    {
                        // TODO: Find camera in array using hash
                        if( m_cameras[ ndx ] == pCamera )
                        {
                            return Results::OK;
                        }
                    }

                    // TDOD: Warrning that camera wasn't created by framework?
                    m_cameras.push_back( pCamera );

                    return Results::OK;
                }
                else
                {
                    if( BGS_FAILED( Memory::AllocateObject( m_pDefaultAllocator, &pCamera ) ) )
                    {
                        return Results::NO_MEMORY;
                    }

                    if( BGS_FAILED( pCamera->Create( desc, this ) ) )
                    {
                        Memory::FreeObject( m_pDefaultAllocator, &pCamera );
                        return Results::FAIL;
                    }
                }

                m_cameras.push_back( pCamera );
                ( *ppCamera ) = pCamera;

                return Results::OK;
            }

            void RenderSystem::DestroyCamera( Camera** ppCamera )
            {
                BGS_ASSERT( ( ppCamera != nullptr ) && ( *ppCamera != nullptr ), " Camera (ppCamera) must be an valid address of a valid pointer." );

                if( ppCamera != nullptr )
                {

                    Camera* pCamera = ( *ppCamera );

                    for( index_t ndx = 0; ndx < m_cameras.size(); ++ndx )
                    {
                        // TODO: Find device in array using hash
                        if( m_cameras[ ndx ] == pCamera )
                        {
                            m_cameras[ ndx ] = m_cameras.back();
                            m_cameras.pop_back();
                            pCamera->Destroy();
                            Memory::FreeObject( m_pDefaultAllocator, &pCamera );
                            ppCamera = nullptr;
                            break;
                        }
                    }
                }
            }

            RESULT RenderSystem::Create( const RenderSystemDesc& desc, Core::Memory::IAllocator* pAllocator, BigosFramework* pFramework )
            {
                BGS_ASSERT( pFramework != nullptr, "Bigos framework (pFramework) must be a valid pointer." );
                BGS_ASSERT( pAllocator != nullptr, "Allocator (pAllocator) must be a valid pointer." );

                m_desc              = desc;
                m_pDefaultAllocator = pAllocator;
                m_pParent           = pFramework;

                if( BGS_FAILED( CreateFactory( desc.factoryDesc, &m_pFactory ) ) )
                {
                    return Results::FAIL;
                }

                if( BGS_FAILED( CreateShaderCompilerFactory( desc.compilerFactoryDesc, &m_pShaderCompilerFactory ) ) )
                {
                    DestroyFactory( &m_pFactory );
                    return Results::FAIL;
                }

                ShaderCompilerDesc compDesc;
                compDesc.type = CompilerTypes::DXC;
                if( BGS_FAILED( m_pShaderCompilerFactory->CreateCompiler( compDesc, &m_pCompiler ) ) )
                {
                    DestroyShaderCompilerFactory( &m_pShaderCompilerFactory );
                    DestroyFactory( &m_pFactory );
                }

                return Results::OK;
            }

            void RenderSystem::Destroy()
            {
                for( index_t ndx = 0; ndx < m_devices.size(); ++ndx )
                {
                    DestroyDevice( &m_devices[ ndx ] );
                }

                for( index_t ndx = 0; ndx < m_cameras.size(); ++ndx )
                {
                    DestroyCamera( &m_cameras[ ndx ] );
                }

                if( m_pFactory != nullptr )
                {
                    DestroyFactory( &m_pFactory );
                }

                if( m_pShaderCompilerFactory != nullptr )
                {
                    DestroyShaderCompilerFactory( &m_pShaderCompilerFactory );
                }

                m_devices.clear();
                m_adapters.clear();

                m_pDefaultAllocator = nullptr;
                m_pParent           = nullptr;
            }

            RESULT RenderSystem::CreateFactory( const Backend::FactoryDesc& desc, Backend::IFactory** ppFactory )
            {
                BGS_ASSERT( ppFactory != nullptr, "Factory (ppFactory) must be a valid address." );
                BGS_ASSERT( *ppFactory == nullptr, "There is a pointer at the given address. Factory (*ppFactory) must be nullptr." );
                if( m_pFactory != nullptr )
                {
                    *ppFactory = m_pFactory;
                    return Results::OK;
                }

                if( ( ppFactory == nullptr ) || ( *ppFactory != nullptr ) )
                {
                    return Results::FAIL;
                }

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

            void RenderSystem::DestroyFactory( Backend::IFactory** ppFactory )
            {
                BGS_ASSERT( ppFactory != nullptr, "Factory (ppFactory) must be a valid address." );
                BGS_ASSERT( *ppFactory != nullptr, "Factory (*ppFactory) must be a valid pointer." );
                BGS_ASSERT( *ppFactory == m_pFactory, "Factory (*ppFactory) has not been created by Bigos Framework." );

                if( ( ppFactory != nullptr ) && ( *ppFactory != nullptr ) && ( *ppFactory == m_pFactory ) )
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
            }

            RESULT RenderSystem::CreateShaderCompilerFactory( const ShaderCompilerFactoryDesc& desc, ShaderCompilerFactory** ppFactory )
            {
                BGS_ASSERT( ppFactory != nullptr, "Shader compiler factory (ppFactory) must be a valid address." );
                BGS_ASSERT( *ppFactory == nullptr, "There is a pointer at the given address. Shader compiler factory (*ppFactory) must be nullptr." );
                if( ( ppFactory == nullptr ) || ( *ppFactory != nullptr ) )
                {
                    return Results::FAIL;
                }

                if( m_pShaderCompilerFactory != nullptr )
                {
                    *ppFactory = m_pShaderCompilerFactory;
                    return Results::OK;
                }

                if( BGS_FAILED( Memory::AllocateObject( m_pDefaultAllocator, &m_pShaderCompilerFactory ) ) )
                {
                    return Results::NO_MEMORY;
                }

                if( BGS_FAILED( m_pShaderCompilerFactory->Create( desc, this ) ) )
                {
                    Memory::FreeObject( m_pDefaultAllocator, &m_pShaderCompilerFactory );
                    return Results::FAIL;
                }

                *ppFactory = m_pShaderCompilerFactory;

                return Results::OK;
            }

            void RenderSystem::DestroyShaderCompilerFactory( ShaderCompilerFactory** ppFactory )
            {
                BGS_ASSERT( ppFactory != nullptr, "Shader compiler factory (ppFactory) must be a valid address." );
                BGS_ASSERT( *ppFactory != nullptr, "Shader compiler factory (*ppFactory) must be a valid pointer." );
                BGS_ASSERT( *ppFactory == m_pShaderCompilerFactory, "Shader compiler factory (*ppFactory) has not been created by Bigos Framework." );
                if( ( ppFactory != nullptr ) && ( *ppFactory != nullptr ) && ( *ppFactory == m_pShaderCompilerFactory ) )
                {
                    m_pShaderCompilerFactory->Destroy();
                    Memory::FreeObject( m_pDefaultAllocator, &m_pShaderCompilerFactory );
                }
            }

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS