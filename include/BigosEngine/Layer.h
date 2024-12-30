#pragma once

#include "Core/Utils/Timestep.h"

namespace BIGOS
{
    class BGS_API Layer
    {
    public:
        Layer( const char* pName = "Layer" );
        virtual ~Layer() = default;

        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnUpdate( Core::Utils::Timestep ) {}
        virtual void OnImGuiRender() {}

    protected:
        const char* m_pDebugName;
    };
} // namespace BIGOS