#include "Driver/Frontend/Context.h"

#include "Driver/Backend/API.h"
#include "Driver/Frontend/RenderSystem.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            GraphicsContext::GraphicsContext()
                : m_pQueue( nullptr )
                , m_pDevice( nullptr )
                , m_pParent( nullptr )
            {
            }

            RESULT GraphicsContext::Create( Backend::IDevice* pDevice, RenderSystem* pSystem )
            {
                BGS_ASSERT( pDevice != nullptr, "Device (pDevice) must be a valid pointer." );
                BGS_ASSERT( pSystem != nullptr, "Render system (pSystem) must be a valid pointer." );
                m_pDevice = pDevice;
                m_pParent = pSystem;

                Backend::QueueDesc qDesc;
                qDesc.priority = Backend::QueuePriorityTypes::HIGH;
                qDesc.type     = Backend::QueueTypes::GRAPHICS;
                if( BGS_FAILED( m_pDevice->CreateQueue( qDesc, &m_pQueue ) ) )
                {
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void GraphicsContext::Destroy()
            {
                if( m_pQueue != nullptr )
                {
                    m_pDevice->DestroyQueue( &m_pQueue );
                }
            }

            ComputeContext::ComputeContext()
                : m_pQueue( nullptr )
                , m_pDevice( nullptr )
                , m_pParent( nullptr )
            {
            }

            RESULT ComputeContext::Create( Backend::IDevice* pDevice, RenderSystem* pSystem )
            {
                BGS_ASSERT( pDevice != nullptr, "Device (pDevice) must be a valid pointer." );
                BGS_ASSERT( pSystem != nullptr, "Render system (pSystem) must be a valid pointer." );
                m_pDevice = pDevice;
                m_pParent = pSystem;

                Backend::QueueDesc qDesc;
                qDesc.priority = Backend::QueuePriorityTypes::HIGH;
                qDesc.type     = Backend::QueueTypes::COMPUTE;
                if( BGS_FAILED( m_pDevice->CreateQueue( qDesc, &m_pQueue ) ) )
                {
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void ComputeContext::Destroy()
            {
                if( m_pQueue != nullptr )
                {
                    m_pDevice->DestroyQueue( &m_pQueue );
                }
            }

            CopyContext::CopyContext()
                : m_pQueue( nullptr )
                , m_pDevice( nullptr )
                , m_pParent( nullptr )
            {
            }

            RESULT CopyContext::Create( Backend::IDevice* pDevice, RenderSystem* pSystem )
            {
                BGS_ASSERT( pDevice != nullptr, "Device (pDevice) must be a valid pointer." );
                BGS_ASSERT( pSystem != nullptr, "Render system (pSystem) must be a valid pointer." );
                m_pDevice = pDevice;
                m_pParent = pSystem;

                Backend::QueueDesc qDesc;
                qDesc.priority = Backend::QueuePriorityTypes::HIGH;
                qDesc.type     = Backend::QueueTypes::COPY;
                if( BGS_FAILED( m_pDevice->CreateQueue( qDesc, &m_pQueue ) ) )
                {
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void CopyContext::Destroy()
            {
                if( m_pQueue != nullptr )
                {
                    m_pDevice->DestroyQueue( &m_pQueue );
                }
            }

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS