#include "Core/Memory/MemorySystem.h"

namespace BIGOS
{
    namespace Core
    {
        namespace Memory
        {
            RESULT MemorySystem::Create( const MemorySystemDesc& desc )
            {
                m_desc = desc;

                AllocatorDesc heapAllocatorDesc;
                heapAllocatorDesc.type    = AllocatorTypes::SYSTEM_HEAP;
                heapAllocatorDesc.pParent = this;

                if( BGS_FAILED( m_systemHeapAllocator.Create( heapAllocatorDesc ) ) )
                {
                    return Results::NO_MEMORY; // TODO: Is that correct error code?
                }

                return Results::OK;
            }

            void MemorySystem::Destroy()
            {
                // Destroy everything

                // Destroy allocators
                for( index_t i = 0; i < m_allocatorPtrs.size(); ++i )
                {
                    m_allocatorPtrs[ i ]->Destroy();
                }
                m_allocatorPtrs.clear();

                // Destroy base heap allocator
                m_systemHeapAllocator.Destroy();

#if( BGS_MEMORY_DEBUG )
                // TODO: Check if there is no leaks and raport?
                m_memoryBlockInfoPtrs.clear();
#endif // ( BGS_MEMORY_DEBUG )
            }

#if( BGS_MEMORY_DEBUG )
            void MemorySystem::Register( const MemoryBlockInfo* pDebugInfo )
            {
                BGS_ASSERT( pDebugInfo != nullptr );

                m_memoryBlockInfoPtrs.push_back( pDebugInfo );
            }

            void MemorySystem::Deregister( const MemoryBlockInfo* pDebugInfo )
            {
                BGS_ASSERT( pDebugInfo != nullptr );

                for( index_t i = 0; i < m_memoryBlockInfoPtrs.size(); ++i )
                {
                    if( m_memoryBlockInfoPtrs[ i ] == pDebugInfo )
                    {
                        m_memoryBlockInfoPtrs[ i ] = m_memoryBlockInfoPtrs.back();
                        m_memoryBlockInfoPtrs.pop_back();

                        break;
                    }
                }
            }
#endif    // ( BGS_MEMORY_DEBUG )
        } // namespace Memory
    }     // namespace Core
} // namespace BIGOS