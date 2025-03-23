#pragma once

#include "BIGOS/BigosEngine.h"
#include "Sample.h"

class Application
{
public:
    static int Run( Sample* pSample );

    static BIGOS::BigosEngine* GetFramework() { return m_pFramework; }

private:
    static BIGOS::BigosEngine* m_pFramework;
};