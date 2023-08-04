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

            VkCommandBufferLevel MapBigosCommandBufferLevelToVulkanCommandBufferLevel( COMMAND_BUFFER_LEVEL lvl )
            {
                static const VkCommandBufferLevel translateTable[ BGS_ENUM_COUNT( CommandBufferLevels ) ] = {
                    VK_COMMAND_BUFFER_LEVEL_PRIMARY,  // PRIMARY
                    VK_COMMAND_BUFFER_LEVEL_SECONDARY // SECONDARY
                };

                return translateTable[ BGS_ENUM_INDEX( lvl ) ];
            }

            VkPrimitiveTopology MapBigosPrimitiveTopologyToVulkanPrimitiveTopology( PRIMITIVE_TOPOLOGY topo )
            {
                static const VkPrimitiveTopology translateTable[ BGS_ENUM_COUNT( PrimitiveTopologies ) ] = {
                    VK_PRIMITIVE_TOPOLOGY_POINT_LIST,                    // POINT_LIST
                    VK_PRIMITIVE_TOPOLOGY_LINE_LIST,                     // LINE_LIST
                    VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,                    // LINE_STRIP
                    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,                 // TRIANGLE_LIST
                    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,                // TRIANGLE_STRIP
                    VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,      // LINE_LIST_WITH_ADJACENCY
                    VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY,     // LINE_STRIP_WITH_ADJACENCY
                    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,  // TRIANGLE_LIST_WITH_ADJACENCY
                    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY, // TRIANGLE_STRIP_WITH_ADJACENCY
                    VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,                    // PATCH_LIST
                };

                return translateTable[ BGS_ENUM_INDEX( topo ) ];
            }
        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS