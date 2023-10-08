#include "D3D12Common.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {

            D3D12_COMMAND_LIST_TYPE MapBigosQueueTypeToD3D12CommandListType( QUEUE_TYPE type )
            {
                static const D3D12_COMMAND_LIST_TYPE translateTable[ BGS_ENUM_COUNT( QueueTypes ) ] = {
                    D3D12_COMMAND_LIST_TYPE_DIRECT,  // GRAPHICS
                    D3D12_COMMAND_LIST_TYPE_COMPUTE, // COMPUTE
                    D3D12_COMMAND_LIST_TYPE_COPY,    // COPY
                };

                return translateTable[ BGS_ENUM_INDEX( type ) ];
            }

            D3D12_COMMAND_QUEUE_PRIORITY MapBigosQueuePriorityToD3D12CommandQueuePriority( QUEUE_PRIORITY_TYPE prio )
            {
                static const D3D12_COMMAND_QUEUE_PRIORITY translateTable[ BGS_ENUM_COUNT( QueuePriorityTypes ) ] = {
                    D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,          // NORMAL
                    D3D12_COMMAND_QUEUE_PRIORITY_HIGH,            // HIGH
                    D3D12_COMMAND_QUEUE_PRIORITY_GLOBAL_REALTIME, // REALTIME
                };

                return translateTable[ BGS_ENUM_INDEX( prio ) ];
            }

            D3D_PRIMITIVE_TOPOLOGY MapBigosPrimitiveTopologyToD3D12PrimitiveTopology( PRIMITIVE_TOPOLOGY topo )
            {
                static const D3D_PRIMITIVE_TOPOLOGY translateTable[ BGS_ENUM_COUNT( PrimitiveTopologies ) ] = {
                    D3D_PRIMITIVE_TOPOLOGY_POINTLIST,                 // POINT_LIST
                    D3D_PRIMITIVE_TOPOLOGY_LINELIST,                  // LINE_LIST
                    D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,                 // LINE_STRIP
                    D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,              // TRIANGLE_LIST
                    D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,             // TRIANGLE_STRIP
                    D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ,              // LINE_LIST_WITH_ADJACENCY
                    D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ,             // LINE_STRIP_WITH_ADJACENCY
                    D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ,          // TRIANGLE_LIST_WITH_ADJACENCY
                    D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ,         // TRIANGLE_STRIP_WITH_ADJACENCY
                    D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST, // PATCH_LIST
                };

                return translateTable[ BGS_ENUM_INDEX( topo ) ];
            }

            D3D12_PRIMITIVE_TOPOLOGY_TYPE MapBigosPrimitiveTopologyToD3D12PrimitiveTopologyType( PRIMITIVE_TOPOLOGY topo )
            {
                static const D3D12_PRIMITIVE_TOPOLOGY_TYPE translateTable[ BGS_ENUM_COUNT( PrimitiveTopologies ) ] = {
                    D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT,    // POINT_LIST
                    D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,     // LINE_LIST
                    D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,     // LINE_STRIP
                    D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, // TRIANGLE_LIST
                    D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, // TRIANGLE_STRIP
                    D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,     // LINE_LIST_WITH_ADJACENCY
                    D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,     // LINE_STRIP_WITH_ADJACENCY
                    D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, // TRIANGLE_LIST_WITH_ADJACENCY
                    D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, // TRIANGLE_STRIP_WITH_ADJACENCY
                    D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH,    // PATCH_LIST
                };

                return translateTable[ BGS_ENUM_INDEX( topo ) ];
            }

            D3D12_INPUT_CLASSIFICATION MapBigosInputStepRateToD3D12InputClassification( INPUT_STEP_RATE rate )
            {
                static const D3D12_INPUT_CLASSIFICATION translateTable[ BGS_ENUM_COUNT( InputStepRates ) ] = {
                    D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,   // PER_VERTEX
                    D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, // PER_INSTANCE
                };

                return translateTable[ BGS_ENUM_INDEX( rate ) ];
            }

            DXGI_FORMAT MapBigosFormatToD3D12Format( FORMAT format )
            {
                static const DXGI_FORMAT translateTable[ BGS_ENUM_COUNT( Formats ) ] = {
                    DXGI_FORMAT_UNKNOWN,             // UNKNOWN
                    DXGI_FORMAT_B5G5R5A1_UNORM,      // B5G5R5A1_UNORM
                    DXGI_FORMAT_B5G6R5_UNORM,        // B5G6R5_UNORM
                    DXGI_FORMAT_B8G8R8A8_UNORM,      // B8G8R8A8_UNORM
                    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, // B8G8R8A8_UNORM_SRGB
                    DXGI_FORMAT_BC1_UNORM,           // BC1_UNORM
                    DXGI_FORMAT_BC1_UNORM_SRGB,      // BC1_UNORM_SRGB
                    DXGI_FORMAT_BC2_UNORM,           // BC2_UNORM
                    DXGI_FORMAT_BC2_UNORM_SRGB,      // BC2_UNORM_SRGB
                    DXGI_FORMAT_BC3_UNORM,           // BC3_UNORM
                    DXGI_FORMAT_BC3_UNORM_SRGB,      // BC3_UNORM_SRGB
                    DXGI_FORMAT_BC4_UNORM,           // BC4_UNORM
                    DXGI_FORMAT_BC4_SNORM,           // BC4_SNORM
                    DXGI_FORMAT_BC5_UNORM,           // BC5_UNORM
                    DXGI_FORMAT_BC5_SNORM,           // BC5_SNORM
                    DXGI_FORMAT_BC6H_UF16,           // BC6H_UFLOAT
                    DXGI_FORMAT_BC6H_SF16,           // BC6H_SFLOAT
                    DXGI_FORMAT_BC7_UNORM,           // BC7_UNORM
                    DXGI_FORMAT_BC7_UNORM_SRGB,      // BC7_UNORM_SRGB
                    DXGI_FORMAT_R8_UINT,             // R8_UINT
                    DXGI_FORMAT_R8_SINT,             // R8_SINT
                    DXGI_FORMAT_R8_UNORM,            // R8_UNORM
                    DXGI_FORMAT_R8_SNORM,            // R8_SNORM
                    DXGI_FORMAT_R16_UINT,            // R16_UINT
                    DXGI_FORMAT_R16_SINT,            // R16_SINT
                    DXGI_FORMAT_R16_UNORM,           // R16_UNORM
                    DXGI_FORMAT_R16_SNORM,           // R16_SNORM
                    DXGI_FORMAT_R16_FLOAT,           // R16_FLOAT
                    DXGI_FORMAT_D16_UNORM,           // D16_UNORM
                    DXGI_FORMAT_R8G8_UINT,           // R8G8_UINT
                    DXGI_FORMAT_R8G8_SINT,           // R8G8_SINT
                    DXGI_FORMAT_R8G8_UNORM,          // R8G8_UNORM
                    DXGI_FORMAT_R8G8_SNORM,          // R8G8_SNORM
                    DXGI_FORMAT_D24_UNORM_S8_UINT,   // D24_UNORM_S8_UINT
                    DXGI_FORMAT_R32_FLOAT,           // R32_FLOAT
                    DXGI_FORMAT_R32_UINT,            // R32_UINT
                    DXGI_FORMAT_R32_SINT,            // R32_SINT
                    DXGI_FORMAT_D32_FLOAT,           // D32_FLOAT
                    DXGI_FORMAT_R16G16_UINT,         // R16G16_UINT
                    DXGI_FORMAT_R16G16_SINT,         // R16G16_SINT
                    DXGI_FORMAT_R16G16_UNORM,        // R16G16_UNORM
                    DXGI_FORMAT_R16G16_SNORM,        // R16G16_SNORM
                    DXGI_FORMAT_R16G16_FLOAT,        // R16G16_FLOAT
                    DXGI_FORMAT_R8G8B8A8_UNORM,      // R8G8B8A8_UNORM
                    DXGI_FORMAT_R8G8B8A8_SNORM,      // R8G8B8A8_SNORM
                    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, // R8G8B8A8_UNORM_SRGB
                    DXGI_FORMAT_R8G8B8A8_UINT,       // R8G8B8A8_UINT
                    DXGI_FORMAT_R8G8B8A8_SINT,       // R8G8B8A8_SINT
                    DXGI_FORMAT_R32G32_UINT,         // R32G32_UINT
                    DXGI_FORMAT_R32G32_SINT,         // R32G32_SINT
                    DXGI_FORMAT_R32G32_FLOAT,        // R32G32_FLOAT
                    DXGI_FORMAT_R16G16B16A16_UNORM,  // R16G16B16A16_UNORM
                    DXGI_FORMAT_R16G16B16A16_SNORM,  // R16G16B16A16_SNORM
                    DXGI_FORMAT_R16G16B16A16_UINT,   // R16G16B16A16_UINT
                    DXGI_FORMAT_R16G16B16A16_SINT,   // R16G16B16A16_SINT
                    DXGI_FORMAT_R16G16B16A16_FLOAT,  // R16G16B16A16_FLOAT
                    DXGI_FORMAT_R32G32B32_UINT,      // R32G32B32_UINT
                    DXGI_FORMAT_R32G32B32_SINT,      // R32G32B32_SINT
                    DXGI_FORMAT_R32G32B32_FLOAT,     // R32G32B32_FLOAT
                    DXGI_FORMAT_R32G32B32A32_UINT,   // R32G32B32A32_UINT
                    DXGI_FORMAT_R32G32B32A32_SINT,   // R32G32B32A32_SINT
                    DXGI_FORMAT_R32G32B32A32_FLOAT,  // R32G32B32A32_FLOAT
                };

                return translateTable[ BGS_ENUM_INDEX( format ) ];
            }

            D3D12_INDEX_BUFFER_STRIP_CUT_VALUE MapBigosIndexRestartValueToD3D12IndexBufferStripCutValue( INDEX_RESTART_VALUE val )
            {
                static const D3D12_INDEX_BUFFER_STRIP_CUT_VALUE translateTable[ BGS_ENUM_COUNT( IndexRestartValues ) ] = {
                    D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,   // DISABLED
                    D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF,     // ENABLED_0xFFFF
                    D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF, // ENABLED_0xFFFFFF
                };

                return translateTable[ BGS_ENUM_INDEX( val ) ];
            }

            D3D12_FILL_MODE MapBigosPolygonFillModeToD3D12FillModes( POLYGON_FILL_MODE mode )
            {
                static const D3D12_FILL_MODE translateTable[ BGS_ENUM_COUNT( PolygonFillModes ) ] = {
                    D3D12_FILL_MODE_WIREFRAME, // WIREFRAME
                    D3D12_FILL_MODE_SOLID,     // SOLID
                };

                return translateTable[ BGS_ENUM_INDEX( mode ) ];
            }

            D3D12_CULL_MODE MapBigosCullModeToD3D12CullMode( CULL_MODE mode )
            {
                static const D3D12_CULL_MODE translateTable[ BGS_ENUM_COUNT( CullModes ) ] = {
                    D3D12_CULL_MODE_NONE,  // NONE
                    D3D12_CULL_MODE_FRONT, // FRONT
                    D3D12_CULL_MODE_BACK,  // BACK
                };

                return translateTable[ BGS_ENUM_INDEX( mode ) ];
            }

            BOOL MapBigosFrontFaceModeToD3D12Bool( FRONT_FACE_MODE mode )
            {
                static const BOOL translateTable[ BGS_ENUM_COUNT( FrontFaceModes ) ] = {
                    TRUE,  // COUNTER_CLOCKWISE
                    FALSE, // CLOCKWISE
                };

                return translateTable[ BGS_ENUM_INDEX( mode ) ];
            }

            UINT MapBigosSampleCountToD3D12Uint( SAMPLE_COUNT cnt )
            {
                static const UINT translateTable[ BGS_ENUM_COUNT( SampleCount ) ] = {
                    1U,  // COUNT_1
                    2U,  // COUNT_2
                    4U,  // COUNT_4
                    8U,  // COUNT_8
                    16U, // COUNT_16
                    32U, // COUNT_32
                    64U, // COUNT_64
                };

                return translateTable[ BGS_ENUM_INDEX( cnt ) ];
            }

            D3D12_DEPTH_WRITE_MASK MapBigosBoolToD3D12DepthWriteMask( bool_t write )
            {
                static const D3D12_DEPTH_WRITE_MASK translateTable[ 2 ] = {
                    D3D12_DEPTH_WRITE_MASK_ZERO, // BGS_FALSE
                    D3D12_DEPTH_WRITE_MASK_ALL,  // BGS_TRUE
                };

                return translateTable[ BGS_BOOL_INDEX( write ) ];
            }

            D3D12_STENCIL_OP MapBigosStencilOperationTypeToD3D12StencilOp( STENCIL_OPERATION_TYPE op )
            {
                static const D3D12_STENCIL_OP translateTable[ BGS_ENUM_COUNT( StencilOperationTypes ) ] = {
                    D3D12_STENCIL_OP_KEEP,     // KEEP
                    D3D12_STENCIL_OP_ZERO,     // ZERO
                    D3D12_STENCIL_OP_REPLACE,  // REPLACE
                    D3D12_STENCIL_OP_INCR_SAT, // INCREMENT_AND_CLAMP
                    D3D12_STENCIL_OP_DECR_SAT, // DECREMENT_AND_CLAMP
                    D3D12_STENCIL_OP_INVERT,   // INVERT
                    D3D12_STENCIL_OP_INCR,     // INCREMENT_AND_WRAP
                    D3D12_STENCIL_OP_DECR,     // DECREMENT_AND_WRAP
                };

                return translateTable[ BGS_ENUM_INDEX( op ) ];
            }

            D3D12_COMPARISON_FUNC MapBigosCompareOperationTypeToD3D12ComparsionFunc( COMPARE_OPERATION_TYPE op )
            {
                static const D3D12_COMPARISON_FUNC translateTable[ BGS_ENUM_COUNT( CompareOperationTypes ) ] = {
                    D3D12_COMPARISON_FUNC_NEVER,         // NEVER,
                    D3D12_COMPARISON_FUNC_LESS,          // LESS,
                    D3D12_COMPARISON_FUNC_EQUAL,         // EQUAL,
                    D3D12_COMPARISON_FUNC_LESS_EQUAL,    // LESS_OR_EQUAL,
                    D3D12_COMPARISON_FUNC_GREATER,       // GREATER,
                    D3D12_COMPARISON_FUNC_NOT_EQUAL,     // NOT_EQUAL,
                    D3D12_COMPARISON_FUNC_GREATER_EQUAL, // GREATER_OR_EQUAL,
                    D3D12_COMPARISON_FUNC_ALWAYS,        // ALWAYS,
                };

                return translateTable[ BGS_ENUM_INDEX( op ) ];
            }

            D3D12_LOGIC_OP MapBigosLogicOperationTypeToD3D12LogicOp( LOGIC_OPERATION_TYPE op )
            {
                static const D3D12_LOGIC_OP translateTable[ BGS_ENUM_COUNT( LogicOperationTypes ) ] = {
                    D3D12_LOGIC_OP_CLEAR,         // CLEAR
                    D3D12_LOGIC_OP_AND,           // AND
                    D3D12_LOGIC_OP_AND_REVERSE,   // AND_REVERSE
                    D3D12_LOGIC_OP_COPY,          // COPY
                    D3D12_LOGIC_OP_AND_INVERTED,  // AND_INVERTED
                    D3D12_LOGIC_OP_NOOP,          // NO_OP
                    D3D12_LOGIC_OP_XOR,           // XOR
                    D3D12_LOGIC_OP_OR,            // OR
                    D3D12_LOGIC_OP_NOR,           // NOR
                    D3D12_LOGIC_OP_EQUIV,         // EQUIVALENT
                    D3D12_LOGIC_OP_INVERT,        // INVERT
                    D3D12_LOGIC_OP_OR_REVERSE,    // OR_REVERSE
                    D3D12_LOGIC_OP_COPY_INVERTED, // COPY_INVERTED
                    D3D12_LOGIC_OP_OR_INVERTED,   // OR_INVERTED
                    D3D12_LOGIC_OP_NAND,          // NAND
                    D3D12_LOGIC_OP_SET,           // SET
                };

                return translateTable[ BGS_ENUM_INDEX( op ) ];
            }

            D3D12_BLEND MapBigosBlendFactorToD3D12Blend( BLEND_FACTOR factor )
            {
                static const D3D12_BLEND translateTable[ BGS_ENUM_COUNT( BlendFactors ) ] = {
                    D3D12_BLEND_ZERO,           // ZERO
                    D3D12_BLEND_ONE,            // ONE
                    D3D12_BLEND_SRC_COLOR,      // SRC_COLOR
                    D3D12_BLEND_INV_SRC_COLOR,  // ONE_MINUS_SRC_COLOR
                    D3D12_BLEND_DEST_COLOR,     // DST_COLOR
                    D3D12_BLEND_INV_DEST_COLOR, // ONE_MINUS_DST_COLOR
                    D3D12_BLEND_SRC_ALPHA,      // SRC_ALPHA
                    D3D12_BLEND_INV_SRC_ALPHA,  // ONE_MINUS_SRC_ALPHA
                    D3D12_BLEND_DEST_ALPHA,     // DST_ALPHA
                    D3D12_BLEND_INV_DEST_ALPHA, // ONE_MINUS_DST_ALPHA
                    D3D12_BLEND_SRC1_COLOR,     // SRC1_COLOR
                    D3D12_BLEND_INV_SRC1_COLOR, // ONE_MINUS_SRC1_COLOR
                    D3D12_BLEND_SRC1_ALPHA,     // SRC1_ALPHA
                    D3D12_BLEND_INV_SRC1_ALPHA, // ONE_MINUS_SRC1_ALPHA
                    D3D12_BLEND_SRC_ALPHA_SAT,  // SRC_ALPHA_SATURATE
                };

                return translateTable[ BGS_ENUM_INDEX( factor ) ];
            }

            D3D12_BLEND_OP MapBigosBlendOperationTypeToD3D12BlendOp( BLEND_OPERATION_TYPE op )
            {
                static const D3D12_BLEND_OP translateTable[ BGS_ENUM_COUNT( BlendOperationTypes ) ] = {
                    D3D12_BLEND_OP_ADD,          // ADD
                    D3D12_BLEND_OP_SUBTRACT,     // SUBTRACT
                    D3D12_BLEND_OP_REV_SUBTRACT, // REVERSE_SUBTRACT
                    D3D12_BLEND_OP_MIN,          // MIN
                    D3D12_BLEND_OP_MAX,          // MAX
                };

                return translateTable[ BGS_ENUM_INDEX( op ) ];
            }

            D3D12_HEAP_FLAGS MapBigosMemoryHeapUsageToD3D12HeapFlags( MEMORY_HEAP_USAGE usage )
            {
                static const D3D12_HEAP_FLAGS translateTable[ BGS_ENUM_COUNT( MemoryHeapUsages ) ] = {
                    D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS,                                           // BUFFERS
                    D3D12_HEAP_FLAG_DENY_BUFFERS | D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES, // TEXTURES
                    D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_BUFFERS |
                        D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES, // RENDER_TARGETS
                };

                return translateTable[ BGS_ENUM_INDEX( usage ) ];
            }

            D3D12_RESOURCE_DIMENSION MapBigosResourceTypeToD3D12ResourceDimension( RESOURCE_TYPE type )
            {
                static const D3D12_RESOURCE_DIMENSION translateTable[ BGS_ENUM_COUNT( ResourceTypes ) ] = {
                    D3D12_RESOURCE_DIMENSION_UNKNOWN,   // UNKNOWN
                    D3D12_RESOURCE_DIMENSION_BUFFER,    // BUFFER
                    D3D12_RESOURCE_DIMENSION_TEXTURE1D, // TEXTURE_1D
                    D3D12_RESOURCE_DIMENSION_TEXTURE2D, // TEXTURE_2D
                    D3D12_RESOURCE_DIMENSION_TEXTURE3D, // TEXTURE_3D
                };

                return translateTable[ BGS_ENUM_INDEX( type ) ];
            }

            D3D12_RESOURCE_FLAGS MapBigosResourceUsageFlagsToD3D12ResourceFlags( ResourceUsageFlags flags )
            {
                D3D12_RESOURCE_FLAGS nativeFlags = D3D12_RESOURCE_FLAG_NONE;

                if( flags & static_cast<uint32_t>( ResourceUsageFlagBits::COLOR_RENDER_TARGET ) )
                {
                    nativeFlags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
                }
                if( flags & static_cast<uint32_t>( ResourceUsageFlagBits::DEPTH_STENCIL_TARGET ) )
                {
                    nativeFlags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
                }
                if( ( flags & static_cast<uint32_t>( ResourceUsageFlagBits::STORAGE_TEXEL_BUFFER ) ) &&
                    ( flags & static_cast<uint32_t>( ResourceUsageFlagBits::STORAGE_TEXTURE ) ) &&
                    ( flags & static_cast<uint32_t>( ResourceUsageFlagBits::READ_WRITE_STORAGE_BUFFER ) ) )
                {
                    nativeFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
                }

                return nativeFlags;
            }

            UINT MapBigosTextureComponentFlagsToD3D12PlaneSlice( TextureComponentFlags flags )
            {
                return flags & static_cast<uint32_t>( TextureComponentFlagBits::STENCIL ) ? 1 : 0;
            }

            D3D12_FILTER MapBigosFiltesToD3D12Filter( FILTER_TYPE min, FILTER_TYPE mag, FILTER_TYPE mip, SAMPLER_REDUCTION_MODE mode,
                                                      bool_t anisotropy, bool_t compare )
            {
                uint32_t nativeFilter = 0;

                if( mip == FilterTypes::LINEAR )
                {
                    SET_BIT( nativeFilter, 0 );
                }
                if( mag == FilterTypes::LINEAR )
                {
                    SET_BIT( nativeFilter, 2 );
                }
                if( min == FilterTypes::LINEAR )
                {
                    SET_BIT( nativeFilter, 4 );
                }
                if( anisotropy )
                {
                    SET_BIT( nativeFilter, 0 );
                    SET_BIT( nativeFilter, 2 );
                    SET_BIT( nativeFilter, 4 );
                    SET_BIT( nativeFilter, 6 );
                }
                if( compare )
                {
                    SET_BIT( nativeFilter, 7 );
                }
                if( mode == SamplerReductionModes::MIN )
                {
                    SET_BIT( nativeFilter, 8 );
                    // If compare is true and mode == SamplerReductionModes::MIN, we would get the same state as mode == SamplerReductionModes::MAX.
                    // Thats why we need to clear 7th BIT
                    CLEAR_BIT( nativeFilter, 7 );
                }
                if( mode == SamplerReductionModes::MAX )
                {
                    SET_BIT( nativeFilter, 7 );
                    SET_BIT( nativeFilter, 8 );
                }

                return static_cast<D3D12_FILTER>( nativeFilter );
            }

            D3D12_STATIC_BORDER_COLOR MapBigosBorderColorToD3D12StaticBorderColor( BORDER_COLOR color )
            {
                static const D3D12_STATIC_BORDER_COLOR translateTable[ BGS_ENUM_COUNT( BorderColors ) ] = {
                    D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK, // TRANSPARENT_BLACK
                    D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK,      // OPAQUE_BLACK
                    D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,      // OPAQUE_WHITE
                };

                return translateTable[ BGS_ENUM_INDEX( color ) ];
            }

            D3D12_TEXTURE_ADDRESS_MODE MapBigosTextureAddressModeToD3D12TextureAddressMode( TEXTURE_ADDRESS_MODE mode )
            {
                static const D3D12_TEXTURE_ADDRESS_MODE translateTable[ BGS_ENUM_COUNT( TextureAddressModes ) ] = {
                    D3D12_TEXTURE_ADDRESS_MODE_WRAP,        // REPEAT
                    D3D12_TEXTURE_ADDRESS_MODE_MIRROR,      // MIRRORED_REPEAT
                    D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE, // MIRRORED_ONCE
                    D3D12_TEXTURE_ADDRESS_MODE_CLAMP,       // CLAMP_TO_EDGE
                    D3D12_TEXTURE_ADDRESS_MODE_BORDER,      // CLAMP_TO_BORDER
                };

                return translateTable[ BGS_ENUM_INDEX( mode ) ];
            }

            D3D12_SHADER_VISIBILITY MapBigosShaderVisibilityToD3D12ShaderVisibility( SHADER_VISIBILITY vis )
            {
                static const D3D12_SHADER_VISIBILITY translateTable[ BGS_ENUM_COUNT( ShaderVisibilities ) ] = {
                    D3D12_SHADER_VISIBILITY_VERTEX,   // VERTEX
                    D3D12_SHADER_VISIBILITY_PIXEL,    // PIXEL
                    D3D12_SHADER_VISIBILITY_GEOMETRY, // GEOMETRYs
                    D3D12_SHADER_VISIBILITY_HULL,     // HULL
                    D3D12_SHADER_VISIBILITY_DOMAIN,   // DOMAIN
                    D3D12_SHADER_VISIBILITY_ALL,      // COMPUTE
                    D3D12_SHADER_VISIBILITY_ALL,      // ALL_GRAPHICS
                    D3D12_SHADER_VISIBILITY_ALL,      // ALL
                };

                return translateTable[ BGS_ENUM_INDEX( vis ) ];
            }

            D3D12_DESCRIPTOR_RANGE_TYPE MapBigosBindingTypeToD3D12DescriptorRangeType( BINDING_TYPE type )
            {
                static const D3D12_DESCRIPTOR_RANGE_TYPE translateTable[ BGS_ENUM_COUNT( BindingTypes ) ] = {
                    D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, // SAMPLER
                    D3D12_DESCRIPTOR_RANGE_TYPE_SRV,     // SAMPLED_TEXTURE
                    D3D12_DESCRIPTOR_RANGE_TYPE_UAV,     // STORAGE_TEXTURE
                    D3D12_DESCRIPTOR_RANGE_TYPE_SRV,     // CONSTANT_TEXEL_BUFFER
                    D3D12_DESCRIPTOR_RANGE_TYPE_UAV,     // STORAGE_TEXEL_BUFFER
                    D3D12_DESCRIPTOR_RANGE_TYPE_CBV,     // CONSTANT_BUFFER
                    D3D12_DESCRIPTOR_RANGE_TYPE_SRV,     // READ_ONLY_STORAGE_BUFFER
                    D3D12_DESCRIPTOR_RANGE_TYPE_UAV,     // READ_WRITE_STORAGE_BUFFER
                };

                return translateTable[ BGS_ENUM_INDEX( type ) ];
            }

            D3D12_DESCRIPTOR_HEAP_TYPE MapBigosBindingHeapTypeToD3D12DescriptorHeapType( BINDING_HEAP_TYPE type )
            {
                static const D3D12_DESCRIPTOR_HEAP_TYPE translateTable[ BGS_ENUM_COUNT( BindingHeapTypes ) ] = {
                    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, // SHADER_RESOURCE
                    D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,     // SAMPLER
                };

                return translateTable[ BGS_ENUM_INDEX( type ) ];
            }

            D3D12_QUERY_HEAP_TYPE MapBigosQueryTypeToD3D12QueryHeapType( QUERY_TYPE type )
            {
                static const D3D12_QUERY_HEAP_TYPE translateTable[ BGS_ENUM_COUNT( QueryTypes ) ] = {
                    D3D12_QUERY_HEAP_TYPE_OCCLUSION,           // OCCLUSION
                    D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS, // PIPELINE_STATISTICS
                    D3D12_QUERY_HEAP_TYPE_TIMESTAMP,           // TIMESTAMP
                };

                return translateTable[ BGS_ENUM_INDEX( type ) ];
            }

            D3D12_QUERY_TYPE MapBigosQueryTypeToD3D12QueryType( QUERY_TYPE type )
            {
                static const D3D12_QUERY_TYPE translateTable[ BGS_ENUM_COUNT( QueryTypes ) ] = {
                    D3D12_QUERY_TYPE_OCCLUSION,           // OCCLUSION
                    D3D12_QUERY_TYPE_PIPELINE_STATISTICS, // PIPELINE_STATISTICS
                    D3D12_QUERY_TYPE_TIMESTAMP,           // TIMESTAMP
                };

                return translateTable[ BGS_ENUM_INDEX( type ) ];
            }

            D3D12_BARRIER_SYNC MapBigosPipelineStageFlagsToD3D12BarrierSync( PipelineStageFlags flags )
            {
                uint32_t nativeFlags = 0;

                if( flags & BGS_FLAG( PipelineStageFlagBits::INPUT_ASSEMBLER ) )
                {
                    nativeFlags |= D3D12_BARRIER_SYNC_INPUT_ASSEMBLER;
                }
                if( flags & BGS_FLAG( PipelineStageFlagBits::VERTEX_SHADING ) )
                {
                    nativeFlags |= D3D12_BARRIER_SYNC_VERTEX_SHADING;
                }
                if( flags & BGS_FLAG( PipelineStageFlagBits::PIXEL_SHADING ) )
                {
                    nativeFlags |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
                }
                if( flags & BGS_FLAG( PipelineStageFlagBits::COMPUTE_SHADING ) )
                {
                    nativeFlags |= D3D12_BARRIER_SYNC_COMPUTE_SHADING;
                }
                if( flags & BGS_FLAG( PipelineStageFlagBits::TRANSFER ) )
                {
                    nativeFlags |= D3D12_BARRIER_SYNC_COPY;
                }
                if( flags & BGS_FLAG( PipelineStageFlagBits::RESOLVE ) )
                {
                    nativeFlags |= D3D12_BARRIER_SYNC_RESOLVE;
                }
                if( flags & BGS_FLAG( PipelineStageFlagBits::EXECUTE_INDIRECT ) )
                {
                    nativeFlags |= D3D12_BARRIER_SYNC_EXECUTE_INDIRECT;
                }
                if( flags & BGS_FLAG( PipelineStageFlagBits::RENDER_TARGET ) )
                {
                    nativeFlags |= D3D12_BARRIER_SYNC_RENDER_TARGET;
                }
                if( flags & BGS_FLAG( PipelineStageFlagBits::DEPTH_STENCIL ) )
                {
                    nativeFlags |= D3D12_BARRIER_SYNC_DEPTH_STENCIL;
                }

                return static_cast<D3D12_BARRIER_SYNC>( nativeFlags );
            }

            D3D12_BARRIER_ACCESS MapBigosAccessFlagsToD3D12BarrierAccess( AccessFlags flags )
            {
                uint32_t nativeFlags = 0;

                if( flags & BGS_FLAG( AccessFlagBits::NONE ) )
                {
                    nativeFlags |= D3D12_BARRIER_ACCESS_NO_ACCESS;
                }
                if( flags & BGS_FLAG( AccessFlagBits::GENERAL ) )
                {
                    nativeFlags |= D3D12_BARRIER_ACCESS_COMMON;
                }
                if( flags & BGS_FLAG( AccessFlagBits::VERTEX_BUFFER ) )
                {
                    nativeFlags |= D3D12_BARRIER_ACCESS_VERTEX_BUFFER;
                }
                if( flags & BGS_FLAG( AccessFlagBits::INDEX_BUFFER ) )
                {
                    nativeFlags |= D3D12_BARRIER_ACCESS_INDEX_BUFFER;
                }
                if( flags & BGS_FLAG( AccessFlagBits::CONSTANT_BUFFER ) )
                {
                    nativeFlags |= D3D12_BARRIER_ACCESS_CONSTANT_BUFFER;
                }
                if( flags & BGS_FLAG( AccessFlagBits::INDIRECT_BUFFER ) )
                {
                    nativeFlags |= D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT;
                }
                if( flags & BGS_FLAG( AccessFlagBits::RENDER_TARGET ) )
                {
                    nativeFlags |= D3D12_BARRIER_ACCESS_RENDER_TARGET;
                }
                if( flags & BGS_FLAG( AccessFlagBits::SHADER_READ_ONLY ) )
                {
                    nativeFlags |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
                }
                if( flags & BGS_FLAG( AccessFlagBits::SHADER_READ_WRITE ) )
                {
                    nativeFlags |= D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
                }
                if( flags & BGS_FLAG( AccessFlagBits::DEPTH_STENCIL_READ ) )
                {
                    nativeFlags |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;
                }
                if( flags & BGS_FLAG( AccessFlagBits::DEPTH_STENCIL_WRITE ) )
                {
                    nativeFlags |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;
                }
                if( flags & BGS_FLAG( AccessFlagBits::TRANSFER_SRC ) )
                {
                    nativeFlags |= D3D12_BARRIER_ACCESS_COPY_SOURCE;
                }
                if( flags & BGS_FLAG( AccessFlagBits::TRANSFER_DST ) )
                {
                    nativeFlags |= D3D12_BARRIER_ACCESS_COPY_DEST;
                }
                if( flags & BGS_FLAG( AccessFlagBits::RESOLVE_SRC ) )
                {
                    nativeFlags |= D3D12_BARRIER_ACCESS_RESOLVE_SOURCE;
                }
                if( flags & BGS_FLAG( AccessFlagBits::RESOLVE_DST ) )
                {
                    nativeFlags |= D3D12_BARRIER_ACCESS_RESOLVE_DEST;
                }

                return static_cast<D3D12_BARRIER_ACCESS>( nativeFlags );
            }

            D3D12_BARRIER_LAYOUT MapBigosTextureLayoutToD3D12BarierrLayout( TEXTURE_LAYOUT layout )
            {
                static const D3D12_BARRIER_LAYOUT translateTable[ BGS_ENUM_COUNT( TextureLayouts ) ] = {
                    D3D12_BARRIER_LAYOUT_UNDEFINED,           // UNDEFINED
                    D3D12_BARRIER_LAYOUT_COMMON,              // GENERAL
                    D3D12_BARRIER_LAYOUT_PRESENT,             // PRESENT
                    D3D12_BARRIER_LAYOUT_RENDER_TARGET,       // RENDER_TARGET
                    D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ,  // DEPTH_STENCIL_READ
                    D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE, // DEPTH_STENCIL_WRITE
                    D3D12_BARRIER_LAYOUT_SHADER_RESOURCE,     // SHADER_READ_ONLY
                    D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS,    // SHADER_READ_WRITE
                    D3D12_BARRIER_LAYOUT_COPY_SOURCE,         // TRANSFER_SRC
                    D3D12_BARRIER_LAYOUT_COPY_DEST,           // TRANSFER_DST
                    D3D12_BARRIER_LAYOUT_RESOLVE_SOURCE,      // RESOLVE_SRC
                    D3D12_BARRIER_LAYOUT_RESOLVE_DEST,        // RESOLVE_DST
                };

                return translateTable[ BGS_ENUM_INDEX( layout ) ];
            }

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS