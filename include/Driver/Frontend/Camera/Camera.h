#pragma once

#include "Core/CoreTypes.h"
#include "Driver/Frontend/RenderSystemTypes.h"

#include "glm/glm.hpp"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {
            class BGS_API Camera
            {
                friend class RenderSystem;

            public:
                Camera()          = default;
                virtual ~Camera() = default;

                const glm::mat4& GetProjection() const { return m_projection; }

            protected:
                RESULT Create( const CameraDesc& desc, RenderSystem* pRenderSystem );
                void   Destroy();

            protected:
                glm::mat4     m_projection;
                RenderSystem* m_pParent;
            };
        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS