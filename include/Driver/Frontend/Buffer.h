#pragma once

#include "Driver/Frontend/RenderSystemTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            class BGS_API Buffer final
            {
                friend class RenderSystem;

            public:
                Buffer();
                ~Buffer() = default;

                RESULT Map( void** ppHost );
                void   Unmap();

            protected:
                RESULT Create( const BufferDesc& desc, RenderSystem* pSystem );
                void   Destroy();

            private:
                BufferDesc                  m_desc;
                RenderSystem*               m_pParent;
                Backend::MemoryHandle       m_hMemory;
                Backend::ResourceHandle     m_hResource;
                Backend::ResourceViewHandle m_hConstantAccess;
                Backend::ResourceViewHandle m_hReadAccess;
                Backend::ResourceViewHandle m_hReadWriteAccess;
            };

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS