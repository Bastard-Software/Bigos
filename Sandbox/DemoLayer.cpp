#include "DemoLayer.h"

DemoLayer::DemoLayer()
    : BIGOS::Layer( "DemoLayer" )
{
}

void DemoLayer::OnAttach()

{
}

void DemoLayer::OnDetach()
{
}

void DemoLayer::OnUpdate( BIGOS::Core::Utils::Timestep )
{
    printf( "CWEL\n" );
}

void DemoLayer::OnImGuiRender()
{
}
