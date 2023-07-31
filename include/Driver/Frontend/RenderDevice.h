#pragma once

#include "Driver/Frontend/RenderSystemTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            class BGS_API RenderDevice final
            {
                friend class RenderSystem;

            public:
            protected:
                RESULT Create( const RenderDeviceDesc& desc, Backend::IFactory* pFactory, RenderSystem* pDriverSystem );
                void   Destroy();

            private:
                RESULT CreateQueues( QueueDesc* descs, uint32_t descCount );
                void   DestroyQueues();

            private:
                RenderDeviceDesc   m_desc;
                Backend::IFactory* m_pFactory = nullptr;
                RenderSystem*      m_pParent  = nullptr;
                Backend::IDevice*  m_pDevice  = nullptr;
                QueueArray         m_graphicsQueues;
                QueueArray         m_computeQueues;
                QueueArray         m_copyQueues;
            };

        } // namespace Frontend
    }     // namespace Driver
} // namespace BIGOS