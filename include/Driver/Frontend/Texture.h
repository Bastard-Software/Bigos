#pragma once

#include "Driver/Frontend/RenderSystemTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            class BGS_API Texture final
            {
                friend class RenderSystem;

            public:
                Texture();
                ~Texture() = default;

            protected:
                RESULT Create( const TextureDesc& desc, RenderSystem* pSystem );
                void   Destroy();

            private:
                TextureDesc                 m_desc;
                RenderSystem*               m_pParent;
                Backend::MemoryHandle       m_hMemory;
                Backend::ResourceHandle     m_hResource;
                Backend::ResourceViewHandle m_hSampleAccess;
                Backend::ResourceViewHandle m_hStorageAccess;
            };

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS