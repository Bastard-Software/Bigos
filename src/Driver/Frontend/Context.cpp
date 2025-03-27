#include "Driver/Frontend/Context.h"

#include "Driver/Backend/API.h"
#include "Driver/Frontend/RenderSystem.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            GraphicsContext::GraphicsContext()
                : m_pQueue( nullptr )
            {
            }

            RESULT GraphicsContext::Create( Backend::IDevice* pDevice, RenderSystem* pSystem )
            {
                pDevice;
                pSystem;
                return Results::OK;
            }

            void GraphicsContext::Destroy()
            {
            }

            ComputeContext::ComputeContext()
                : m_pQueue( nullptr )
            {
            }

            RESULT ComputeContext::Create( Backend::IDevice* pDevice, RenderSystem* pSystem )
            {
                pDevice;
                pSystem;
                return Results::OK;
            }

            void ComputeContext::Destroy()
            {
            }

            CopyContext::CopyContext()
                : m_pQueue( nullptr )
            {
            }

            RESULT CopyContext::Create( Backend::IDevice* pDevice, RenderSystem* pSystem )
            {
                pDevice;
                pSystem;
                return Results::OK;
            }

            void CopyContext::Destroy()
            {
            }

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS