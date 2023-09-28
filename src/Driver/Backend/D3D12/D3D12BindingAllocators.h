#pragma once

#include "Core/CoreTypes.h"

namespace BIGOS::Driver::Backend
{

    class PoolAllocator
    {
    public:
        PoolAllocator()  = default;
        ~PoolAllocator() = default;

        RESULT Create( uint32_t capacity );
        void   Destroy();

        RESULT Allocate( uint32_t* pNdx );
        void   Free( uint32_t ndx );

    private:
        HeapArray<bool_t> m_chunkMap;
    };

} // namespace BIGOS::Driver::Backend