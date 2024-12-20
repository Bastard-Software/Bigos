#include "Driver/Frontend/Camera/Camera.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {
            RESULT Camera::Create( const CameraDesc& desc, RenderSystem* pRenderSystem )
            {
                BGS_ASSERT( pRenderSystem != nullptr, "Render system (pRenderSystem) must be a valid pointer." );

                m_pParent    = pRenderSystem;
                m_projection = desc.projection;

                return Results::OK;
            }

            void Camera::Destroy()
            {
            }
        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS