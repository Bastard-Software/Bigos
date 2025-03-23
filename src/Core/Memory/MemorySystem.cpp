#include "Core/Memory/MemorySystem.h"

namespace BIGOS
{
    namespace Core
    {
        namespace Memory
        {
            MemorySystem::MemorySystem()
                : m_desc()
                , m_allocators()
                , m_systemHeapAllocator()
#if( BGS_MEMORY_DEBUG )
                , m_memoryBlockInfos()
#endif // ( BGS_MEMORY_DEBUG )
                , m_pParent( nullptr )
            {
            }
            RESULT MemorySystem::Create( const MemorySystemDesc& desc, BigosEngine* pEngine )
            {
                m_desc    = desc;
                m_pParent = pEngine;

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
                for( index_t ndx = 0; ndx < m_allocators.size(); ++ndx )
                {
                    m_allocators[ ndx ]->Destroy();
                }
                m_allocators.clear();

                // Destroy base heap allocator
                m_systemHeapAllocator.Destroy();

#if( BGS_MEMORY_DEBUG )
                // TODO: Check if there is no leaks and report (assert for now)
                BGS_ASSERT( m_memoryBlockInfos.size() == 0 );

                m_memoryBlockInfos.clear();
#endif // ( BGS_MEMORY_DEBUG )
                m_pParent = nullptr;
            }

#if( BGS_MEMORY_DEBUG )
            void MemorySystem::Register( const MemoryBlockInfo* pDebugInfo )
            {
                BGS_ASSERT( pDebugInfo != nullptr );

                m_memoryBlockInfos.push_back( pDebugInfo );
            }

            void MemorySystem::Deregister( const MemoryBlockInfo* pDebugInfo )
            {
                BGS_ASSERT( pDebugInfo != nullptr );

                for( index_t ndx = 0; ndx < m_memoryBlockInfos.size(); ++ndx )
                {
                    if( m_memoryBlockInfos[ ndx ] == pDebugInfo )
                    {
                        m_memoryBlockInfos[ ndx ] = m_memoryBlockInfos.back();
                        m_memoryBlockInfos.pop_back();

                        break;
                    }
                }
            }
#endif // ( BGS_MEMORY_DEBUG )
        } // namespace Memory
    } // namespace Core
} // namespace BIGOS