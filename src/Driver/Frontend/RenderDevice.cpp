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

                m_pDevice = pDevice;

                // TODO: Temporary solution
                // Creating hardware queues
                QueueDesc qDescs[ 3 ] = {
                    { Backend::QueueTypes::GRAPHICS, Backend::QueuePriorityTypes::NORMAL },
                    { Backend::QueueTypes::COMPUTE, Backend::QueuePriorityTypes::NORMAL },
                    { Backend::QueueTypes::COPY, Backend::QueuePriorityTypes::NORMAL },
                };
                uint32_t qCnt = 3;
                if( BGS_FAILED( CreateQueues( qDescs, qCnt ) ) )
                {
                    m_pFactory->DestroyDevice( &m_pDevice );
                    Results::FAIL;
                }

                return Results::OK;
            }

            void RenderDevice::Destroy()
            {
                DestroyQueues();
                if( m_pFactory && m_pDevice )
                {
                    m_pFactory->DestroyDevice( &m_pDevice );
                }

                m_pParent  = nullptr;
                m_pFactory = nullptr;
            }

            RESULT RenderDevice::CreateQueues( QueueDesc* descs, uint32_t descCount )
            {
                BGS_ASSERT( descs != nullptr, "" );
                BGS_ASSERT( descCount != 0, "" );
                BGS_ASSERT( m_pDevice != nullptr, "" );

                const index_t qCnt = static_cast<index_t>( descCount );
                for( index_t ndx = 0; ndx < qCnt; ++ndx )
                {
                    const QueueDesc& desc   = descs[ ndx ];
                    Backend::IQueue* pQueue = nullptr;
                    if( BGS_FAILED( m_pDevice->CreateQueue( desc, &pQueue ) ) )
                    {
                        // TODO: LOg which creation failed
                        return Results::FAIL;
                    }

                    if( desc.type == Backend::QueueTypes::GRAPHICS )
                    {
                        m_graphicsQueues.push_back( pQueue );
                    }
                    else if( desc.type == Backend::QueueTypes::COMPUTE )
                    {
                        m_computeQueues.push_back( pQueue );
                    }
                    else // desc.type == Backend::QueueTypes::COPY
                    {
                        m_copyQueues.push_back( pQueue );
                    }
                }

                return Results::OK;
            }

            void RenderDevice::DestroyQueues()
            {
                for( index_t ndx = 0; ndx < m_graphicsQueues.size(); ++ndx )
                {
                    m_pDevice->DestroyQueue( &m_graphicsQueues[ ndx ] );
                }

                for( index_t ndx = 0; ndx < m_computeQueues.size(); ++ndx )
                {
                    m_pDevice->DestroyQueue( &m_computeQueues[ ndx ] );
                }

                for( index_t ndx = 0; ndx < m_copyQueues.size(); ++ndx )
                {
                    m_pDevice->DestroyQueue( &m_copyQueues[ ndx ] );
                }
            }

        } // namespace Frontend
    }     // namespace Driver
} // namespace BIGOS