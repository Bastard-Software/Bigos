#include "DemoLayer.h"
#include <BigosEngine/Application.h>
#include <BigosEngine/EntryPoint.h>

class Sandbox : public BIGOS::Application
{
public:
    Sandbox()
        : Application( "Sandbox", BIGOS::Driver::Backend::APITypes::VULKAN )
    {
        PushLayer( new DemoLayer() );
    }

    ~Sandbox() {}
};

BIGOS::Application* BIGOS::CreateApplication()
{
    return new Sandbox();
}