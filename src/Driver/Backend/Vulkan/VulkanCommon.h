#pragma once

#include "VulkanTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            float                MapBigosQueuePriorityToVulkanQueuePriority( QUEUE_PRIORITY_TYPE prio );
            VkCommandBufferLevel MapBigosCommandBufferLevelToVulkanCommandBufferLevel( COMMAND_BUFFER_LEVEL lvl );
            VkPrimitiveTopology  MapBigosPrimitiveTopologyToVulkanPrimitiveTopology( PRIMITIVE_TOPOLOGY topo );
        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS