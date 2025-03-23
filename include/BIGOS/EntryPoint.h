#pragma once

#include "BIGOS/Application.h"

extern BIGOS::IApplication* BIGOS::CreateApplication();

int main( int, char** )
{
    BIGOS::IApplication* app = BIGOS::CreateApplication();
    app->Create();
    app->Run();
    app->Destroy();

    delete app;

    return 0;
}