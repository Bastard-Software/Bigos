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

    while( pSample->IsRunning() )
    {
        pSample->OnUpdate();
        pSample->OnRender();
    }

    pSample->OnDestroy();

    DestroyBigosFramework( &m_pFramework );

    return 0;
}
