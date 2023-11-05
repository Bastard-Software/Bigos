#include "Application.h"

#include <chrono>

BIGOS::BigosFramework* Application::m_pFramework = nullptr;

int Application::Run( Sample* pSample )
{
    if( pSample == nullptr )
    {
        printf( "Sample is invalid." );
        return -1;
    }

    BIGOS::BigosFrameworkDesc frameworkDesc;
    if( BGS_FAILED( CreateBigosFramework( frameworkDesc, &m_pFramework ) ) )
    {
        printf( "Framework initialization failed." );
        return -1;
    }

    pSample->OnInit();

    // Renders and display frame for 10 seconds
    for( auto start = std::chrono::steady_clock::now(), now = start; now < start + std::chrono::seconds{ 10 };
         now = std::chrono::steady_clock::now() )
    {
        pSample->OnUpdate();
        pSample->OnRender();
    }

    pSample->OnDestroy();

    DestroyBigosFramework( &m_pFramework );

    return 0;
}
