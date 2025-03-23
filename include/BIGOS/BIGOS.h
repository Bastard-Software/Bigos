#pragma once

#include "Core/CoreTypes.h"

#include "BIGOS/BigosEngine.h"

namespace BIGOS
{
    class BigosEngine;

    extern BigosEngine* g_pEngine = nullptr;

    BGS_FORCEINLINE BGS_API void AlocateGlobals()
    {
        g_pEngine = new BigosEngine();
    }

    BGS_FORCEINLINE BGS_API void FreeGlobals()
    {
        delete g_pEngine;
    }

} // namespace BIGOS