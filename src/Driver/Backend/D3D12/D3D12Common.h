#pragma once

#include "D3D12Types.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            D3D12_COMMAND_LIST_TYPE MapBigosQueueTypeToD3D12CommandListType( QUEUE_TYPE type );

            D3D12_COMMAND_QUEUE_PRIORITY MapBigosQueuePriorityToD3D12CommandQueuePriority( QUEUE_PRIORITY_TYPE prio );
        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS