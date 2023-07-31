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

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS