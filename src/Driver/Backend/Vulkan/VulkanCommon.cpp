#include "VulkanCommon.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            float MapBigosQueuePriorityToVulkanQueuePriority( QUEUE_PRIORITY_TYPE prio )
            {
                static const float translateTable[ BGS_ENUM_COUNT( QueuePriorityTypes ) ] = {
                    0.0f, // NORMAL
                    0.5f, // HIGH
                    1.0f, // REALTIME
                };

                return translateTable[ BGS_ENUM_INDEX( prio ) ];
            }
        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS