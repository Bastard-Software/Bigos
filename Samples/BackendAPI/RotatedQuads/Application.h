#pragma once

#include "BigosFramework/BigosFramework.h"

#include "Sample.h"

class Application
{
public:
    static int Run( Sample* pSample );

    static BIGOS::BigosFramework* GetFramework() { return m_pFramework; }

private:
    static BIGOS::BigosFramework* m_pFramework;
};