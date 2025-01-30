#include "Driver/Frontend/RenderDevice.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {
            RESULT RenderDevice::Create( const RenderDeviceDesc& desc, Backend::IFactory* pFactory, RenderSystem* pDriverSystem )
            {
                BGS_ASSERT( pDriverSystem != nullptr, "Render system (pRenderSystem) must be a valid pointer." );
                BGS_ASSERT( pFactory != nullptr, "Factory (pFactory) must be a valid pointer." );

                m_desc     = desc;
                m_pFactory = pFactory;
                m_pParent  = pDriverSystem;

                Backend::DeviceDesc backendDesc;
                backendDesc.pAdapter = m_pFactory->GetAdapters()[ m_desc.adapter.index ];

                Backend::IDevice* pDevice = nullptr;
                if( BGS_FAILED( m_pFactory->CreateDevice( backendDesc, &pDevice ) ) )
                {
                    Results::FAIL;
                }

                m_pAPIDevice = pDevice;

                // TODO: Create hardware queue interfaces
                /*
                QueueDesc qDescs[ 3 ] = {
                    { Backend::QueueTypes::GRAPHICS, Backend::QueuePriorityTypes::NORMAL },
                    { Backend::QueueTypes::COMPUTE, Backend::QueuePriorityTypes::NORMAL },
                    { Backend::QueueTypes::COPY, Backend::QueuePriorityTypes::NORMAL },
                };
                uint32_t qCnt = 3;
                if( BGS_FAILED( CreateQueues( qDescs, qCnt ) ) )
                {
                    m_pFactory->DestroyDevice( &m_pAPIDevice );
                    Results::FAIL;
                }
                */

                return Results::OK;
            }

            void RenderDevice::Destroy()
            {
                if( m_pFactory && m_pAPIDevice )
                {
                    m_pFactory->DestroyDevice( &m_pAPIDevice );
                }

                m_pParent  = nullptr;
                m_pFactory = nullptr;
            }
        } // namespace Frontend
    }     // namespace Driver
} // namespace BIGOS