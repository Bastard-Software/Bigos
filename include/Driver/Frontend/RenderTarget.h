#pragma once

#include "Driver/Frontend/RenderSystemTypes.h"

#include "Driver/Frontend/ResourceState.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            class BGS_API RenderTarget final
            {
                friend class RenderSystem;
                friend class Swapchain;

            public:
                RenderTarget();
                ~RenderTarget() = default;

                bool IsMultisampled() const { return static_cast<uint32_t>( m_targetDesc.sampleCount ) > 1; }

                void                 SetState( const ResourceState& state ) { m_state = state; }
                const ResourceState& GetState() const { return m_state; }
                Size2D               GetSize() const { return { m_desc.width, m_desc.height }; }
                uint32_t             GetWidth() const { return m_desc.width; }
                uint32_t             GetHeight() const { return m_desc.height; }

            protected:
                RESULT Create( const RenderTargetDesc& desc, RenderSystem* pSystem );
                void   Destroy();

                Backend::ResourceHandle GetResource() const { return m_hResource; };

            private:
                RenderTargetDesc            m_desc;
                Backend::ResourceDesc       m_targetDesc;
                ResourceState               m_state;
                RenderSystem*               m_pParent;
                Backend::MemoryHandle       m_hMemory;
                Backend::ResourceHandle     m_hResource;
                Backend::ResourceViewHandle m_hView;
                Backend::ResourceViewHandle m_hSampleView;
                // TODO: Should Render Target store MS resource?
            };
        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS