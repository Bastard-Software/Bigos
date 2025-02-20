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
                friend class RenderDevice;

            public:
                Texture();
                ~Texture() = default;

            protected:
                RESULT Create( const TextureDesc& desc, RenderDevice* pDevice );
                void   Destroy();

            private:
                TextureDesc                 m_desc;
                RenderDevice*               m_pParent;
                Backend::MemoryHandle       m_hMemory;
                Backend::ResourceHandle     m_hResource;
                Backend::ResourceViewHandle m_hSampleAccess;
                Backend::ResourceViewHandle m_hStorageAccess;
            };

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS