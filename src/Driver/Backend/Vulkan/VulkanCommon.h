#pragma once

#include "VulkanTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            float MapBigosQueuePriorityToVulkanQueuePriority( QUEUE_PRIORITY_TYPE prio );
        }
    } // namespace Driver
} // namespace BIGOS