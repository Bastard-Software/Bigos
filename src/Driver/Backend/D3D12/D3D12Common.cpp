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

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS