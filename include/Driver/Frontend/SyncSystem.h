#pragma once

#include "Driver/Frontend/RenderSystemTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {
            class SyncPoint
            {
                friend class SyncSystem;

            public:
                SyncPoint()
                    : m_value( 0 )
                    , m_contextType( CONTEXT_TYPE::_MAX_ENUM )
                    , m_pDebugName( nullptr )
                {
                }

                uint64_t     GetValue() const { return m_value; }
                CONTEXT_TYPE GetContextType() const { return m_contextType; }
                const char*  GetDebugName() const { return m_pDebugName; }

            protected:
                SyncPoint( uint64_t value, CONTEXT_TYPE ctxType, const char* pName )
                    : m_value( value )
                    , m_contextType( ctxType )
                    , m_pDebugName( pName )
                {
                }

            private:
                uint64_t     m_value;
                const char*  m_pDebugName;
                CONTEXT_TYPE m_contextType;
            };

            class BGS_API SyncSystem
            {
                friend class RenderSystem;

            public:
                SyncSystem();
                ~SyncSystem() = default;

                SyncPoint CreateSyncPoint( CONTEXT_TYPE ctxType, const char* pName = nullptr );

                RESULT Signal( const SyncPoint& point );

                RESULT Wait( const SyncPoint& point );
                RESULT Wait( const HeapArray<SyncPoint>& points );

            protected:
                RESULT Create( const SyncSystemDesc& desc, RenderSystem* pSystem );
                void   Destroy();

            private:
                void CleanupCompletedPoints( CONTEXT_TYPE ctxType );

            private:
                struct PerContextData
                {
                    Backend::FenceHandle hFence;
                    Atomic<uint64_t>     counter;
                    HeapArray<SyncPoint> syncPoints;
                    Mutex                mutex;
                };

            private:
                SyncSystemDesc                                             m_desc;
                RenderSystem*                                              m_pParent;
                StackArray<PerContextData, BGS_ENUM_COUNT( ContextTypes )> m_contextData;
            };

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS