#include <BigosEngine/Application.h>
#include <BigosEngine/EntryPoint.h>

#include "DemoLayer.h"

class Sandbox : public BIGOS::Application
{
public:
    Sandbox()
        : Application( "Sandbox" )
    {
        PushLayer( new DemoLayer() );
    }

    ~Sandbox() {}
};

BIGOS::Application* BIGOS::CreateApplication()
{
    return new Sandbox();
}