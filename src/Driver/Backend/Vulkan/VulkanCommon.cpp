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

            VkVertexInputRate MapBigosInputStepRateToVulkanVertexInputStepRate( INPUT_STEP_RATE rate )
            {
                static const VkVertexInputRate translateTable[ BGS_ENUM_COUNT( InputStepRates ) ] = {
                    VK_VERTEX_INPUT_RATE_VERTEX,   // PER_VERTEX
                    VK_VERTEX_INPUT_RATE_INSTANCE, // PER_INSTANCE
                };

                return translateTable[ BGS_ENUM_INDEX( rate ) ];
            }

            VkFormat MapBigosFormatToVulkanFormat( FORMAT format )
            {
                static const VkFormat translateTable[ BGS_ENUM_COUNT( Formats ) ] = {
                    VK_FORMAT_UNDEFINED,             // UNKNOWN
                    VK_FORMAT_B5G5R5A1_UNORM_PACK16, // B5G5R5A1_UNORM
                    VK_FORMAT_B5G6R5_UNORM_PACK16,   // B5G6R5_UNORM
                    VK_FORMAT_B8G8R8A8_UNORM,        // B8G8R8A8_UNORM
                    VK_FORMAT_B8G8R8A8_SRGB,         // B8G8R8A8_UNORM_SRGB
                    VK_FORMAT_BC1_RGBA_UNORM_BLOCK,  // BC1_UNORM
                    VK_FORMAT_BC1_RGBA_SRGB_BLOCK,   // BC1_UNORM_SRGB
                    VK_FORMAT_BC2_UNORM_BLOCK,       // BC2_UNORM
                    VK_FORMAT_BC2_SRGB_BLOCK,        // BC2_UNORM_SRGB
                    VK_FORMAT_BC3_UNORM_BLOCK,       // BC3_UNORM
                    VK_FORMAT_BC3_SRGB_BLOCK,        // BC3_UNORM_SRGB
                    VK_FORMAT_BC4_UNORM_BLOCK,       // BC4_UNORM
                    VK_FORMAT_BC4_SNORM_BLOCK,       // BC4_SNORM
                    VK_FORMAT_BC5_UNORM_BLOCK,       // BC5_UNORM
                    VK_FORMAT_BC5_SNORM_BLOCK,       // BC5_SNORM
                    VK_FORMAT_BC6H_UFLOAT_BLOCK,     // BC6H_UFLOAT
                    VK_FORMAT_BC6H_SFLOAT_BLOCK,     // BC6H_SFLOAT
                    VK_FORMAT_BC7_UNORM_BLOCK,       // BC7_UNORM
                    VK_FORMAT_BC7_SRGB_BLOCK,        // BC7_UNORM_SRGB
                    VK_FORMAT_R8_UINT,               // R8_UINT
                    VK_FORMAT_R8_SINT,               // R8_SINT
                    VK_FORMAT_R8_UNORM,              // R8_UNORM
                    VK_FORMAT_R8_SNORM,              // R8_SNORM
                    VK_FORMAT_R16_UINT,              // R16_UINT
                    VK_FORMAT_R16_SINT,              // R16_SINT
                    VK_FORMAT_R16_UNORM,             // R16_UNORM
                    VK_FORMAT_R16_SNORM,             // R16_SNORM
                    VK_FORMAT_R16_SFLOAT,            // R16_FLOAT
                    VK_FORMAT_D16_UNORM,             // D16_UNORM
                    VK_FORMAT_R8G8_UINT,             // R8G8_UINT
                    VK_FORMAT_R8G8_SINT,             // R8G8_SINT
                    VK_FORMAT_R8G8_UNORM,            // R8G8_UNORM
                    VK_FORMAT_R8G8_SNORM,            // R8G8_SNORM
                    VK_FORMAT_D24_UNORM_S8_UINT,     // D24_UNORM_S8_UINT
                    VK_FORMAT_R32_SFLOAT,            // R32_FLOAT
                    VK_FORMAT_R32_UINT,              // R32_UINT
                    VK_FORMAT_R32_SINT,              // R32_SINT
                    VK_FORMAT_D32_SFLOAT,            // D32_FLOAT
                    VK_FORMAT_R16G16_UINT,           // R16G16_UINT
                    VK_FORMAT_R16G16_SINT,           // R16G16_SINT
                    VK_FORMAT_R16G16_UNORM,          // R16G16_UNORM
                    VK_FORMAT_R16G16_SNORM,          // R16G16_SNORM
                    VK_FORMAT_R16G16_SFLOAT,         // R16G16_FLOAT
                    VK_FORMAT_R8G8B8A8_UNORM,        // R8G8B8A8_UNORM
                    VK_FORMAT_R8G8B8A8_SNORM,        // R8G8B8A8_SNORM
                    VK_FORMAT_R8G8B8A8_SRGB,         // R8G8B8A8_UNORM_SRGB
                    VK_FORMAT_R8G8B8A8_UINT,         // R8G8B8A8_UINT
                    VK_FORMAT_R8G8B8A8_SINT,         // R8G8B8A8_SINT
                    VK_FORMAT_R32G32_UINT,           // R32G32_UINT
                    VK_FORMAT_R32G32_SINT,           // R32G32_SINT
                    VK_FORMAT_R32G32_SFLOAT,         // R32G32_FLOAT
                    VK_FORMAT_R16G16B16A16_UNORM,    // R16G16B16A16_UNORM
                    VK_FORMAT_R16G16B16A16_SNORM,    // R16G16B16A16_SNORM
                    VK_FORMAT_R16G16B16A16_UINT,     // R16G16B16A16_UINT
                    VK_FORMAT_R16G16B16A16_SINT,     // R16G16B16A16_SINT
                    VK_FORMAT_R16G16B16A16_SFLOAT,   // R16G16B16A16_FLOAT
                    VK_FORMAT_R32G32B32_UINT,        // R32G32B32_UINT
                    VK_FORMAT_R32G32B32_SINT,        // R32G32B32_SINT
                    VK_FORMAT_R32G32B32_SFLOAT,      // R32G32B32_FLOAT
                    VK_FORMAT_R32G32B32A32_UINT,     // R32G32B32A32_UINT
                    VK_FORMAT_R32G32B32A32_SINT,     // R32G32B32A32_SINT
                    VK_FORMAT_R32G32B32A32_SFLOAT,   // R32G32B32A32_FLOAT
                };

                return translateTable[ BGS_ENUM_INDEX( format ) ];
            }

            VkBool32 MapBigosIndexRestartValueToVulkanBool( INDEX_RESTART_VALUE val )
            {
                static const VkBool32 translateTable[ BGS_ENUM_COUNT( IndexRestartValues ) ] = {
                    // DISABLED
                    // ENABLED_0xFFFF
                    // ENABLED_0xFFFFFF
                };

                return translateTable[ BGS_ENUM_INDEX( val ) ];
            }

            VkPolygonMode MapBigosPolygonFillModeToVulkanPolygonMode( POLYGON_FILL_MODE mode )
            {
                static const VkPolygonMode translateTable[ BGS_ENUM_COUNT( PolygonFillModes ) ] = {
                    VK_POLYGON_MODE_LINE, // WIREFRAME
                    VK_POLYGON_MODE_FILL, // SOLID
                };

                return translateTable[ BGS_ENUM_INDEX( mode ) ];
            }

            VkCullModeFlags MapBigosCullModeToVulkanCullModeFlags( CULL_MODE mode )
            {
                static const VkCullModeFlags translateTable[ BGS_ENUM_COUNT( CullModes ) ] = {
                    VK_CULL_MODE_NONE,      // NONE
                    VK_CULL_MODE_FRONT_BIT, // FRONT
                    VK_CULL_MODE_BACK_BIT,  // BACK
                };

                return translateTable[ BGS_ENUM_INDEX( mode ) ];
            }

            VkFrontFace MapBigosFrontFaceModeToVulkanFrontFace( FRONT_FACE_MODE mode )
            {
                static const VkFrontFace translateTable[ BGS_ENUM_COUNT( FrontFaceModes ) ] = {
                    VK_FRONT_FACE_COUNTER_CLOCKWISE, // COUNTER_CLOCKWISE
                    VK_FRONT_FACE_CLOCKWISE,         // CLOCKWISE
                };

                return translateTable[ BGS_ENUM_INDEX( mode ) ];
            }

            VkSampleCountFlagBits MapBigosSampleCountToVulkanSampleCountFlags( SAMPLE_COUNT cnt )
            {
                static const VkSampleCountFlagBits translateTable[ BGS_ENUM_COUNT( SampleCount ) ] = {
                    VK_SAMPLE_COUNT_1_BIT,  // COUNT_1
                    VK_SAMPLE_COUNT_2_BIT,  // COUNT_2
                    VK_SAMPLE_COUNT_4_BIT,  // COUNT_4
                    VK_SAMPLE_COUNT_8_BIT,  // COUNT_8
                    VK_SAMPLE_COUNT_16_BIT, // COUNT_16
                    VK_SAMPLE_COUNT_32_BIT, // COUNT_32
                    VK_SAMPLE_COUNT_64_BIT, // COUNT_64
                };

                return translateTable[ BGS_ENUM_INDEX( cnt ) ];
            }

            VkCompareOp MapBigosCompareOperationTypeToVulkanCompareOp( COMPARE_OPERATION_TYPE op )
            {
                static const VkCompareOp translateTable[ BGS_ENUM_COUNT( CompareOperationTypes ) ] = {
                    VK_COMPARE_OP_NEVER,            // NEVER
                    VK_COMPARE_OP_LESS,             // LESS
                    VK_COMPARE_OP_EQUAL,            // EQUAL
                    VK_COMPARE_OP_LESS_OR_EQUAL,    // LESS_OR_EQUAL
                    VK_COMPARE_OP_GREATER,          // GREATER
                    VK_COMPARE_OP_NOT_EQUAL,        // NOT_EQUAL
                    VK_COMPARE_OP_GREATER_OR_EQUAL, // GREATER_OR_EQUAL
                    VK_COMPARE_OP_ALWAYS,           // ALWAYS
                };

                return translateTable[ BGS_ENUM_INDEX( op ) ];
            }

            VkStencilOp MapBigosStencilOperationTypeToVulkanStencilOp( STENCIL_OPERATION_TYPE op )
            {
                static const VkStencilOp translateTable[ BGS_ENUM_COUNT( StencilOperationTypes ) ] = {
                    VK_STENCIL_OP_KEEP,                // KEEP
                    VK_STENCIL_OP_ZERO,                // ZERO
                    VK_STENCIL_OP_REPLACE,             // REPLACE
                    VK_STENCIL_OP_INCREMENT_AND_CLAMP, // INCREMENT_AND_CLAMP
                    VK_STENCIL_OP_DECREMENT_AND_CLAMP, // DECREMENT_AND_CLAMP
                    VK_STENCIL_OP_INVERT,              // INVERT
                    VK_STENCIL_OP_INCREMENT_AND_WRAP,  // INCREMENT_AND_WRAP
                    VK_STENCIL_OP_DECREMENT_AND_WRAP,  // DECREMENT_AND_WRAP
                };

                return translateTable[ BGS_ENUM_INDEX( op ) ];
            }

            VkLogicOp MapBigosLogicOperationTypeToVulkanLogicOp( LOGIC_OPERATION_TYPE op )
            {
                static const VkLogicOp translateTable[ BGS_ENUM_COUNT( LogicOperationTypes ) ] = {
                    VK_LOGIC_OP_CLEAR,         // CLEAR
                    VK_LOGIC_OP_AND,           // AND
                    VK_LOGIC_OP_AND_REVERSE,   // AND_REVERSE
                    VK_LOGIC_OP_COPY,          // COPY
                    VK_LOGIC_OP_AND_INVERTED,  // AND_INVERTED
                    VK_LOGIC_OP_NO_OP,         // NO_OP
                    VK_LOGIC_OP_XOR,           // XOR
                    VK_LOGIC_OP_OR,            // OR
                    VK_LOGIC_OP_NOR,           // NOR
                    VK_LOGIC_OP_EQUIVALENT,    // EQUIVALENT
                    VK_LOGIC_OP_INVERT,        // INVERT
                    VK_LOGIC_OP_OR_REVERSE,    // OR_REVERSE
                    VK_LOGIC_OP_COPY_INVERTED, // COPY_INVERTED
                    VK_LOGIC_OP_OR_INVERTED,   // OR_INVERTED
                    VK_LOGIC_OP_NAND,          // NAND
                    VK_LOGIC_OP_SET,           // SET
                };

                return translateTable[ BGS_ENUM_INDEX( op ) ];
            }

            VkBlendFactor MapBigosBlendFactorToVulkanBlendFactor( BLEND_FACTOR factor )
            {
                static const VkBlendFactor translateTable[ BGS_ENUM_COUNT( BlendFactors ) ] = {
                    VK_BLEND_FACTOR_ZERO,                 // ZERO
                    VK_BLEND_FACTOR_ONE,                  // ONE
                    VK_BLEND_FACTOR_SRC_COLOR,            // SRC_COLOR
                    VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,  // ONE_MINUS_SRC_COLOR
                    VK_BLEND_FACTOR_DST_COLOR,            // DST_COLOR
                    VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,  // ONE_MINUS_DST_COLOR
                    VK_BLEND_FACTOR_SRC_ALPHA,            // SRC_ALPHA
                    VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,  // ONE_MINUS_SRC_ALPHA
                    VK_BLEND_FACTOR_DST_ALPHA,            // DST_ALPHA
                    VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,  // ONE_MINUS_DST_ALPHA
                    VK_BLEND_FACTOR_SRC1_COLOR,           // SRC1_COLOR
                    VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR, // ONE_MINUS_SRC1_COLOR
                    VK_BLEND_FACTOR_SRC1_ALPHA,           // SRC1_ALPHA
                    VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA, // ONE_MINUS_SRC1_ALPHA
                    VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,   // SRC_ALPHA_SATURATE
                };

                return translateTable[ BGS_ENUM_INDEX( factor ) ];
            }

            VkBlendOp MapBigosBlendOperationTypeToVulkanBlendOp( BLEND_OPERATION_TYPE op )
            {
                static const VkBlendOp translateTable[ BGS_ENUM_COUNT( BlendOperationTypes ) ] = {
                    VK_BLEND_OP_ADD,              // ADD
                    VK_BLEND_OP_SUBTRACT,         // SUBTRACT
                    VK_BLEND_OP_REVERSE_SUBTRACT, // REVERSE_SUBTRACT
                    VK_BLEND_OP_MIN,              // MIN
                    VK_BLEND_OP_MAX,              // MAX
                };

                return translateTable[ BGS_ENUM_INDEX( op ) ];
            }

            VkMemoryPropertyFlags MapBigosMemoryAccessFlagsToVulkanMemoryPropertFlags( MemoryAccessFlags flags )
            {
                VkMemoryPropertyFlags nativeFlags = 0;

                if( flags & static_cast<uint32_t>( MemoryAccessFlagBits::GPU_ACCESS ) &&
                    !( flags & static_cast<uint32_t>( MemoryAccessFlagBits::CPU_ACCESS ) ) )
                {
                    // Device access only
                    nativeFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
                }
                else
                {
                    if( flags & static_cast<uint32_t>( MemoryAccessFlagBits::CPU_ACCESS ) )
                    {
                        nativeFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
                    }
                    if( flags & static_cast<uint32_t>( MemoryAccessFlagBits::GPU_DEVICE_ACCESS ) )
                    {
                        nativeFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
                    }
                }
                return nativeFlags;
            }

            VkBufferUsageFlags MapBigosResourceUsageToVulkanBufferUsageFlags( ResourceUsageFlags flags )
            {
                VkBufferUsageFlags nativeFlags = 0;
                if( flags & static_cast<uint32_t>( ResourceUsageFlagBits::TRANSFER_SRC ) )
                {
                    nativeFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                }
                if( flags & static_cast<uint32_t>( ResourceUsageFlagBits::TRANSFER_DST ) )
                {
                    nativeFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                }
                if( flags & static_cast<uint32_t>( ResourceUsageFlagBits::READ_ONLY_STORAGE_BUFFER ) ||
                    flags & static_cast<uint32_t>( ResourceUsageFlagBits::READ_WRITE_STORAGE_BUFFER ) )
                {
                    nativeFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
                }
                if( flags & static_cast<uint32_t>( ResourceUsageFlagBits::CONSTANT_TEXEL_BUFFER ) )
                {
                    nativeFlags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
                }
                if( flags & static_cast<uint32_t>( ResourceUsageFlagBits::STORAGE_TEXEL_BUFFER ) )
                {
                    nativeFlags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
                }
                if( flags & static_cast<uint32_t>( ResourceUsageFlagBits::INDEX_BUFFER ) )
                {
                    nativeFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
                }
                if( flags & static_cast<uint32_t>( ResourceUsageFlagBits::CONSTANT_BUFFER ) )
                {
                    nativeFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
                }
                if( flags & static_cast<uint32_t>( ResourceUsageFlagBits::VERTEX_BUFFER ) )
                {
                    nativeFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
                }
                if( flags & static_cast<uint32_t>( ResourceUsageFlagBits::INDIRECT_BUFFER ) )
                {
                    nativeFlags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
                }

                return nativeFlags;
            }

            VkSharingMode MapBigosResourceSharingModeToVulkanSharingMode( RESOURCE_SHARING_MODE mode )
            {
                static const VkSharingMode translateTable[ BGS_ENUM_COUNT( ResourceSharingModes ) ] = {
                    VK_SHARING_MODE_EXCLUSIVE,  // EXCLUSIVE_ACCESS
                    VK_SHARING_MODE_CONCURRENT, // SIMULTANEOUS_ACCESS
                };

                return translateTable[ BGS_ENUM_INDEX( mode ) ];
            }

            VkImageUsageFlags MapBigosResourceUsageFlagsToVulkanImageUsageFlags( ResourceUsageFlags flags )
            {
                VkImageUsageFlags nativeFlags = 0;

                if( flags & static_cast<uint32_t>( ResourceUsageFlagBits::TRANSFER_SRC ) )
                {
                    nativeFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
                }
                if( flags & static_cast<uint32_t>( ResourceUsageFlagBits::TRANSFER_DST ) )
                {
                    nativeFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
                }
                if( flags & static_cast<uint32_t>( ResourceUsageFlagBits::SAMPLED_TEXTURE ) )
                {
                    nativeFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
                }
                if( flags & static_cast<uint32_t>( ResourceUsageFlagBits::STORAGE_TEXTURE ) )
                {
                    nativeFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
                }
                if( flags & static_cast<uint32_t>( ResourceUsageFlagBits::COLOR_RENDER_TARGET ) )
                {
                    nativeFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                }
                if( flags & static_cast<uint32_t>( ResourceUsageFlagBits::DEPTH_STENCIL_TARGET ) )
                {
                    nativeFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                }

                return nativeFlags;
            }

            VkImageType MapBigosResourceTypeToVulkanImageType( RESOURCE_TYPE type )
            {
                BGS_ASSERT( ( type != ResourceTypes::UNKNOWN ) && ( type != ResourceTypes::BUFFER ) );
                static const VkImageType translateTable[ BGS_ENUM_COUNT( ResourceTypes ) ] = {
                    VK_IMAGE_TYPE_MAX_ENUM, // UNKNOWN - invalid for image
                    VK_IMAGE_TYPE_MAX_ENUM, // BUFFER - invalid for image
                    VK_IMAGE_TYPE_1D,       // TEXTURE_1D
                    VK_IMAGE_TYPE_2D,       // TEXTURE_2D
                    VK_IMAGE_TYPE_3D,       // TEXTURE_3D
                };

                return translateTable[ BGS_ENUM_INDEX( type ) ];
            }

            VkImageTiling MapBigosResourceLayoutToVulkanImageTiling( RESOURCE_LAYOUT layout )
            {
                static const VkImageTiling translateTable[ BGS_ENUM_COUNT( ResourceLayouts ) ] = {
                    VK_IMAGE_TILING_OPTIMAL, // OPTIMAL
                    VK_IMAGE_TILING_LINEAR,  // LINEAR
                };

                return translateTable[ BGS_ENUM_INDEX( layout ) ];
            }

            VkImageAspectFlags MapBigosTextureComponentFlagsToVulkanImageAspectFlags( TextureComponentFlags flags )
            {
                VkImageAspectFlags nativeFlags = 0;
                if( flags & static_cast<uint32_t>( TextureComponentFlagBits::COLOR ) )
                {
                    nativeFlags |= VK_IMAGE_ASPECT_COLOR_BIT;
                }
                if( flags & static_cast<uint32_t>( TextureComponentFlagBits::DEPTH ) )
                {
                    nativeFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
                }
                if( flags & static_cast<uint32_t>( TextureComponentFlagBits::STENCIL ) )
                {
                    nativeFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
                }
                return nativeFlags;
            }

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS