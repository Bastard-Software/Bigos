#pragma once

#include <BigosEngine/Layer.h>
#include <Core/Utils/Timestep.h>
#include <Platform/Event/IEvent.h>

class DemoLayer : public BIGOS::Layer
{
public:
    DemoLayer();
    virtual ~DemoLayer() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate( BIGOS::Core::Utils::Timestep ) override;
    virtual void OnImGuiRender() override;
};