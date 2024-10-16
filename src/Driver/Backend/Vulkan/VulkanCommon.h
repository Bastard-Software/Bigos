#pragma once

#include "VulkanTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {

            float                  MapBigosQueuePriorityToVulkanQueuePriority( QUEUE_PRIORITY_TYPE prio );
            VkCommandBufferLevel   MapBigosCommandBufferLevelToVulkanCommandBufferLevel( COMMAND_BUFFER_LEVEL lvl );
            VkPrimitiveTopology    MapBigosPrimitiveTopologyToVulkanPrimitiveTopology( PRIMITIVE_TOPOLOGY topo );
            VkVertexInputRate      MapBigosInputStepRateToVulkanVertexInputStepRate( INPUT_STEP_RATE rate );
            VkFormat               MapBigosFormatToVulkanFormat( FORMAT format );
            VkBool32               MapBigosIndexRestartValueToVulkanBool( INDEX_RESTART_VALUE val );
            VkPolygonMode          MapBigosPolygonFillModeToVulkanPolygonMode( POLYGON_FILL_MODE mode );
            VkCullModeFlags        MapBigosCullModeToVulkanCullModeFlags( CULL_MODE mode );
            VkFrontFace            MapBigosFrontFaceModeToVulkanFrontFace( FRONT_FACE_MODE mode );
            VkSampleCountFlagBits  MapBigosSampleCountToVulkanSampleCountFlags( SAMPLE_COUNT cnt );
            VkCompareOp            MapBigosCompareOperationTypeToVulkanCompareOp( COMPARE_OPERATION_TYPE op );
            VkStencilOp            MapBigosStencilOperationTypeToVulkanStencilOp( STENCIL_OPERATION_TYPE op );
            VkLogicOp              MapBigosLogicOperationTypeToVulkanLogicOp( LOGIC_OPERATION_TYPE op );
            VkBlendFactor          MapBigosBlendFactorToVulkanBlendFactor( BLEND_FACTOR factor );
            VkBlendOp              MapBigosBlendOperationTypeToVulkanBlendOp( BLEND_OPERATION_TYPE op );
            VkMemoryPropertyFlags  MapBigosMemoryAccessFlagsToVulkanMemoryPropertFlags( MemoryAccessFlags flags );
            VkBufferUsageFlags     MapBigosResourceUsageToVulkanBufferUsageFlags( ResourceUsageFlags flags );
            VkSharingMode          MapBigosResourceSharingModeToVulkanSharingMode( RESOURCE_SHARING_MODE mode );
            VkImageUsageFlags      MapBigosResourceUsageFlagsToVulkanImageUsageFlags( ResourceUsageFlags flags );
            VkImageType            MapBigosResourceTypeToVulkanImageType( RESOURCE_TYPE type );
            VkImageTiling          MapBigosResourceLayoutToVulkanImageTiling( RESOURCE_LAYOUT layout );
            VkImageAspectFlags     MapBigosTextureComponentFlagsToVulkanImageAspectFlags( TextureComponentFlags flags );
            VkSamplerReductionMode MapBigosSamplerReductionModeToVulkanSamplerReductionMode( SAMPLER_REDUCTION_MODE mode );
            VkFilter               MapBigosFilterTypeToVulkanFilterType( FILTER_TYPE type );
            VkSamplerMipmapMode    MapBigosFilterTypeToVulkanMipMapMode( FILTER_TYPE type );
            VkBorderColor          MapBigosBorderColorToVulkanBorderColor( BORDER_COLOR color );
            VkSamplerAddressMode   MapBigosTextureAddressModeToVultakSamplerAddressMode( TEXTURE_ADDRESS_MODE mode );
            VkDescriptorType       MapBigosBindingTypeToVulkanDescriptorType( BINDING_TYPE type );
            VkShaderStageFlags     MapBigosShaderVisibilityToVulkanShaderStageFlags( SHADER_VISIBILITY vis );
            VkBufferUsageFlags     MapBigosBindingHeapTypeToVulkanBufferUsageFlags( BINDING_HEAP_TYPE type );
            VkQueryType            MapBigosQueryTypeToVulkanQueryType( QUERY_TYPE type );
            VkImageViewType        MapBigosTextureTypeToVulkanImageViewType( TEXTURE_TYPE type );
            VkPipelineBindPoint    MapBigosPipelineTypeToVulkanPipelineBindPoint( PIPELINE_TYPE type );
            VkPipelineStageFlags2  MapBigosPipelineStageFlagsToVulkanPipelineStageFlags( PipelineStageFlags flags );
            VkAccessFlags2         MapBigosAccessFlagsToVulkanAccessFlags( AccessFlags flags );
            VkImageLayout          MapBigosTextureLayoutToVulkanImageLayout( TEXTURE_LAYOUT layout );
            VkIndexType            MapBigosIndexTypeToVulkanIndexType( INDEX_TYPE type );

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS