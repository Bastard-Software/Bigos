#pragma once

#include "BigosEngine/Application.h"

extern BIGOS::Application* BIGOS::CreateApplication();

BIGOS::Application*    BIGOS::Application::s_pInstance  = nullptr;
BIGOS::BigosFramework* BIGOS::Application::s_pFramework = nullptr;

int main( int, char** )
{
    BIGOS::Application* app = BIGOS::CreateApplication();
    app->Create();
    app->Run();
    app->Destroy();

    delete app;

    return 0;
}