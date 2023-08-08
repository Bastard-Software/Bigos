#pragma once

#include "Platform/PlatformTypes.h"

#if defined LoadLibrary
#    undef LoadLibrary
#endif

namespace BIGOS
{
    namespace Platform
    {
        BGS_FORCEINLINE LibraryHandle LoadLibrary( const char* pName )
        {
#if BGS_WINDOWS
            return ::LoadLibraryA( pName );
#else
#    error
#endif // BGS_WINDOWS
        }

        BGS_FORCEINLINE void FreeLibrary( LibraryHandle handle )
        {
#if BGS_WINDOWS
            ::FreeLibrary( handle );
#else
#    error
#endif // BGS_WINDOWS
        }

        BGS_FORCEINLINE void* GetProcAddress( LibraryHandle handle, const char* pName )
        {
#if BGS_WINDOWS
            return ::GetProcAddress( handle, pName );
#else
#    error
#endif // BGS_WINDOWS
        }
    } // namespace Platform
} // namespace BIGOS