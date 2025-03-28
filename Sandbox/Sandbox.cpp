#include <BIGOS/Application.h>
#include <BIGOS/BIGOS.h>
#include <BIGOS/EntryPoint.h>

class Sandbox : public BIGOS::WindowedApplication
{
public:
    Sandbox()
        : WindowedApplication( "Sandbox", BIGOS::Driver::Backend::APITypes::D3D12 )
    {
    }

    ~Sandbox() {}

    virtual BIGOS::RESULT OnCreate() override;
    virtual void          OnUpdate( BIGOS::Utils::Timestep ts ) override;
    virtual void          OnRender( BIGOS::Utils::Timestep ts ) override;
    virtual void          OnDestroy() override;
};

BIGOS::IApplication* BIGOS::CreateApplication()
{
    return new Sandbox();
}

BIGOS::RESULT Sandbox::OnCreate()
{
    printf( "Startup\n" );
    return BIGOS::Results::OK;
}

void Sandbox::OnUpdate( BIGOS::Utils::Timestep ts )
{
    ts;
}

void Sandbox::OnRender( BIGOS::Utils::Timestep ts )
{
    ts;
    m_renderer.Render();
}

void Sandbox::OnDestroy()
{
    printf( "Shutdown\n" );
}
