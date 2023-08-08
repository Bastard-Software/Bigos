#pragma once
#include "Core/CoreTypes.h"
#include "Core/Memory/MemoryTypes.h"

#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Memory.h"
#include "Core/Memory/SystemHeapAllocator.h"

namespace BIGOS
{
    namespace Core
    {
        namespace Memory
        {
            class BGS_API MemorySystem final
            {
                friend class BigosFramework;
                friend class SystemHeapAllocator;

            public:
                MemorySystem()  = default;
                ~MemorySystem() = default;

                SystemHeapAllocator* GetSystemHeapAllocator() { return &m_systemHeapAllocator; }

                const MemorySystemDesc& GetDesc() { return m_desc; }

#if( BGS_MEMORY_DEBUG )
                const MemoryBlockInfoArray& GetMemoryBlockInfo() const { return m_memoryBlockInfos; };
#endif // ( BGS_MEMORY_DEBUG )

            protected:
                RESULT Create( const MemorySystemDesc& desc );
                void   Destroy();

#if( BGS_MEMORY_DEBUG )
                void Register( const MemoryBlockInfo* pDebugInfo );
                void Deregister( const MemoryBlockInfo* pDebugInfo );
#endif // ( BGS_MEMORY_DEBUG

            private:
                MemorySystemDesc    m_desc;
                AllocatorArray      m_allocators;
                SystemHeapAllocator m_systemHeapAllocator;

#if( BGS_MEMORY_DEBUG )

                MemoryBlockInfoArray m_memoryBlockInfos;

#endif // ( BGS_MEMORY_DEBUG )
            };
        } // namespace Memory
    }     // namespace Core
} // namespace BIGOS