#pragma once
#include "Core/CoreTypes.h"

namespace BIGOS::Driver::Backend
{
    BGS_DECLARE_HANDLE( Adapter );
    BGS_DECLARE_HANDLE( Factory );
    BGS_DECLARE_HANDLE( Device );
    BGS_DECLARE_HANDLE( Queue );
    BGS_DECLARE_HANDLE( Swapchain );
    BGS_DECLARE_HANDLE( CommandPool );
    BGS_DECLARE_HANDLE( CommandBuffer );
    BGS_DECLARE_HANDLE( Shader );
    BGS_DECLARE_HANDLE( PipelineLayout );
    BGS_DECLARE_HANDLE( Pipeline );
    BGS_DECLARE_HANDLE( Fence );
    BGS_DECLARE_HANDLE( Semaphore );
    BGS_DECLARE_HANDLE( Memory );
    BGS_DECLARE_HANDLE( Resource );
    BGS_DECLARE_HANDLE( ResourceView );
    BGS_DECLARE_HANDLE( Sampler );
    BGS_DECLARE_HANDLE( BindingSetLayout );
    BGS_DECLARE_HANDLE( BindingHeap );
    BGS_DECLARE_HANDLE( QueryPool );
    BGS_DECLARE_HANDLE( CommandLayout );
} // namespace BIGOS::Driver::Backend