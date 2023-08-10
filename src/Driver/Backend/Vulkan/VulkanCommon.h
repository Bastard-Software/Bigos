#pragma once

#include "VulkanTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {

            float                 MapBigosQueuePriorityToVulkanQueuePriority( QUEUE_PRIORITY_TYPE prio );
            VkCommandBufferLevel  MapBigosCommandBufferLevelToVulkanCommandBufferLevel( COMMAND_BUFFER_LEVEL lvl );
            VkPrimitiveTopology   MapBigosPrimitiveTopologyToVulkanPrimitiveTopology( PRIMITIVE_TOPOLOGY topo );
            VkVertexInputRate     MapBigosInputStepRateToVulkanVertexInputStepRate( INPUT_STEP_RATE rate );
            VkFormat              MapBigosFormatToVulkanFormat( FORMAT format );
            VkBool32              MapBigosIndexRestartValueToVulkanBool( INDEX_RESTART_VALUE val );
            VkPolygonMode         MapBigosPolygonFillModeToVulkanPolygonMode( POLYGON_FILL_MODE mode );
            VkCullModeFlags       MapBigosCullModeToVulkanCullModeFlags( CULL_MODE mode );
            VkFrontFace           MapBigosFrontFaceModeToVulkanFrontFace( FRONT_FACE_MODE mode );
            VkSampleCountFlagBits MapBigosSampleCountToVulkanSampleCountFlags( SAMPLE_COUNT cnt );
            VkCompareOp           MapBigosCompareOperationTypeToVulkanCompareOp( COMPARE_OPERATION_TYPE op );
            VkStencilOp           MapBigosStencilOperationTypeToVulkanStencilOp( STENCIL_OPERATION_TYPE op );
            VkLogicOp             MapBigosLogicOperationTypeToVulkanLogicOp( LOGIC_OPERATION_TYPE op );
            VkBlendFactor         MapBigosBlendFactorToVulkanBlendFactor( BLEND_FACTOR factor );
            VkBlendOp             MapBigosBlendOperationTypeToVulkanBlendOp( BLEND_OPERATION_TYPE op );
            VkMemoryPropertyFlags MapBigosMemoryAccessFlagsToVulkanMemoryPropertFlags( MemoryAccessFlags flags );

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS