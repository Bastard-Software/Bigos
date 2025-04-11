#pragma once

#include "Driver/Frontend/RenderSystemTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {
            struct SyncPoint
            {
                uint64_t    value = MAX_UINT64; // Invalid
                const char* debugName;
            };

            class BGS_API SyncSystem
            {
                friend class RenderSystem;

            public:
                SyncSystem();
                ~SyncSystem() = default;

                SyncPoint GetSyncPoint( const char* pName = nullptr );

                // TODO: Host signaling not supported for now
                RESULT Signal( const SyncPoint& point );

                RESULT Wait( const SyncPoint& point);
                RESULT Wait( const HeapArray<SyncPoint>&& points);

            protected:
                RESULT Create( const SyncSystemDesc& desc, RenderSystem* pSystem );
                void   Destroy();

            private:
                SyncSystemDesc       m_desc;
                RenderSystem*        m_pParent;
                Backend::FenceHandle m_hFence;
                Atomic<uint64_t>     m_counter;
                mutable Mutex        m_mutex;
            };

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS