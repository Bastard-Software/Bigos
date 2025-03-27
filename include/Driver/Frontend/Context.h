#pragma once
#include "Core/CoreTypes.h"
#include "Driver/Backend/APITypes.h"
#include "Driver/Frontend/RenderSystemTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            class BGS_API GraphicsContext final
            {
                friend class RenderSystem;
                friend class Swapchain;

            public:
                GraphicsContext();
                ~GraphicsContext() = default;

            protected:
                RESULT Create( Backend::IDevice* pDevice, RenderSystem* m_pSystem );
                void   Destroy();

                Backend::IQueue* GetQueue() { return m_pQueue; }

            private:
                Backend::IQueue* m_pQueue;
            };

            class BGS_API ComputeContext final
            {
                friend class RenderSystem;

            public:
                ComputeContext();
                ~ComputeContext() = default;

            protected:
                RESULT Create( Backend::IDevice* pDevice, RenderSystem* m_pSystem );
                void   Destroy();

                Backend::IQueue* GetQueue() { return m_pQueue; }

            private:
                Backend::IQueue* m_pQueue;
            };

            class BGS_API CopyContext final
            {
                friend class RenderSystem;

            public:
                CopyContext();
                ~CopyContext() = default;

            protected:
                RESULT Create( Backend::IDevice* pDevice, RenderSystem* m_pSystem );
                void   Destroy();

                Backend::IQueue* GetQueue() { return m_pQueue; }

            private:
                Backend::IQueue* m_pQueue;
            };

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS