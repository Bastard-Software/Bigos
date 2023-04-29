#pragma once
#include "Core/CoreTypes.h"

namespace BIGOS
{
    namespace Core
    {
        namespace Memory
        {
            class MemorySystem;
            class IAllocator;
            class SystemHeapAllocator;

            using AllocatorArray = HeapArray<IAllocator*>;

            enum class AllocatorTypes : uint8_t
            {
                SYSTEM_HEAP,
                // TODO: Add during development
                _LAST_ENUM,
            };
            using ALLOCATOR_TYPE = AllocatorTypes;

            struct MemorySystemDesc
            {
                // TODO: Add during development
            };

            struct AllocatorDesc
            {
                MemorySystem*  pParent;
                ALLOCATOR_TYPE type;
            };

#if( BGS_MEMORY_DEBUG )

            enum class AllocationBlockTypes : uint8_t
            {
                NORMAL,
                ALIGNED,
                _LAST_ENUM,
            };
            using ALLOCATION_BLOCK_TYPE = AllocationBlockTypes;

            struct MemoryBlockInfo
            {
                uint64_t              blockID;
                const char*           pFile;
                size_t                blockSize;
                size_t                alignment;
                uint32_t              line;
                ALLOCATION_BLOCK_TYPE allocType;
            };

            using MemoryBlockInfoArray = HeapArray<const MemoryBlockInfo*>;

            struct MemoryInfo
            {
                uint64_t usage;
                // TODO: Add during development
            };

#endif // ( BGS_MEMORY_DEBUG )

        } // namespace Memory
    }     // namespace Core
} // namespace BIGOS