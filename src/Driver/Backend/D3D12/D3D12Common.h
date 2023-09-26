#pragma once

#include "D3D12Types.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {

            D3D12_COMMAND_LIST_TYPE            MapBigosQueueTypeToD3D12CommandListType( QUEUE_TYPE type );
            D3D12_COMMAND_QUEUE_PRIORITY       MapBigosQueuePriorityToD3D12CommandQueuePriority( QUEUE_PRIORITY_TYPE prio );
            D3D_PRIMITIVE_TOPOLOGY             MapBigosPrimitiveTopologyToD3D12PrimitiveTopology( PRIMITIVE_TOPOLOGY topo );
            D3D12_PRIMITIVE_TOPOLOGY_TYPE      MapBigosPrimitiveTopologyToD3D12PrimitiveTopologyType( PRIMITIVE_TOPOLOGY topo );
            D3D12_INPUT_CLASSIFICATION         MapBigosInputStepRateToD3D12InputClassification( INPUT_STEP_RATE rate );
            DXGI_FORMAT                        MapBigosFormatToD3D12Format( FORMAT format );
            D3D12_INDEX_BUFFER_STRIP_CUT_VALUE MapBigosIndexRestartValueToD3D12IndexBufferStripCutValue( INDEX_RESTART_VALUE val );
            D3D12_FILL_MODE                    MapBigosPolygonFillModeToD3D12FillModes( POLYGON_FILL_MODE mode );
            D3D12_CULL_MODE                    MapBigosCullModeToD3D12CullMode( CULL_MODE mode );
            BOOL                               MapBigosFrontFaceModeToD3D12Bool( FRONT_FACE_MODE mode );
            UINT                               MapBigosSampleCountToD3D12Uint( SAMPLE_COUNT cnt );
            D3D12_DEPTH_WRITE_MASK             MapBigosBoolToD3D12DepthWriteMask( bool_t write );
            D3D12_STENCIL_OP                   MapBigosStencilOperationTypeToD3D12StencilOp( STENCIL_OPERATION_TYPE op );
            D3D12_COMPARISON_FUNC              MapBigosCompareOperationTypeToD3D12ComparsionFunc( COMPARE_OPERATION_TYPE op );
            D3D12_LOGIC_OP                     MapBigosLogicOperationTypeToD3D12LogicOp( LOGIC_OPERATION_TYPE op );
            D3D12_BLEND                        MapBigosBlendFactorToD3D12Blend( BLEND_FACTOR factor );
            D3D12_BLEND_OP                     MapBigosBlendOperationTypeToD3D12BlendOp( BLEND_OPERATION_TYPE op );
            D3D12_HEAP_FLAGS                   MapBigosMemoryHeapUsageToD3D12HeapFlags( MEMORY_HEAP_USAGE usage );
            D3D12_RESOURCE_DIMENSION           MapBigosResourceTypeToD3D12ResourceDimension( RESOURCE_TYPE type );
            D3D12_RESOURCE_FLAGS               MapBigosResourceUsageFlagsToD3D12ResourceFlags( ResourceUsageFlags flags );
            UINT                               MapBigosTextureComponentFlagsToD3D12PlaneSlice( TextureComponentFlags flags );
            D3D12_FILTER                MapBigosFiltesToD3D12Filter( FILTER_TYPE min, FILTER_TYPE mag, FILTER_TYPE mip, SAMPLER_REDUCTION_MODE mode,
                                                                     bool_t anisotropy, bool_t compare );
            D3D12_STATIC_BORDER_COLOR   MapBigosBorderColorToD3D12StaticBorderColor( BORDER_COLOR color );
            D3D12_TEXTURE_ADDRESS_MODE  MapBigosTextureAddressModeToD3D12TextureAddressMode( TEXTURE_ADDRESS_MODE mode );
            D3D12_SHADER_VISIBILITY     MapBigosShaderVisibilityToD3D12ShaderVisibility( SHADER_VISIBILITY vis );
            D3D12_DESCRIPTOR_RANGE_TYPE MapBigosBindingTypeToD3D12DescriptorRangeType( BINDING_TYPE type );
            D3D12_DESCRIPTOR_HEAP_TYPE  MapBigosBindingHeapTypeToD3D12DescriptorHeapType( BINDING_HEAP_TYPE type );
            D3D12_QUERY_HEAP_TYPE       MapBigosQueryTypeToD3D12QueryHeapType( QUERY_TYPE type );
            D3D12_QUERY_TYPE            MapBigosQueryTypeToD3D12QueryType( QUERY_TYPE type );

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS