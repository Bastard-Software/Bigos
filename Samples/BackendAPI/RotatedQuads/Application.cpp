#include "Application.h"

#include <chrono>

BIGOS::BigosEngine* Application::m_pFramework = nullptr;

int Application::Run( Sample* pSample )
{
    if( pSample == nullptr )
    {
        printf( "Sample is invalid." );
        return -1;
    }

    BIGOS::BigosEngineDesc frameworkDesc;
    if( BGS_FAILED( CreateBigosEngine( frameworkDesc, &m_pFramework ) ) )
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

    DestroyBigosEngine( &m_pFramework );

    return 0;
}
