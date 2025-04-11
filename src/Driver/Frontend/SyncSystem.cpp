#include "Driver/Frontend/SyncSystem.h"

#include "Driver/Frontend/RenderSystem.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {
            SyncSystem::SyncSystem()
                : m_desc()
                , m_pParent( nullptr )
                , m_hFence()
                , m_counter( 0 )
                , m_mutex()
            {
            }

            SyncPoint SyncSystem::GetSyncPoint( const char* pName )
            {
                SyncPoint point;
                point.value     = m_counter.fetch_add( 1 );
                point.debugName = pName;

                return point;
            }

            RESULT SyncSystem::Signal( const SyncPoint& point )
            {
                BGS_ASSERT( "false" );
                return Results::FAIL;
            }

            RESULT SyncSystem::Wait( const SyncPoint& point )
            {
                return Wait( { point } );
            }

            RESULT SyncSystem::Wait( const HeapArray<SyncPoint>&& points )
            {
                uint64_t waitValue = 0;
                for( index_t ndx = 0; points.size(); ++ndx )
                {
                }
            }

            RESULT SyncSystem::Create( const SyncSystemDesc& desc, RenderSystem* pSystem )
            {
                BGS_ASSERT( pSystem != nullptr, "Render system (pSystem) must be a valid pointer." );
                m_pParent                    = pSystem;
                m_desc                       = desc;
                Backend::IDevice* pAPIDevice = m_pParent->GetDevice();

                Backend::FenceDesc fenceDesc;
                fenceDesc.initialValue = 0;
                if( BGS_FAILED( pAPIDevice->CreateFence( fenceDesc, &m_hFence ) ) )
                {
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void SyncSystem::Destroy()
            {
                Backend::IDevice* pAPIDevice = m_pParent->GetDevice();
                if( m_hFence != Backend::FenceHandle() )
                {
                    pAPIDevice->DestroyFence( &m_hFence );
                }
            }
        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS