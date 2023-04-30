#include "Core/Memory/SystemHeapAllocator.h"

#include "Core/Memory/MemorySystem.h"

namespace BIGOS
{
    namespace Core
    {
        namespace Memory
        {
            RESULT SystemHeapAllocator::Allocate( size_t size, void** ppMemory, const char* pFile, uint32_t line )
            {
                BGS_ASSERT( ( ppMemory != nullptr ) && ( *ppMemory == nullptr ) );

                void* pMemory = nullptr;

#if( BGS_MEMORY_DEBUG )
                pMemory = Core::Memory::MallocDebug( size );

                uint8_t*         pHeader    = reinterpret_cast<uint8_t*>( pMemory ) - sizeof( MemoryBlockInfo );
                MemoryBlockInfo* pBlockInfo = reinterpret_cast<MemoryBlockInfo*>( pHeader );

                // Fill debug info
                pBlockInfo->allocType = AllocationBlockTypes::NORMAL;
                pBlockInfo->line      = line;
                pBlockInfo->blockSize = size;
                pBlockInfo->pFile     = pFile;
                pBlockInfo->alignment = DEFAULT_ALIGNMENT;

                Register( pBlockInfo );
#else
                pMemory = Core::Memory::Malloc( size );
#endif // ( BGS_MEMORY_DEBUG )

                *ppMemory = pMemory;

                return Results::OK;
            }

            RESULT SystemHeapAllocator::AllocateAligned( size_t size, size_t alignment, void** ppMemory, const char* pFile, uint32_t line )
            {
                BGS_ASSERT( ( ppMemory != nullptr ) && ( *ppMemory == nullptr ) );

                void* pMemory = nullptr;

#if( BGS_MEMORY_DEBUG )
                pMemory = Core::Memory::MallocAlignedDebug( size, alignment );

                uint8_t*         pHeader    = reinterpret_cast<uint8_t*>( pMemory ) - sizeof( MemoryBlockInfo );
                MemoryBlockInfo* pBlockInfo = reinterpret_cast<MemoryBlockInfo*>( pHeader );

                // Fill debug info
                pBlockInfo->allocType = AllocationBlockTypes::ALIGNED;
                pBlockInfo->line      = line;
                pBlockInfo->blockSize = size;
                pBlockInfo->pFile     = pFile;
                pBlockInfo->alignment = alignment;

                Register( pBlockInfo );
#else
                pMemory = Core::Memory::MallocAligned( size, alignment );
#endif // ( BGS_MEMORY_DEBUG )

                *ppMemory = pMemory;

                return Results::OK;
            }

            void SystemHeapAllocator::Free( void** ppMemory )
            {
                BGS_ASSERT( ( ppMemory != nullptr ) && ( *ppMemory != nullptr ) );

#if( BGS_MEMORY_DEBUG )
                uint8_t*         pHeader    = reinterpret_cast<uint8_t*>( *ppMemory ) - sizeof( MemoryBlockInfo );
                MemoryBlockInfo* pBlockInfo = reinterpret_cast<MemoryBlockInfo*>( pHeader );

                BGS_ASSERT( pBlockInfo->allocType == AllocationBlockTypes::NORMAL );

                Deregister( pBlockInfo );
                Core::Memory::FreeDebug( *ppMemory );
#else
                Core::Memory::Free( *ppMemory );
#endif // ( BGS_MEMORY_DEBUG )

                *ppMemory = nullptr;
            }

            void SystemHeapAllocator::FreeAligned( void** ppMemory )
            {
                BGS_ASSERT( ( ppMemory != nullptr ) && ( *ppMemory != nullptr ) );

#if( BGS_MEMORY_DEBUG )
                uint8_t*         pHeader    = reinterpret_cast<uint8_t*>( *ppMemory ) - sizeof( MemoryBlockInfo );
                MemoryBlockInfo* pBlockInfo = reinterpret_cast<MemoryBlockInfo*>( pHeader );

                BGS_ASSERT( pBlockInfo->allocType == AllocationBlockTypes::ALIGNED );

                Deregister( pBlockInfo );
                Core::Memory::FreeAlignedDebug( *ppMemory );
#else
                Core::Memory::FreeAligned( *ppMemory );
#endif // ( BGS_MEMORY_DEBUG )

                ppMemory;
            }

            RESULT SystemHeapAllocator::Create( const AllocatorDesc& desc )
            {
                BGS_ASSERT( desc.pParent != nullptr, "Invalid parent (pParent)." );
                BGS_ASSERT( desc.type == AllocatorTypes::SYSTEM_HEAP );
                m_pParent = desc.pParent;

                // TODO: Add during development

                return Results::OK;
            }

            void SystemHeapAllocator::Destroy()
            {
                // TODO: Add during development
            }

#if( BGS_MEMORY_DEBUG )
            void SystemHeapAllocator::Register( const MemoryBlockInfo* pDebugInfo )
            {
                BGS_ASSERT( pDebugInfo != nullptr );

                m_pParent->Register( pDebugInfo );
            }

            void SystemHeapAllocator::Deregister( const MemoryBlockInfo* pDebugInfo )
            {
                BGS_ASSERT( pDebugInfo != nullptr );

                m_pParent->Deregister( pDebugInfo );
            }
#endif // ( BGS_MEMORY_DEBUG )

        } // namespace Memory
    }     // namespace Core
} // namespace BIGOS