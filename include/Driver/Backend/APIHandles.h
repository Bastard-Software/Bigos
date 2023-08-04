#pragma once
#include "Core/CoreTypes.h"

namespace BIGOS::Driver::Backend
{
    BGS_DECLARE_HANDLE( Adapter );
    BGS_DECLARE_HANDLE( Factory );
    BGS_DECLARE_HANDLE( Device );
    BGS_DECLARE_HANDLE( Queue );
    BGS_DECLARE_HANDLE( CommandPool );
    BGS_DECLARE_HANDLE( CommandBuffer );
    BGS_DECLARE_HANDLE( Pipeline );
    BGS_DECLARE_HANDLE( Fence );
    BGS_DECLARE_HANDLE( Semaphore );
} // namespace BIGOS::Driver::Backend