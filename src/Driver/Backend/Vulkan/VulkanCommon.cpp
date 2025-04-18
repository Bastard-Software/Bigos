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
                    VK_COMMAND_BUFFER_LEVEL_PRIMARY,   // PRIMARY
                    VK_COMMAND_BUFFER_LEVEL_SECONDARY, // SECONDARY
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
                    VK_IMAGE_TYPE_2D,       // TEXTURE_3D
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

            VkSamplerReductionMode MapBigosSamplerReductionModeToVulkanSamplerReductionMode( SAMPLER_REDUCTION_MODE mode )
            {
                static const VkSamplerReductionMode translateTable[ BGS_ENUM_COUNT( SamplerReductionModes ) ] = {
                    VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE, // WEIGHTED_AVERAGE
                    VK_SAMPLER_REDUCTION_MODE_MIN,              // MIN
                    VK_SAMPLER_REDUCTION_MODE_MAX,              // MAX
                };

                return translateTable[ BGS_ENUM_INDEX( mode ) ];
            }

            VkFilter MapBigosFilterTypeToVulkanFilterType( FILTER_TYPE type )
            {
                static const VkFilter translateTable[ BGS_ENUM_COUNT( FilterTypes ) ] = {
                    VK_FILTER_NEAREST, // NEAREST
                    VK_FILTER_LINEAR,  // LINEAR
                };

                return translateTable[ BGS_ENUM_INDEX( type ) ];
            }

            VkSamplerMipmapMode MapBigosFilterTypeToVulkanMipMapMode( FILTER_TYPE type )
            {
                static const VkSamplerMipmapMode translateTable[ BGS_ENUM_COUNT( FilterTypes ) ] = {
                    VK_SAMPLER_MIPMAP_MODE_NEAREST, // NEAREST
                    VK_SAMPLER_MIPMAP_MODE_LINEAR,  // LINEAR
                };

                return translateTable[ BGS_ENUM_INDEX( type ) ];
            }

            VkBorderColor MapBigosBorderColorToVulkanBorderColor( BORDER_COLOR color )
            {
                // Using only floats to mimic D3D12 behaviour
                static const VkBorderColor translateTable[ BGS_ENUM_COUNT( BorderColors ) ] = {
                    VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK, // TRANSPARENT_BLACK
                    VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,      // OPAQUE_BLACK
                    VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,      // OPAQUE_WHITE
                };

                return translateTable[ BGS_ENUM_INDEX( color ) ];
            }

            VkSamplerAddressMode MapBigosTextureAddressModeToVultakSamplerAddressMode( TEXTURE_ADDRESS_MODE mode )
            {
                static const VkSamplerAddressMode translateTable[ BGS_ENUM_COUNT( TextureAddressModes ) ] = {
                    VK_SAMPLER_ADDRESS_MODE_REPEAT,               // REPEAT
                    VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,      // MIRRORED_REPEAT
                    VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE, // MIRRORED_ONCE
                    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,        // CLAMP_TO_EDGE
                    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,      // CLAMP_TO_BORDER
                };

                return translateTable[ BGS_ENUM_INDEX( mode ) ];
            }

            VkDescriptorType MapBigosBindingTypeToVulkanDescriptorType( BINDING_TYPE type )
            {
                static const VkDescriptorType translateTable[ BGS_ENUM_COUNT( BindingTypes ) ] = {
                    VK_DESCRIPTOR_TYPE_SAMPLER,              // SAMPLER
                    VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,        // SAMPLED_TEXTURE
                    VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,        // STORAGE_TEXTURE
                    VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, // CONSTANT_TEXEL_BUFFER
                    VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, // STORAGE_TEXEL_BUFFER
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,       // CONSTANT_BUFFER
                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,       // READ_ONLY_STORAGE_BUFFER
                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,       // READ_WRITE_STORAGE_BUFFER
                };

                return translateTable[ BGS_ENUM_INDEX( type ) ];
            }

            VkShaderStageFlags MapBigosShaderVisibilityToVulkanShaderStageFlags( SHADER_VISIBILITY vis )
            {
                static const uint32_t translateTable[ BGS_ENUM_COUNT( ShaderVisibilities ) ] = {
                    VK_SHADER_STAGE_VERTEX_BIT,                  // VERTEX
                    VK_SHADER_STAGE_FRAGMENT_BIT,                // PIXEL
                    VK_SHADER_STAGE_GEOMETRY_BIT,                // GEOMETRY
                    VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,    // HULL
                    VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, // DOMAIN
                    VK_SHADER_STAGE_COMPUTE_BIT,                 // COMPUTE
                    VK_SHADER_STAGE_ALL_GRAPHICS,                // ALL_GRAPHICS
                    VK_SHADER_STAGE_ALL,                         // ALL
                };

                return static_cast<VkShaderStageFlags>( translateTable[ BGS_ENUM_INDEX( vis ) ] );
            }

            VkBufferUsageFlags MapBigosBindingHeapTypeToVulkanBufferUsageFlags( BINDING_HEAP_TYPE type )
            {
                static const VkBufferUsageFlags translateTable[ BGS_ENUM_COUNT( BindingHeapTypes ) ] = {
                    VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, // SHADER_RESOURCE
                    VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT,  // SAMPLER
                };

                return static_cast<VkShaderStageFlags>( translateTable[ BGS_ENUM_INDEX( type ) ] );
            }

            VkQueryType MapBigosQueryTypeToVulkanQueryType( QUERY_TYPE type )
            {
                static const VkQueryType translateTable[ BGS_ENUM_COUNT( QueryTypes ) ] = {
                    VK_QUERY_TYPE_OCCLUSION,           // OCCLUSION
                    VK_QUERY_TYPE_PIPELINE_STATISTICS, // PIPELINE_STATISTICS
                    VK_QUERY_TYPE_TIMESTAMP,           // TIMESTAMP
                };

                return translateTable[ BGS_ENUM_INDEX( type ) ];
            }

            VkImageViewType MapBigosTextureTypeToVulkanImageViewType( TEXTURE_TYPE type )
            {
                static const VkImageViewType translateTable[ BGS_ENUM_COUNT( TextureTypes ) ] = {
                    VK_IMAGE_VIEW_TYPE_1D,         // TEXTURE_1D
                    VK_IMAGE_VIEW_TYPE_1D_ARRAY,   // TEXTURE_1D_ARRAY
                    VK_IMAGE_VIEW_TYPE_2D,         // TEXTURE_2D
                    VK_IMAGE_VIEW_TYPE_2D_ARRAY,   // TEXTURE_2D_ARRAY
                    VK_IMAGE_VIEW_TYPE_3D,         // TEXTURE_3D
                    VK_IMAGE_VIEW_TYPE_CUBE,       // TEXURTE_CUBE
                    VK_IMAGE_VIEW_TYPE_CUBE_ARRAY, // TEXURTE_CUBE_ARRAY
                };

                return translateTable[ BGS_ENUM_INDEX( type ) ];
            }

            VkPipelineBindPoint MapBigosPipelineTypeToVulkanPipelineBindPoint( PIPELINE_TYPE type )
            {
                static const VkPipelineBindPoint translateTable[ BGS_ENUM_COUNT( TextureTypes ) ] = {
                    VK_PIPELINE_BIND_POINT_GRAPHICS,        // GRAPHICS
                    VK_PIPELINE_BIND_POINT_COMPUTE,         // COMPUTE
                    VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, // RAY_TRACING
                };

                return translateTable[ BGS_ENUM_INDEX( type ) ];
            }

            VkPipelineStageFlags2 MapBigosPipelineStageFlagsToVulkanPipelineStageFlags( PipelineStageFlags flags )
            {
                VkPipelineStageFlags2 nativeFlags = 0;

                if( flags & BGS_FLAG( PipelineStageFlagBits::INPUT_ASSEMBLER ) )
                {
                    nativeFlags |=
                        VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT | VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
                }
                if( flags & BGS_FLAG( PipelineStageFlagBits::VERTEX_SHADING ) )
                {
                    nativeFlags |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
                }
                if( flags & BGS_FLAG( PipelineStageFlagBits::PIXEL_SHADING ) )
                {
                    nativeFlags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
                }
                if( flags & BGS_FLAG( PipelineStageFlagBits::COMPUTE_SHADING ) )
                {
                    nativeFlags |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
                }
                if( flags & BGS_FLAG( PipelineStageFlagBits::TRANSFER ) )
                {
                    nativeFlags |= VK_PIPELINE_STAGE_2_COPY_BIT;
                }
                if( flags & BGS_FLAG( PipelineStageFlagBits::RESOLVE ) )
                {
                    nativeFlags |= VK_PIPELINE_STAGE_2_RESOLVE_BIT;
                }
                if( flags & BGS_FLAG( PipelineStageFlagBits::EXECUTE_INDIRECT ) )
                {
                    nativeFlags |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
                }
                if( flags & BGS_FLAG( PipelineStageFlagBits::RENDER_TARGET ) )
                {
                    nativeFlags |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
                }
                if( flags & BGS_FLAG( PipelineStageFlagBits::DEPTH_STENCIL ) )
                {
                    nativeFlags |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
                }

                return nativeFlags;
            }

            VkAccessFlags2 MapBigosAccessFlagsToVulkanAccessFlags( AccessFlags flags )
            {
                VkAccessFlags2 nativeFlags = 0;

                if( flags & BGS_FLAG( AccessFlagBits::GENERAL ) )
                {
                    nativeFlags |= VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
                }
                if( flags & BGS_FLAG( AccessFlagBits::VERTEX_BUFFER ) )
                {
                    nativeFlags |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
                }
                if( flags & BGS_FLAG( AccessFlagBits::INDEX_BUFFER ) )
                {
                    nativeFlags |= VK_ACCESS_2_INDEX_READ_BIT;
                }
                if( flags & BGS_FLAG( AccessFlagBits::CONSTANT_BUFFER ) )
                {
                    nativeFlags |= VK_ACCESS_2_UNIFORM_READ_BIT;
                }
                if( flags & BGS_FLAG( AccessFlagBits::INDIRECT_BUFFER ) )
                {
                    nativeFlags |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
                }
                if( flags & BGS_FLAG( AccessFlagBits::RENDER_TARGET ) )
                {
                    nativeFlags |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
                }
                if( flags & BGS_FLAG( AccessFlagBits::SHADER_READ_ONLY ) )
                {
                    nativeFlags |= VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
                }
                if( flags & BGS_FLAG( AccessFlagBits::SHADER_READ_WRITE ) )
                {
                    nativeFlags |= VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
                }
                if( flags & BGS_FLAG( AccessFlagBits::DEPTH_STENCIL_READ ) )
                {
                    nativeFlags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                }
                if( flags & BGS_FLAG( AccessFlagBits::DEPTH_STENCIL_WRITE ) )
                {
                    nativeFlags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                }
                if( flags & BGS_FLAG( AccessFlagBits::TRANSFER_SRC ) )
                {
                    nativeFlags |= VK_ACCESS_2_TRANSFER_READ_BIT;
                }
                if( flags & BGS_FLAG( AccessFlagBits::TRANSFER_DST ) )
                {
                    nativeFlags |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
                }
                if( flags & BGS_FLAG( AccessFlagBits::RESOLVE_SRC ) )
                {
                    nativeFlags |= VK_ACCESS_2_TRANSFER_READ_BIT;
                }
                if( flags & BGS_FLAG( AccessFlagBits::RESOLVE_DST ) )
                {
                    nativeFlags |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
                }

                return nativeFlags;
            }

            VkImageLayout MapBigosTextureLayoutToVulkanImageLayout( TEXTURE_LAYOUT layout )
            {
                static const VkImageLayout translateTable[ BGS_ENUM_COUNT( TextureLayouts ) ] = {
                    VK_IMAGE_LAYOUT_UNDEFINED,                        // UNDEFINED
                    VK_IMAGE_LAYOUT_GENERAL,                          // GENERAL
                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,                  // PRESENT
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         // RENDER_TARGET
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  // DEPTH_STENCIL_READ
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, // DEPTH_STENCIL_WRITE
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         // SHADER_READ_ONLY
                    VK_IMAGE_LAYOUT_GENERAL,                          // SHADER_READ_WRITE
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             // TRANSFER_SRC
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             // TRANSFER_DST
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             // RESOLVE_SRC
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             // RESOLVE_DST
                };

                return translateTable[ BGS_ENUM_INDEX( layout ) ];
            }

            VkIndexType MapBigosIndexTypeToVulkanIndexType( INDEX_TYPE type )
            {
                static const VkIndexType translateTable[ BGS_ENUM_COUNT( IndexTypes ) ] = {
                    VK_INDEX_TYPE_UINT8_EXT, // UINT8
                    VK_INDEX_TYPE_UINT16,    // UINT16
                    VK_INDEX_TYPE_UINT32,    // UINT32
                };

                return translateTable[ BGS_ENUM_INDEX( type ) ];
            }

        } // namespace Backend
    } // namespace Driver
} // namespace BIGOS