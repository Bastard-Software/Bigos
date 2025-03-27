#include "Driver/Frontend/RenderPass.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {
            RenderPass::RenderPass()
                : m_desc()
                , m_pParent( nullptr )
            {
            }

            RESULT RenderPass::Create( const RenderPassDesc& desc, RenderSystem* pSystem )
            {
                BGS_ASSERT( pSystem != nullptr, "Render system (pSystem) must be a valid pointer." );
                m_pParent = pSystem;
                m_desc    = desc;

                return Results::OK;
            }

            void RenderPass::Destroy()
            {
                // TODO: Implement
            }

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS