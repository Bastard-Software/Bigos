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
                , m_contextData()
            {
            }

            SyncPoint SyncSystem::CreateSyncPoint( CONTEXT_TYPE ctxType, const char* pName )
            {
                PerContextData&        contextData = m_contextData[ BGS_ENUM_INDEX( ctxType ) ];
                std::lock_guard<Mutex> lock( contextData.mutex );

                uint64_t  value = contextData.counter.fetch_add( 1 );
                SyncPoint point( value, ctxType, pName );
                contextData.syncPoints.emplace_back( point );

                return contextData.syncPoints.back();
            }

            RESULT SyncSystem::Signal( const SyncPoint& point )
            {
                PerContextData&        contextData = m_contextData[ BGS_ENUM_INDEX( point.GetContextType() ) ];
                std::lock_guard<Mutex> lock( contextData.mutex );

                return m_pParent->GetDevice()->SignalFence( point.GetValue(), contextData.hFence );
            }

            RESULT SyncSystem::Wait( const SyncPoint& point )
            {
                PerContextData& contextData = m_contextData[ BGS_ENUM_INDEX( point.GetContextType() ) ];
                uint64_t        waitVal     = point.GetValue();

                Backend::WaitForFencesDesc waitDesc;
                waitDesc.fenceCount  = 1;
                waitDesc.pFences     = &contextData.hFence;
                waitDesc.pWaitValues = &waitVal;
                waitDesc.waitAll     = BGS_TRUE;
                return m_pParent->GetDevice()->WaitForFences( waitDesc, MAX_UINT64 );
            }

            RESULT SyncSystem::Wait( const HeapArray<SyncPoint>& points )
            {
                StackArray<uint64_t, BGS_ENUM_COUNT( ContextTypes )> maxVals   = { 0, 0, 0 };
                StackArray<bool_t, BGS_ENUM_COUNT( ContextTypes )>   hasPoints = { BGS_FALSE, BGS_FALSE, BGS_FALSE };

                // Getting max values for each context type
                for( index_t ndx = 0; ndx < points.size(); ++ndx )
                {
                    const SyncPoint& point   = points[ ndx ];
                    CONTEXT_TYPE     ctxType = point.GetContextType();
                    if( point.GetValue() > maxVals[ BGS_ENUM_INDEX( ctxType ) ] )
                    {
                        maxVals[ BGS_ENUM_INDEX( ctxType ) ]   = point.GetValue();
                        hasPoints[ BGS_ENUM_INDEX( ctxType ) ] = BGS_TRUE;
                    }
                }

                StackArray<Backend::FenceHandle, BGS_ENUM_COUNT( ContextTypes )> fences;
                StackArray<uint64_t, BGS_ENUM_COUNT( ContextTypes )>             vals;
                uint32_t                                                         cnt = 0;

                for( index_t ndx = 0; ndx < BGS_ENUM_COUNT( ContextTypes ); ++ndx )
                {
                    if( hasPoints[ ndx ] )
                    {
                        fences[ cnt ] = m_contextData[ ndx ].hFence;
                        vals[ cnt ]   = maxVals[ ndx ];
                        ++cnt;
                    }
                }

                if( cnt == 0 )
                {
                    return Results::OK;
                }

                Backend::WaitForFencesDesc waitDesc;
                waitDesc.fenceCount  = cnt;
                waitDesc.pFences     = fences.data();
                waitDesc.pWaitValues = vals.data();
                waitDesc.waitAll     = BGS_TRUE;
                RESULT res           = m_pParent->GetDevice()->WaitForFences( waitDesc, MAX_UINT64 );
                if( BGS_SUCCESS( res ) )
                {
                    for( index_t ndx = 0; ndx < BGS_ENUM_COUNT( ContextTypes ); ++ndx )
                    {
                        if( hasPoints[ ndx ] )
                        {
                            CleanupCompletedPoints( static_cast<CONTEXT_TYPE>( ndx ) );
                        }
                    }
                }

                return res;
            }

            RESULT SyncSystem::Create( const SyncSystemDesc& desc, RenderSystem* pSystem )
            {
                BGS_ASSERT( pSystem != nullptr, "Render system (pSystem) must be a valid pointer." );
                m_pParent                    = pSystem;
                m_desc                       = desc;
                Backend::IDevice* pAPIDevice = m_pParent->GetDevice();

                Backend::FenceDesc fenceDesc;
                fenceDesc.initialValue = 0;
                for( index_t ndx = 0; ndx < m_contextData.size(); ++ndx )
                {
                    PerContextData& contextData = m_contextData[ ndx ];
                    contextData.hFence          = Backend::FenceHandle();
                    contextData.syncPoints.reserve( m_desc.syncPointCount );
                    contextData.counter.store( 1 );
                    if( BGS_FAILED( pAPIDevice->CreateFence( fenceDesc, &contextData.hFence ) ) )
                    {
                        Destroy();
                        return Results::FAIL;
                    }
                }

                return Results::OK;
            }

            void SyncSystem::Destroy()
            {
                // Wait for all sync points to be signaled and destroy fence
                for( index_t ndx = 0; ndx < m_contextData.size(); ++ndx )
                {
                    PerContextData& contextData = m_contextData[ ndx ];
                    Wait( contextData.syncPoints );
                    if( contextData.hFence != Backend::FenceHandle() )
                    {
                        m_pParent->GetDevice()->DestroyFence( &contextData.hFence );
                    }
                }
            }

            void SyncSystem::CleanupCompletedPoints( CONTEXT_TYPE ctxType )
            {
                PerContextData&        contextData = m_contextData[ BGS_ENUM_INDEX( ctxType ) ];
                std::lock_guard<Mutex> lock( contextData.mutex );

                uint64_t fenceVal = 0;
                m_pParent->GetDevice()->GetFenceValue( contextData.hFence, &fenceVal );

                SyncPoint* pPoints = contextData.syncPoints.data();
                index_t    cnt     = contextData.syncPoints.size();
                uint32_t   newCnt  = 0;
                for( index_t ndx = 0; ndx < cnt; ++ndx )
                {
                    if( pPoints[ ndx ].GetValue() > fenceVal )
                    {
                        if( newCnt != ndx )
                        {
                            pPoints[ newCnt ] = pPoints[ ndx ];
                        }
                        newCnt++;
                    }
                }

                contextData.syncPoints.resize( newCnt );
            }

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS